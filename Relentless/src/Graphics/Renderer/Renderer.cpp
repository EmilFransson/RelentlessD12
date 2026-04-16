#include "Renderer.h"

#include "Assets/AssetManager.h"
#include "Assets/CoreTypes/Environment.h"
#include "Assets/CoreTypes/Material.h"
#include "Assets/CoreTypes/Mesh.h"
#include "Assets/CoreTypes/Texture2D.h"
#include "Assets/CoreTypes/TextureCube.h"

#include "Core/Time.h"

#include "ECS/Component.h"

#include "File/FilePath.h"

#include "Graphics/RHI/CommandContext.h"
#include "Graphics/RHI/Device.h"
#include "Graphics/RHI/RingBufferAllocator.h"

#include "Scene/Scene.h"
#include "Subsystem/CoreTypes/SkyBoxRenderSubsystem.h"
#include "Subsystem/CoreTypes/SkyLightRenderSubsystem.h"

#include "Utility/FilepathUtils.h"

namespace Relentless
{
	Renderer::Renderer(GraphicsDevice* pDevice) noexcept
		: m_pDevice{pDevice}
	{
		GraphicsCommon::Create(m_pDevice);

		//Subsystems:
		GetSubsystem<SkyBoxRenderSubsystem>();
		GetSubsystem<SkyLightRenderSubsystem>();

		m_pForwardRenderer			= MakeUnique<ForwardRenderer>(pDevice);
		m_pEditorGrid				= MakeUnique<EditorGrid>(pDevice);
		m_pPostProcessing			= MakeUnique<PostProcessing>(pDevice);
		m_pDepthPrePass				= MakeUnique<DepthPrePass>(pDevice);
		m_pHBAOPlus					= MakeUnique<HBAOPlus>(pDevice);
		m_pOutlines					= MakeUnique<Outlines>(pDevice);
		m_pAutoExposure				= MakeUnique<AutoExposure>(pDevice);
		m_pSkyBoxRenderer			= MakeUnique<SkyBoxRenderer>(pDevice);

		InitializePipelines();

		m_EntityIDReadbackBuffer = m_pDevice->CreateBuffer(BufferDesc::CreateReadback(sizeof(float)), "Entity ID Readback Buffer");
	}

	void Renderer::Render(Scene* pScene, ViewTransform* pViewTransform, MAYBE_UNUSED const GraphicsOptions& graphicsOptions, Ref<Texture> pTarget) noexcept
	{
		PROFILE_SCOPE("Renderer::Render");

		{
			PROFILE_SCOPE("Renderer::Render::DispatchCallbacks");
			std::lock_guard<std::mutex> guard(s_DispatchMutex);
			for (auto& request : s_EnqueuedRequests)
				request(this);

			s_EnqueuedRequests.clear();
		}

		{
			PROFILE_SCOPE("Renderer::Render::OnFrameBeginCallbacks");
			std::lock_guard<std::mutex> guard(m_OnFrameBeginMutex);
			for (auto&[id, frameBeginCallback] : m_OnFrameBeginCallbacks)
				frameBeginCallback();
		}

		{
			PROFILE_SCOPE("Renderer::Render::RenderJobs");

			for (auto it = s_InProgressRenderJobs.begin(); it != s_InProgressRenderJobs.end();)
			{
				RenderJob& job = *it;
				if (job.State->Sync.IsComplete())
				{
					job.State->ConditionVariable.notify_all();
					it = s_InProgressRenderJobs.erase(it);
				}
				else
					break;
			}

			std::lock_guard<std::mutex> guard(s_RenderJobMutex);
			for (auto& job : s_EnqueuedRenderJobs)
			{
				CommandContext* pCommandContext = m_pDevice->AllocateCommandContext(job.Type == ERenderJobType::Raster ? D3D12_COMMAND_LIST_TYPE_DIRECT : D3D12_COMMAND_LIST_TYPE_COMPUTE);
				job.Callback(*pCommandContext);
				job.State->Sync = pCommandContext->Execute();
				job.State->Submitted = true;
			}

			std::ranges::move(s_EnqueuedRenderJobs, std::back_inserter(s_InProgressRenderJobs));
			s_EnqueuedRenderJobs.clear();
		}

		if (!m_BRDFLutTextureHandle.IsValid())
		{
			m_BRDFLutTextureHandle = m_OnRequestBRDFLut();
			Ref<Texture2D> pBRDFLutTexture = AssetManager::Get<Texture2D>(m_BRDFLutTextureHandle);
			pBRDFLutTexture->CreateResource();
			GetSubsystem<SkyLightRenderSubsystem>()->SetBRDFLutTexture(pBRDFLutTexture->GetResource());
		}

		if (!m_EntityIDSyncs.empty() && m_EntityIDSyncs.front().IsComplete())
		{
			m_EntityIDReadbackBuffer->Map(0u, nullptr);

			void* pMappedData = m_EntityIDReadbackBuffer->GetMappedData();
			const float encodedID = *reinterpret_cast<float*>(pMappedData);
			
			m_EntityIDReadbackBuffer->Unmap(0u, nullptr);

			const uint32 id = static_cast<uint32>(encodedID);
			OnEntityIDReadbackDone(id);

			m_EntityIDSyncs.pop();
		}

		m_pCurrentScene = pScene;

		const uint32 width = pTarget->GetWidth();
		const uint32 height = pTarget->GetHeight();

		bool drawGrid = false;
		bool drawEntityMask = false;
		RenderModeEx renderMode = RenderModeEx::Solid;
		float minLogLuminance = -4.0f;
		float minEV100 = -10.0f;
		float maxEV100 = 20.0f;
		float exposureCompensation = 1.0f;

		ViewportRenderView* pViewportRenderView = dynamic_cast<ViewportRenderView*>(pViewTransform);
		if (pViewportRenderView)
		{
			drawGrid = pViewportRenderView->DrawGrid;
			drawEntityMask = pViewportRenderView->MouseHoverCoordinates != Vector2i(-1, -1);
			renderMode = pViewportRenderView->RenderMode;
			minLogLuminance = pViewportRenderView->MinLogLuminance;
			minEV100 = pViewportRenderView->MinEV100;
			maxEV100 = pViewportRenderView->MaxEV100;
			exposureCompensation = pViewportRenderView->ExposureCompensation;
		}

		//Set up render view:
		{
			m_MainView.Viewport				= FloatRect(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
			m_MainView.pRenderer			= this;
			m_MainView.pScene				= m_pCurrentScene;

			m_MainView.ViewToWorld			= pViewTransform->ViewToWorld;
			m_MainView.WorldToView			= pViewTransform->WorldToView;
			m_MainView.ClipToView			= pViewTransform->ClipToView;
			m_MainView.WorldToClipPrev		= m_MainView.WorldToClip;
			m_MainView.WorldToClip			= pViewTransform->WorldToClip;
			m_MainView.ViewToClip			= pViewTransform->ViewToClip;

			m_MainView.LocationPrev			= m_MainView.Location;
			m_MainView.Location				= pViewTransform->Location;
			
			m_MainView.NearPlane			= pViewTransform->NearPlane;
			m_MainView.FarPlane				= pViewTransform->FarPlane;
			m_MainView.FoV					= pViewTransform->FoV;
			m_MainView.IsPerspective		= pViewTransform->IsPerspective;

			m_MainView.PerspectiveFrustum	= pViewTransform->PerspectiveFrustum;
			m_MainView.OrthographicFrustum	= pViewTransform->OrthographicFrustum;

			m_MainView.FrameIndex			= m_Frame;
		}
		
		if (!m_pColortarget || m_pColortarget->GetWidth() != width || m_pColortarget->GetHeight() != height)
			m_pColortarget = m_pDevice->CreateTexture(TextureDesc::Create2D(width, height, ResourceFormat::RGBA32_FLOAT, 1u, TextureFlag::RenderTarget | TextureFlag::ShaderResource | TextureFlag::UnorderedAccess), "Color Target");

		if (!m_pDepthTarget || m_pDepthTarget->GetWidth() != width || m_pDepthTarget->GetHeight() != height)
			m_pDepthTarget = m_pDevice->CreateTexture(TextureDesc::Create2D(width, height, ResourceFormat::R32_TYPELESS), "Depth Target");

		m_SceneTextures.pColorTarget = m_pColortarget;
		m_SceneTextures.pDepthTarget = m_pDepthTarget;
		m_SceneTextures.pEnvironmentTarget = GraphicsCommon::GetDefaultTexture(DefaultTextureType::BlackCube);

		{
			PROFILE_SCOPE("Renderer::Render::SyncRingBuffer");

			m_pDevice->GetRingBuffer()->Sync();
		}
		
		//THIS SHOULD NOT BE DONE PER RENDER SINCE WE CAN HAVE MULTIPLE RENDERVIEWS!!!
		//IT SHOULD BE DONE ONCE PER FRAME BEFORE RENDERS OCCUR! MIND SHARED RESOURCES!
		{
			CommandContext* pCommandContext = m_pDevice->AllocateCommandContext();
			
			{
				for (auto& [id, callback] : m_OnUploadCallbacks)
					callback(*pCommandContext);
			}
			
			UploadSceneData(*pCommandContext);
			UploadViewUniforms(*pCommandContext, m_MainView);

			pCommandContext->Execute();
		}

		// Sort
		{
			PROFILE_SCOPE("Renderer::Render::Sort");

			auto&& CompareSort = [this](const Batch& a, const Batch& b)
				{
					const float aDist = Vector3::DistanceSquared(a.Location, m_MainView.Location);
					const float bDist = Vector3::DistanceSquared(b.Location, m_MainView.Location);
					if (a.BlendMode != b.BlendMode)
						return (int)a.BlendMode < (int)b.BlendMode;
					return EnumHasAnyFlags(a.BlendMode, Batch::Blending::AlphaBlend) ? bDist < aDist : aDist < bDist;
				};
			std::sort(m_Batches.begin(), m_Batches.end(), CompareSort);
		}

		//Pre-Z
		{
			PROFILE_SCOPE("Renderer::Render::Pre-Z");

			CommandContext* pCommandContext = m_pDevice->AllocateCommandContext();
			m_pDepthPrePass->Render(*pCommandContext, m_MainView, m_SceneTextures);
			pCommandContext->Execute();
		}

		//Skybox
		{
			PROFILE_SCOPE("Renderer::Render::Skybox");
			m_pSkyBoxRenderer->Render(m_MainView, m_SceneTextures);
		}

		//Forward
		{
			PROFILE_SCOPE("Renderer::Render::Forward");
			m_pDevice->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT)->InsertWait(m_pDevice->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE));
			
			CommandContext* pCommandContext = m_pDevice->AllocateCommandContext();
			m_pForwardRenderer->Render(*pCommandContext, m_MainView, m_SceneTextures, renderMode);
			pCommandContext->Execute();
		}

		//HBAO+
		{
			PROFILE_SCOPE("Renderer::Render::HBAO+");
			
			CommandContext* pCommandContext = m_pDevice->AllocateCommandContext();
			m_pHBAOPlus->Render(*pCommandContext, m_MainView, m_SceneTextures);
			pCommandContext->Execute();
		}

		//Entity IDs
		{
			PROFILE_SCOPE("Renderer::Render::Entity IDs");

			const uint32 width = m_SceneTextures.pColorTarget->GetWidth();
			const uint32 height = m_SceneTextures.pColorTarget->GetHeight();
			const ResourceFormat colorFormat = ResourceFormat::R32_FLOAT;
			
			if (!m_pEntityIDTexture || m_pEntityIDTexture->GetWidth() != m_SceneTextures.pColorTarget->GetWidth() || m_pEntityIDTexture->GetHeight() != m_SceneTextures.pColorTarget->GetHeight())
			{
				const TextureDesc colorTargetDesc = TextureDesc::Create2D(
					width,
					height,
					colorFormat,
					1u,
					TextureFlag::RenderTarget | TextureFlag::ShaderResource,
					ClearBinding(Colors::Black),
					m_SceneTextures.pColorTarget->GetSampleCount());

				m_pEntityIDTexture = m_pDevice->CreateTexture(colorTargetDesc, "Color Target");
			}

			if (!m_pEntityIDDepthTarget || m_pEntityIDDepthTarget->GetWidth() != m_SceneTextures.pColorTarget->GetWidth() || m_pEntityIDDepthTarget->GetHeight() != m_SceneTextures.pColorTarget->GetHeight())
			{
				const TextureDesc depthTargetDesc = TextureDesc::Create2D(
					width,
					height,
					m_SceneTextures.pDepthTarget->GetFormat(),
					1u,
					TextureFlag::DepthStencil,
					ClearBinding(1.0f, 1u),
					m_SceneTextures.pDepthTarget->GetSampleCount());

				m_pEntityIDDepthTarget = m_pDevice->CreateTexture(depthTargetDesc, "Depth Target");
			}
			
			RenderPassInfo info;
			info.RenderTargets[0].pTarget = m_pEntityIDTexture;
			info.RenderTargets[0].BeginAccessFlags = RenderTargetAccessFlags::Clear;
			info.RenderTargets[0].EndAccessFlags = RenderTargetAccessFlags::Preserve;
			info.RenderTargetCount++;

			info.DepthStencilTarget.pTarget = m_pEntityIDDepthTarget;
			info.DepthStencilTarget.BeginAccessFlags = DepthTargetAccessFlags::ClearDepth;
			info.DepthStencilTarget.EndAccessFlags = DepthTargetAccessFlags::None;

			CommandContext& commandContext = *m_pDevice->AllocateCommandContext();

			commandContext.BeginRenderPass(info);

			commandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			commandContext.SetGraphicsRootSignature(m_pDevice->GetGlobalRootSignature());
			commandContext.SetPipelineState(m_pEntityIdPSO);

			BindViewData(commandContext, m_MainView);

			EntityManager& entityManager = m_pCurrentScene->GetEntityManager();

			for (const Batch& batch : m_Batches)
			{
				struct
				{
					uint32 InstanceIndex;
					uint32 EntityID;
				} params;

				params.InstanceIndex = batch.InstanceID;
				params.EntityID = entityManager.GetIdentity(batch.EntityID) + 1;

				commandContext.BindRootCBV(BindingSlot::PerInstance, (const void*)&params, sizeof(params));

				const uint32 numIndices = batch.pMesh->GetIndexBuffer()->GetNrOfElements();
				commandContext.Draw(0u, numIndices, 0u, 1u);
			}

			commandContext.EndRenderPass();

			if (drawEntityMask)
			{
				
				const uint32 x = Math::Min(pViewportRenderView->MouseHoverCoordinates.x, (int)m_pEntityIDTexture->GetWidth() - 1);
				const uint32 y = Math::Min(pViewportRenderView->MouseHoverCoordinates.y, (int)m_pEntityIDTexture->GetHeight() - 1);

				D3D12_BOX box{};
				box.left = x;
				box.right = box.left + 1;
				box.top = y;
				box.bottom = box.top + 1;
				box.front = 0;
				box.back = 1;
			
				commandContext.InsertResourceBarrier(m_pEntityIDTexture, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE);
				commandContext.CopyTexture(m_pEntityIDTexture, m_EntityIDReadbackBuffer, box);
				commandContext.InsertResourceBarrier(m_pEntityIDTexture, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
			}

			SyncPoint s = commandContext.Execute();
			m_EntityIDSyncs.push(s);
		}

		//Outlines
		{
			PROFILE_SCOPE("Renderer::Render::Outlines");

			CommandContext* pCommandContext = m_pDevice->AllocateCommandContext();
			m_pOutlines->Render(*pCommandContext, m_MainView, m_SceneTextures);
			pCommandContext->Execute();
		}

		//Auto Exposure:
		{
			PROFILE_SCOPE("Renderer::Render::AutoExposure");
			CommandContext* pCommandContext = m_pDevice->AllocateCommandContext();
			m_pAutoExposure->Render(*pCommandContext, m_MainView, m_SceneTextures, minLogLuminance, minEV100, maxEV100, exposureCompensation);
			pCommandContext->Execute();
		}

		//Editor Grid:
		if (drawGrid)
		{
			PROFILE_SCOPE("Renderer::Render::Editor Grid");
		
			CommandContext* pCommandContext = m_pDevice->AllocateCommandContext();
			m_pEditorGrid->Render(*pCommandContext, m_MainView, m_SceneTextures);
			pCommandContext->Execute();
		}

		//Post processing:
		{
			PROFILE_SCOPE("Renderer::Render::Post Process");

			m_pDevice->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE)->InsertWait(m_pDevice->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT));

			CommandContext* pCommandContext = m_pDevice->AllocateCommandContext(D3D12_COMMAND_LIST_TYPE_COMPUTE);
			m_pPostProcessing->Render(*pCommandContext, m_MainView, m_SceneTextures, m_pOutlines->GetSelectedEntityIDOutput(), m_pOutlines->GetBlurredOutput(), m_pAutoExposure->GetAverageLuminanceBuffer(), pTarget);
			pCommandContext->Execute();

			m_pDevice->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT)->InsertWait(m_pDevice->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE));
		}

		//Blit:
		//{
		//	PROFILE_SCOPE("Renderer::Render::Blit");
		//
		//	CommandContext* pCommandContext = m_pDevice->AllocateCommandContext();
		//	
		//	pCommandContext->InsertResourceBarrier(pTarget, D3D12_RESOURCE_STATE_COPY_DEST);
		//	pCommandContext->InsertResourceBarrier(m_SceneTextures.pColorTarget, D3D12_RESOURCE_STATE_COPY_SOURCE);
		//
		//	pCommandContext->CopyResource(m_SceneTextures.pColorTarget, pTarget);
		//
		//	pCommandContext->InsertResourceBarrier(pTarget, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		//	pCommandContext->Execute();
		//}

		m_Frame++;
	}

	CallbackID Renderer::RegisterOnFrameRenderBeginCallback(Callback<void()> aFrameRenderBeginCallback) noexcept
	{
		std::lock_guard<std::mutex> lock(m_OnFrameBeginMutex);
		static CallbackID nextCallbackID = 0;
		CallbackID toReturn = nextCallbackID++;

		m_OnFrameBeginCallbacks.emplace(toReturn, std::move(aFrameRenderBeginCallback));

		return toReturn;
	}

	CallbackID Renderer::RegisterOnUploadCallback(Callback<void(CommandContext&)> aUploadCallback) noexcept
	{
		std::lock_guard<std::mutex> lock(m_OnUploadMutex);
		static CallbackID nextCallbackID = 0;
		CallbackID toReturn = nextCallbackID++;

		m_OnUploadCallbacks.emplace(toReturn, std::move(aUploadCallback));

		return toReturn;
	}

	RenderJobHandle Renderer::SubmitComputeJob(Callback<void(CommandContext&)>&& aCallback) noexcept
	{
		Ref<RenderJobState> pJobState = RLS_NEW RenderJobState();

		std::lock_guard<std::mutex> guard(s_RenderJobMutex);
		RenderJob& renderJob = s_EnqueuedRenderJobs.emplace_back();
		renderJob.Callback = std::move(aCallback);
		renderJob.Type = ERenderJobType::Compute;
		renderJob.State = pJobState;

		return RenderJobHandle(pJobState);
	}

	RenderJobHandle Renderer::SubmitRenderJob(Callback<void(CommandContext&)>&& aCallback) noexcept
	{
		Ref<RenderJobState> pJobState = RLS_NEW RenderJobState();
		
		std::lock_guard<std::mutex> guard(s_RenderJobMutex);
		RenderJob& renderJob = s_EnqueuedRenderJobs.emplace_back();
		renderJob.Callback = std::move(aCallback);
		renderJob.Type = ERenderJobType::Raster;
		renderJob.State = pJobState;

		return RenderJobHandle(pJobState);
	}

	void Renderer::UnregisterOnFrameRenderBeginCallback(CallbackID aCallbackID) noexcept
	{
		std::lock_guard<std::mutex> lock(m_OnFrameBeginMutex);
		m_OnFrameBeginCallbacks.erase(aCallbackID);
	}

	void Renderer::UnregisterOnUploadCallback(CallbackID aCallbackID) noexcept
	{
		std::lock_guard<std::mutex> lock(m_OnUploadMutex);
		m_OnUploadCallbacks.erase(aCallbackID);
	}

	void Renderer::DrawScene(CommandContext& context, const RenderView& view, Batch::Blending blendMode) noexcept
	{
		DrawScene(context, view.pRenderer->GetBatches(), blendMode);
	}

	void Renderer::DrawScene(CommandContext& context, Span<const Batch> batches, Batch::Blending blendMode) noexcept
	{
		PROFILE_SCOPE("Renderer::DrawScene");

		for (const Batch& batch : batches)
		{
			if (EnumHasAllFlags(batch.BlendMode, blendMode))
			{
				struct  
				{
					uint32 InstanceID = 0xFFFFFFFF;
				} params;

				params.InstanceID = batch.InstanceID;
				context.BindRootCBV(BindingSlot::PerInstance, (const void*)&params, sizeof(params));
				
				const uint32 numIndices = batch.pMesh->GetIndexBuffer()->GetNrOfElements();
				context.Draw(0u, numIndices, 0u, 1u);
			}
		}
	}

	Span<const Batch> Renderer::GetBatches() const noexcept
	{
		return m_Batches;
	}

	GraphicsDevice* Renderer::GetDevice() const noexcept
	{
		return m_pDevice;
	}

	uint32 Renderer::GetFrameIndex() const noexcept
	{
		return m_Frame;
	}

	void Renderer::BindViewData(CommandContext& commandContext, const RenderView& pRenderView) noexcept
	{
		ShaderInterop::ViewUniforms viewUniforms;
		pRenderView.pRenderer->GetViewUniforms(pRenderView, viewUniforms);
		commandContext.BindRootCBV(BindingSlot::PerView, &viewUniforms, sizeof(ShaderInterop::ViewUniforms));
	}

	void Renderer::Dispatch(Callback<void(Renderer*)>&& aCallback) noexcept
	{
		std::lock_guard<std::mutex> guard(s_DispatchMutex);
		s_EnqueuedRequests.emplace_back(std::move(aCallback));
	}

	void Renderer::GetViewUniforms(const RenderView& aRenderView, ShaderInterop::ViewUniforms& outViewUniform) noexcept
	{
		outViewUniform.WorldToView				= aRenderView.WorldToView;
		outViewUniform.ViewToWorld				= aRenderView.ViewToWorld;
		outViewUniform.ViewToClip				= aRenderView.ViewToClip;
		outViewUniform.ClipToView				= aRenderView.ClipToView;
		outViewUniform.WorldToClip				= aRenderView.WorldToClip;
		outViewUniform.WorldToClip.Invert(outViewUniform.ClipToWorld);

		outViewUniform.ViewLocation				= aRenderView.Location;

		outViewUniform.ViewportDimensions		= Vector2(aRenderView.Viewport.GetWidth(), aRenderView.Viewport.GetHeight());
		outViewUniform.ViewportDimensionsInv	= Vector2(1.0f / aRenderView.Viewport.GetWidth(), 1.0f / aRenderView.Viewport.GetHeight());

		outViewUniform.FrameIndex				= aRenderView.FrameIndex;
		outViewUniform.DeltaTime				= Time::GetDeltaTime();
		outViewUniform.ElapsedTime				= Time::GetElapsedTime();

		outViewUniform.NumInstances				= m_InstancesBuffer.Count;
		outViewUniform.LightCount				= m_LightsBuffer.Count;
		outViewUniform.EnvironmentIndex			= m_EnvironmentBuffer.pBuffer ? m_EnvironmentBuffer.pBuffer->GetSRVIndex() : ShaderInterop::INVALID_DESCRIPTOR_INDEX;
		
		outViewUniform.InstancesIndex			= m_InstancesBuffer.pBuffer ? m_InstancesBuffer.pBuffer->GetSRVIndex() : ShaderInterop::INVALID_DESCRIPTOR_INDEX;
		outViewUniform.MeshesIndex				= m_MeshesBuffer.pBuffer ? m_MeshesBuffer.pBuffer->GetSRVIndex() : ShaderInterop::INVALID_DESCRIPTOR_INDEX;
		outViewUniform.MaterialsIndex			= m_MaterialsBuffer.pBuffer ? m_MaterialsBuffer.pBuffer->GetSRVIndex() : ShaderInterop::INVALID_DESCRIPTOR_INDEX;
		outViewUniform.LightsIndex				= m_LightsBuffer.pBuffer ? m_LightsBuffer.pBuffer->GetSRVIndex() : ShaderInterop::INVALID_DESCRIPTOR_INDEX;

		outViewUniform.SkyLightIndex			= GetSubsystem<SkyLightRenderSubsystem>()->GetRenderData()->GetSRVIndex();
		outViewUniform.SkyBoxIndex				= GetSubsystem<SkyBoxRenderSubsystem>()->GetRenderData()->GetSRVIndex();
	}

	void Renderer::InitializePipelines()
	{
		//Entity IDs:
		{
			PipelineStateInitializer psoDesc{};
			psoDesc.SetBlendMode(BlendMode::Replace);
			psoDesc.SetDepthEnabled(true);
			psoDesc.SetDepthFunc(D3D12_COMPARISON_FUNC_LESS_EQUAL);
			psoDesc.SetDepthWrite(true);
			psoDesc.SetVertexShader("EntityOutputShader", "vs_main");
			psoDesc.SetPixelShader("EntityOutputShader", "ps_main");
			psoDesc.SetRootSignature(m_pDevice->GetGlobalRootSignature());
			psoDesc.SetRenderTargetFormats(ResourceFormat::R32_FLOAT, ResourceFormat::D32_FLOAT, 1);

			psoDesc.SetName("Outlines - Solid");

			m_pEntityIdPSO = m_pDevice->CreatePipeline(psoDesc);
		}
	}

	void Renderer::UploadSceneData(CommandContext& commandContext) noexcept
	{
		PROFILE_SCOPE("Renderer::UploadSceneData");

		GraphicsDevice* pDevice = m_pDevice;

		auto&& CopyBufferData = [&](uint32 numElements, uint32 stride, const char* pName, const void* pSource, SceneBuffer& target)
			{
				const uint32 desiredElements = Math::AlignUp(Math::Max(1u, numElements), 8u);
				
				if (!target.pBuffer || desiredElements > target.pBuffer->GetNrOfElements())
					target.pBuffer = pDevice->CreateBuffer(BufferDesc::CreateStructured(desiredElements, stride), pName);

				ScratchAllocation alloc = commandContext.AllocateScratch(numElements * stride);
				memcpy(alloc.pMappedMemory, pSource, numElements * stride);
				commandContext.CopyBuffer(alloc.pBackingResource, target.pBuffer, alloc.Size, alloc.Offset, 0);
				target.Count = numElements;
			};

		std::vector<Batch> batches;

		uint32 instanceID = 0u;
		std::unordered_map<UUID, uint32> meshUIDToIndexMap;
		std::unordered_map<UUID, uint32> materialUIDToIndexMap;
		
		//Materials
		{
			const Texture* pDefaultBlack2D = GraphicsCommon::GetDefaultTexture(DefaultTextureType::Black2D);
			const Texture* pDefaultWhite2D = GraphicsCommon::GetDefaultTexture(DefaultTextureType::White2D);
			const Texture* pDefaultNormal = GraphicsCommon::GetDefaultTexture(DefaultTextureType::Normal2D);

			std::vector<ShaderInterop::Material> materials;
			materials.reserve(AssetManager::GetNumAssets<Material>());

			auto&& ConditionallyCreateAndReturnTextureIndex = [&](ETextureType textureType, const Material& aMaterial, const Texture* aFallbackTexture = nullptr) -> uint32
				{
					if (aMaterial.HasTexture(textureType))
					{
						Ref<Texture2D> pTexture = aMaterial.GetTexture(textureType);
						if (!pTexture->GetResource())
							pTexture->CreateResource();

						return pTexture->GetResource()->GetSRVIndex();
					}

					if (aFallbackTexture)
						return aFallbackTexture->GetSRVIndex();
					else
						return ShaderInterop::INVALID_DESCRIPTOR_INDEX;
				};

			AssetManager::ForEachAsset<Material>([&](Material& aMaterial)
				{
					meshUIDToIndexMap[aMaterial.GetUUID()] = (uint32)materials.size();

					ShaderInterop::Material& material = materials.emplace_back();
					material.AlbedoIndex = ConditionallyCreateAndReturnTextureIndex(ETextureType::Albedo, aMaterial, pDefaultWhite2D);
					material.NormalIndex = ConditionallyCreateAndReturnTextureIndex(ETextureType::NormalMap, aMaterial, pDefaultNormal);
					material.RoughnessIndex = ConditionallyCreateAndReturnTextureIndex(ETextureType::Roughness, aMaterial, pDefaultWhite2D);
					material.MetalnessIndex = ConditionallyCreateAndReturnTextureIndex(ETextureType::Metallic, aMaterial, pDefaultWhite2D);
					material.EmissiveIndex = ConditionallyCreateAndReturnTextureIndex(ETextureType::Emission, aMaterial, pDefaultWhite2D);
					material.HeightMapIndex = ConditionallyCreateAndReturnTextureIndex(ETextureType::DisplacementMap, aMaterial, pDefaultBlack2D);
					material.AOIndex = ConditionallyCreateAndReturnTextureIndex(ETextureType::AmbientOcclusion, aMaterial, pDefaultWhite2D);
					material.RoughnessMetalnessIndex = ShaderInterop::INVALID_DESCRIPTOR_INDEX;

					material.BaseColorFactor = aMaterial.GetAlbedoColor();
					material.EmissiveFactor = aMaterial.GetEmissiveColor();
					material.MetalnessFactor = aMaterial.GetMetalness();
					material.RoughnessFactor = aMaterial.GetRoughness();
					material.AOFactor = aMaterial.GetAmbientOcclusionIntensity();
					material.HeightFactor = aMaterial.GetDisplacementIntensity();
					material.EmissionIntensity = aMaterial.GetEmissiveIntensity();

					material.TilingFactor = aMaterial.GetGlobalTilingFactor();
					material.Offset = aMaterial.GetGlobalOffset();

					return true;
				});

			CopyBufferData((uint32)materials.size(), sizeof(ShaderInterop::Material), "Materials", materials.data(), m_MaterialsBuffer);
		}

		//Meshes
		{
			std::vector<ShaderInterop::MeshData> meshDatas;
			meshDatas.reserve(AssetManager::GetNumAssets<Mesh>());

			AssetManager::ForEachAsset<Mesh>([&](Mesh& aMesh)
				{
					meshUIDToIndexMap[aMesh.GetUUID()] = (uint32)meshDatas.size();

					ShaderInterop::MeshData& meshData = meshDatas.emplace_back();
					meshData.VertexBufferIndex = aMesh.GetVertexBuffer()->GetSRVIndex();
					meshData.IndexBufferIndex = aMesh.GetIndexBuffer()->GetSRVIndex();

					return true;
				});

			CopyBufferData((uint32)meshDatas.size(), sizeof(ShaderInterop::MeshData), "Meshes", meshDatas.data(), m_MeshesBuffer);
		}

		//Instances
		{
			Collection<TransformComponent, MeshRendererComponent, MeshFilterComponent> instanceCollection = m_pCurrentScene->GetEntityManager().Collect<TransformComponent, MeshRendererComponent, MeshFilterComponent>();

			std::vector<ShaderInterop::InstanceData> instanceDatas;
			instanceDatas.reserve(instanceCollection.Size());

			instanceCollection.Do([&](entity e, TransformComponent& tc, MeshRendererComponent& mrc, MeshFilterComponent& mc)
				{
					if (m_pCurrentScene->GetEntityManager().Has<HiddenInGameComponent>(e))
						return;

					const Ref<Material> pMaterial = AssetManager::Get<Material>(mrc.AssetHandle);
					const Ref<Mesh> pMesh = AssetManager::Get<Mesh>(mc.AssetHandle);

					const uint32 materialIndex = materialUIDToIndexMap[pMaterial->GetUUID()];
					const uint32 meshIndex = meshUIDToIndexMap[pMesh->GetUUID()];;

					auto&& GetBlendMode = [](EBlendMode renderMode) -> Batch::Blending
						{
							switch (renderMode)
							{
							case EBlendMode::Opaque:		return Batch::Blending::Opaque;
							case EBlendMode::AlphaMask:		return Batch::Blending::AlphaMask;
							case EBlendMode::AlphaBlend:	return Batch::Blending::AlphaBlend;
							default:
								RLS_ASSERT(false, "Unreachable.");
								return Batch::Blending::Opaque;
							}
						};

					ShaderInterop::InstanceData& instanceData = instanceDatas.emplace_back();
					instanceData.ID = instanceID;
					instanceData.LocalToWorld = tc.GetWorldMatrix();
					instanceData.MaterialIndex = materialIndex;
					instanceData.MeshDataIndex = meshIndex;

					Batch& batch = batches.emplace_back();
					batch.Location = tc.GetWorldLocation();
					batch.InstanceID = instanceID;
					batch.MaterialIndex = materialIndex;
					batch.MeshIndex = meshIndex;
					batch.pMesh = pMesh;
					batch.BlendMode = GetBlendMode(pMaterial->GetBlendMode());
					batch.EntityID = e;

					instanceID++;
				});

			CopyBufferData((uint32)instanceDatas.size(), sizeof(ShaderInterop::InstanceData), "Instances", instanceDatas.data(), m_InstancesBuffer);
		}

		//Lights
		{
			EntityManager& entityManager = m_pCurrentScene->GetEntityManager();
			Collection<TransformComponent, DirectionalLightComponent> directionalLightCollection = entityManager.Collect<TransformComponent, DirectionalLightComponent>();
			Collection<TransformComponent, PointLightComponent> pointLightCollection = entityManager.Collect<TransformComponent, PointLightComponent>();
			Collection<TransformComponent, SpotLightComponent> spotLightCollection = entityManager.Collect<TransformComponent, SpotLightComponent>();

			std::vector<ShaderInterop::Light> lights;
			lights.reserve(directionalLightCollection.Size() + pointLightCollection.Size() + spotLightCollection.Size());

			directionalLightCollection.Do([&](entity e, TransformComponent& tc, DirectionalLightComponent& dlc)
				{
					if (entityManager.Has<HiddenInGameComponent>(e))
						return;

					ShaderInterop::Light& light = lights.emplace_back();
					light.Intensity = dlc.GetIntensity();
					light.IsDirectional = true;
					light.IsPoint = light.IsSpot = false;
					light.Color = Vector3(dlc.GetColor().x, dlc.GetColor().y, dlc.GetColor().z);
					if (dlc.IsUsingTemperature())
					{
						const Color tempColor = Math::MakeFromColorTemperature(dlc.GetTemperature());
						const Vector3 vTempColor = Vector3(tempColor.x, tempColor.y, tempColor.z);
						light.Color *= vTempColor;
					}

					light.Direction = tc.GetWorldForward();
					light.IsEnabled = dlc.GetIntensity() > 0.0f;
				});

			pointLightCollection.Do([&](entity e, TransformComponent& tc, PointLightComponent& plc)
				{
					if (entityManager.Has<HiddenInGameComponent>(e))
						return;

					ShaderInterop::Light& light = lights.emplace_back();
					light.Intensity = plc.GetIntensity();
					light.IsPoint = true;
					light.IsDirectional = light.IsSpot = false;
					light.Color = Vector3(plc.GetColor().x, plc.GetColor().y, plc.GetColor().z);
					if (plc.IsUsingTemperature())
					{
						const Color tempColor = Math::MakeFromColorTemperature(plc.GetTemperature());
						const Vector3 vTempColor = Vector3(tempColor.x, tempColor.y, tempColor.z);
						light.Color *= vTempColor;
					}

					light.Position = tc.GetWorldLocation();
					light.IsEnabled = plc.GetIntensity() > 0.0f;
					light.Range = plc.GetAttenuationRadius() > 0.0f ? (1.0f / (plc.GetAttenuationRadius() * plc.GetAttenuationRadius())) : 0.0f;
				});

			spotLightCollection.Do([&](entity e, TransformComponent& tc, SpotLightComponent& slc)
				{
					if (entityManager.Has<HiddenInGameComponent>(e))
						return;

					ShaderInterop::Light& light = lights.emplace_back();
					light.Intensity = slc.GetIntensity();
					light.IsSpot = true;
					light.IsDirectional = light.IsPoint = false;
					light.Color = Vector3(slc.GetColor().x, slc.GetColor().y, slc.GetColor().z);
					if (slc.IsUsingTemperature())
					{
						const Color tempColor = Math::MakeFromColorTemperature(slc.GetTemperature());
						const Vector3 vTempColor = Vector3(tempColor.x, tempColor.y, tempColor.z);
						light.Color *= vTempColor;
					}

					light.Position = tc.GetWorldLocation();
					light.IsEnabled = slc.GetIntensity() > 0.0f;
					light.Range = slc.GetAttenuationRadius() > 0.0f ? (1.0f / (slc.GetAttenuationRadius() * slc.GetAttenuationRadius())) : 0.0f;
					light.SpotlightAngles.x = Math::Cos(slc.GetInnerConeAngleRadians() * 0.5f);
					light.SpotlightAngles.y = Math::Cos(slc.GetOuterConeAngleRadians() * 0.5f);
					light.Direction = tc.GetWorldForward();
				});


			CopyBufferData((uint32)lights.size(), sizeof(ShaderInterop::Light), "Lights", lights.data(), m_LightsBuffer);
		}

		batches.swap(m_Batches);
	}

	void Renderer::UploadViewUniforms(CommandContext& commandContext, RenderView& view) noexcept
	{
		ScratchAllocation alloc = commandContext.AllocateScratch(sizeof(ShaderInterop::ViewUniforms));
		ShaderInterop::ViewUniforms& parameters = alloc.As<ShaderInterop::ViewUniforms>();
		GetViewUniforms(view, parameters);
		
		if (!view.ViewCB)
			view.ViewCB = commandContext.GetParent()->CreateBuffer(BufferDesc{ .Size = sizeof(ShaderInterop::ViewUniforms), .ElementSize = sizeof(ShaderInterop::ViewUniforms) }, "ViewUniforms");
		
		commandContext.CopyBuffer(alloc.pBackingResource, view.ViewCB, alloc.Size, alloc.Offset, 0);
	}
}