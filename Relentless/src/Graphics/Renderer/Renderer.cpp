#include "Renderer.h"

#include "Assets/AssetManager.h"
#include "Assets/Factory/TextureFactory.h"
#include "Core/Time.h"
#include "ECS/Component.h"
#include "File/FilePath.h"
#include "Graphics/RHI/CommandContext.h"
#include "Graphics/RHI/Device.h"
#include "Graphics/RHI/RingBufferAllocator.h"
#include "Scene/Scene.h"

namespace Relentless
{
	Renderer::Renderer(GraphicsDevice* pDevice) noexcept
		: m_pDevice{pDevice}
	{
		m_pForwardRenderer = std::make_unique<ForwardRenderer>(pDevice);
		m_pEditorGrid = std::make_unique<EditorGrid>(pDevice);
		m_pPostProcessing = std::make_unique<PostProcessing>(pDevice);
		m_pDepthPrePass = std::make_unique<DepthPrePass>(pDevice);
		m_pHBAOPlus = std::make_unique<HBAOPlus>(pDevice);
		m_pOutlines = std::make_unique<Outlines>(pDevice);

		InitializePipelines();

		m_EntityIDReadbackBuffer = m_pDevice->CreateBuffer(BufferDesc::CreateReadback(sizeof(float)), "Entity ID Readback Buffer");

		Ref<TextureFactory> pFactory = RLS_NEW TextureFactory();
		pFactory->SetImportAsSRGB(true);
		pFactory->SetGenerateMipmaps(false);
		pFactory->OnDone.Connect([this](const ImportedAsset& asset, bool success)
			{
				RLS_VERIFY(success, "[Renderer] Failed to import BRDF LUT.");
				m_BRDFLutTextureHandle = asset.Handle;
			});

		std::vector<AssetImportTask> tasks;
		AssetImportTask& task = tasks.emplace_back();
		task.FilePath = FilepathUtils::Combine(FilePath::GetEngineWorkingDirectory(), "Assets/Textures/brdf_ibl_lut.dds");
		task.pFactory = pFactory;

		Importer::RequestAsyncLoad(tasks).Wait();
	}

	void Renderer::Render(Scene* pScene, ViewTransform* pViewTransform, const GraphicsOptions& graphicsOptions, Ref<Texture> pTarget) noexcept
	{
		PROFILE_SCOPE("Renderer::Render");

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
		float exposure = 1.0f;

		ViewportRenderView* pViewportRenderView = dynamic_cast<ViewportRenderView*>(pViewTransform);
		if (pViewportRenderView)
		{
			drawGrid = pViewportRenderView->DrawGrid;
			drawEntityMask = pViewportRenderView->MouseHoverCoordinates != Vector2i(-1, -1);
			renderMode = pViewportRenderView->RenderMode;
			exposure = pViewportRenderView->Exposure;
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

			m_MainView.Exposure				= exposure;
		}
		
		if (!m_pColortarget || m_pColortarget->GetWidth() != width || m_pColortarget->GetHeight() != height)
			m_pColortarget = m_pDevice->CreateTexture(TextureDesc::Create2D(width, height, ResourceFormat::RGBA32_FLOAT), "Color Target");

		if (!m_pDepthTarget || m_pDepthTarget->GetWidth() != width || m_pDepthTarget->GetHeight() != height)
			m_pDepthTarget = m_pDevice->CreateTexture(TextureDesc::Create2D(width, height, ResourceFormat::R32_TYPELESS), "Depth Target");

		m_SceneTextures.pColorTarget = m_pColortarget;
		m_SceneTextures.pDepthTarget = m_pDepthTarget;

		{
			PROFILE_SCOPE("Renderer::Render::SyncRingBuffer");

			m_pDevice->GetRingBuffer()->Sync();
		}
		
		//THIS SHOULD NOT BE DONE PER RENDER SINCE WE CAN HAVE MULTIPLE RENDERVIEWS!!!
		//IT SHOULD BE DONE ONCE PER FRAME BEFORE RENDERS OCCUR! MIND SHARED RESOURCES!
		{
			CommandContext* pCommandContext = m_pDevice->AllocateCommandContext();
			
			UploadSceneData(*pCommandContext);
			UploadViewUniforms(*pCommandContext, m_MainView);
			
			pCommandContext->Execute();
		}

		// Sort
		{
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

		//Forward
		{
			PROFILE_SCOPE("Renderer::Render::Forward");

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

		//Editor Grid:
		if (drawGrid)
		{
			PROFILE_SCOPE("Renderer::Render::Editor Grid");

			CommandContext* pCommandContext = m_pDevice->AllocateCommandContext();
			m_pEditorGrid->Render(*pCommandContext, m_MainView, m_SceneTextures);
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
			m_pOutlines->Render(*pCommandContext, m_MainView, m_SceneTextures, m_pEntityIDTexture);
			pCommandContext->Execute();
		}

		//Post processing:
		{
			PROFILE_SCOPE("Renderer::Render::Post Process");

			m_pDevice->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE)->InsertWait(m_pDevice->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT));

			CommandContext* pCommandContext = m_pDevice->AllocateCommandContext(D3D12_COMMAND_LIST_TYPE_COMPUTE);
			m_pPostProcessing->Render(*pCommandContext, m_MainView, m_SceneTextures, m_pOutlines->GetSelectedEntityIDOutput(), m_pOutlines->GetBlurredOutput());
			pCommandContext->Execute();

			m_pDevice->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT)->InsertWait(m_pDevice->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE));
		}

		//Blit:
		{
			PROFILE_SCOPE("Renderer::Render::Blit");

			CommandContext* pCommandContext = m_pDevice->AllocateCommandContext();
			
			pCommandContext->InsertResourceBarrier(pTarget, pTarget->GetResourceState(), D3D12_RESOURCE_STATE_COPY_DEST);
			pCommandContext->InsertResourceBarrier(m_SceneTextures.pColorTarget, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_SOURCE);
		
			pCommandContext->CopyResource(m_SceneTextures.pColorTarget, pTarget);
		
			pCommandContext->InsertResourceBarrier(pTarget, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			pCommandContext->Execute();
		}

		m_Frame++;
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

	void Renderer::GetViewUniforms(const RenderView& renderView, ShaderInterop::ViewUniforms& outViewUniform) noexcept
	{
		outViewUniform.WorldToView				= renderView.WorldToView;
		outViewUniform.ViewToWorld				= renderView.ViewToWorld;
		outViewUniform.ViewToClip				= renderView.ViewToClip;
		outViewUniform.ClipToView				= renderView.ClipToView;
		outViewUniform.WorldToClip				= renderView.WorldToClip;
		outViewUniform.WorldToClip.Invert(outViewUniform.ClipToWorld);

		outViewUniform.ViewLocation				= renderView.Location;
		outViewUniform.BRDFfLutTextureIndex		= m_BRDFLutTextureHandle.IsValid() ? AssetManager::Get<Texture>(m_BRDFLutTextureHandle)->GetSRVIndex() : ShaderInterop::INVALID_DESCRIPTOR_INDEX;

		outViewUniform.ViewportDimensions		= Vector2(renderView.Viewport.GetWidth(), renderView.Viewport.GetHeight());
		outViewUniform.ViewportDimensionsInv	= Vector2(1.0f / renderView.Viewport.GetWidth(), 1.0f / renderView.Viewport.GetHeight());

		outViewUniform.FrameIndex				= m_Frame;
		outViewUniform.DeltaTime				= Time::GetDeltaTime();
		outViewUniform.ElapsedTime				= Time::GetElapsedTime();
		outViewUniform.RadianceMapIndex			= ShaderInterop::INVALID_DESCRIPTOR_INDEX;

		outViewUniform.NumInstances				= m_InstancesBuffer.Count;
		outViewUniform.LightCount				= m_LightsBuffer.Count;
		outViewUniform.EnvironmentIndex			= m_EnvironmentBuffer.pBuffer ? m_EnvironmentBuffer.pBuffer->GetSRVIndex() : ShaderInterop::INVALID_DESCRIPTOR_INDEX;
		outViewUniform.IrradianceMapIndex		= ShaderInterop::INVALID_DESCRIPTOR_INDEX;
		
		outViewUniform.InstancesIndex			= m_InstancesBuffer.pBuffer ? m_InstancesBuffer.pBuffer->GetSRVIndex() : ShaderInterop::INVALID_DESCRIPTOR_INDEX;
		outViewUniform.MeshesIndex				= m_MeshesBuffer.pBuffer ? m_MeshesBuffer.pBuffer->GetSRVIndex() : ShaderInterop::INVALID_DESCRIPTOR_INDEX;
		outViewUniform.MaterialsIndex			= m_MaterialsBuffer.pBuffer ? m_MaterialsBuffer.pBuffer->GetSRVIndex() : ShaderInterop::INVALID_DESCRIPTOR_INDEX;
		outViewUniform.LightsIndex				= m_LightsBuffer.pBuffer ? m_LightsBuffer.pBuffer->GetSRVIndex() : ShaderInterop::INVALID_DESCRIPTOR_INDEX;
	
		outViewUniform.Exposure					= renderView.Exposure;
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

		AssetStorage<Material>& materialStorage = AssetManager::GetStorage<Material>();
		AssetStorage<Mesh>& meshStorage = AssetManager::GetStorage<Mesh>();

		//Instances
		{
			Collection<TransformComponent, MeshRendererComponent, MeshFilterComponent> instanceCollection = m_pCurrentScene->GetEntityManager().Collect<TransformComponent, MeshRendererComponent, MeshFilterComponent>();

			std::vector<ShaderInterop::InstanceData> instanceDatas;
			instanceDatas.reserve(instanceCollection.Size());

			instanceCollection.Do([&](entity e, MeshRendererComponent& mrc, MeshFilterComponent& mc)
				{
					if (m_pCurrentScene->GetEntityManager().Has<HiddenInGameComponent>(e))
						return;

					const Ref<Material> pMaterial = AssetManager::Get<Material>(mrc.AssetHandle);
					const Ref<Mesh> pMesh = AssetManager::Get<Mesh>(mc.AssetHandle);

					const uint32 materialIndex = materialStorage.GetPhysicalIndex(mrc.AssetHandle);
					const uint32 meshIndex = meshStorage.GetPhysicalIndex(mc.AssetHandle);

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
					instanceData.LocalToWorld = m_pCurrentScene->GetWorldTransform(e);
					instanceData.MaterialIndex = materialIndex;
					instanceData.MeshDataIndex = meshIndex;

					Batch& batch = batches.emplace_back();
					batch.Location = m_pCurrentScene->GetWorldLocation(e);
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
		
		//Materials
		{
			std::vector<ShaderInterop::Material> materials;
			materials.reserve(materialStorage.Assets.size());

			for (uint32 i = 0; i < materialStorage.Assets.size(); ++i)
			{
				const auto& pMaterial = materialStorage.Assets[i];

				ShaderInterop::Material& material = materials.emplace_back();
				material.AlbedoIndex = pMaterial->HasAlbedoTexture() ? pMaterial->GetAlbedoTexture()->GetSRVIndex() : ShaderInterop::INVALID_DESCRIPTOR_INDEX;
				material.NormalIndex = pMaterial->HasNormalMap() ? pMaterial->GetNormalMap()->GetSRVIndex() : ShaderInterop::INVALID_DESCRIPTOR_INDEX;
				material.RoughnessIndex = pMaterial->HasRoughnessTexture() ? pMaterial->GetRoughnessTexture()->GetSRVIndex() : ShaderInterop::INVALID_DESCRIPTOR_INDEX;
				material.MetalnessIndex = pMaterial->HasMetallicTexture() ? pMaterial->GetMetallicTexture()->GetSRVIndex() : ShaderInterop::INVALID_DESCRIPTOR_INDEX;
				material.EmissiveIndex = pMaterial->HasEmissionTexture() ? pMaterial->GetEmissionTexture()->GetSRVIndex() : ShaderInterop::INVALID_DESCRIPTOR_INDEX;
				material.HeightMapIndex = pMaterial->HasHeightMap() ? pMaterial->GetHeightMap()->GetSRVIndex() : ShaderInterop::INVALID_DESCRIPTOR_INDEX;
				material.AOIndex = pMaterial->HasAmbientOcclusionTexture() ? pMaterial->GetAmbientOcclusionTexture()->GetSRVIndex() : ShaderInterop::INVALID_DESCRIPTOR_INDEX;
				material.RoughnessMetalnessIndex = ShaderInterop::INVALID_DESCRIPTOR_INDEX;

				material.BaseColorFactor = pMaterial->m_AlbedoColor;
				material.EmissiveFactor = pMaterial->m_EmissionColor;
				material.MetalnessFactor = pMaterial->m_Metallic;
				material.RoughnessFactor = pMaterial->m_Roughness;
				material.AOFactor = pMaterial->m_AOScale;
				material.HeightFactor = pMaterial->m_HeightScale;
				material.EmissionIntensity = pMaterial->m_EmissionIntensity;

				material.TilingFactor = pMaterial->m_TilingFactor;
				material.Offset = pMaterial->m_Offset;
			}
			CopyBufferData((uint32)materials.size(), sizeof(ShaderInterop::Material), "Materials", materials.data(), m_MaterialsBuffer);
		}

		//Meshes
		{
			std::vector<ShaderInterop::MeshData> meshDatas;
			meshDatas.reserve(meshStorage.Assets.size());

			for (uint32 i = 0; i < meshStorage.Assets.size(); ++i)
			{
				const auto& pMesh = meshStorage.Assets[i];

				ShaderInterop::MeshData& meshData = meshDatas.emplace_back();
				meshData.VertexBufferIndex = pMesh->GetVertexBuffer()->GetSRVIndex();
				meshData.IndexBufferIndex = pMesh->GetIndexBuffer()->GetSRVIndex();
			}

			CopyBufferData((uint32)meshDatas.size(), sizeof(ShaderInterop::MeshData), "Meshes", meshDatas.data(), m_MeshesBuffer);
		}

		//Lights
		{
			EntityManager& entityManager = m_pCurrentScene->GetEntityManager();
			Collection<DirectionalLightComponent> directionalLightCollection = entityManager.Collect<DirectionalLightComponent>();
			Collection<PointLightComponent> pointLightCollection = entityManager.Collect<PointLightComponent>();
			Collection<SpotLightComponent> spotLightCollection = entityManager.Collect<SpotLightComponent>();

			std::vector<ShaderInterop::Light> lights;
			lights.reserve(directionalLightCollection.Size() + pointLightCollection.Size() + spotLightCollection.Size());

			directionalLightCollection.Do([&](entity e, DirectionalLightComponent& dlc)
				{
					if (entityManager.Has<HiddenInGameComponent>(e))
						return;

					ShaderInterop::Light& light = lights.emplace_back();
					light.Intensity = dlc.Intensity;
					light.IsDirectional = true;
					light.IsPoint = light.IsSpot = false;
					light.Color = Vector3(dlc.Color.x, dlc.Color.y, dlc.Color.z);
					if (dlc.UseTemperature)
					{
						const Color tempColor = Math::MakeFromColorTemperature(dlc.Temperature);
						const Vector3 vTempColor = Vector3(tempColor.x, tempColor.y, tempColor.z);
						light.Color *= vTempColor;
					}

					light.Direction = m_pCurrentScene->GetWorldForward(e);
					light.IsEnabled = dlc.Intensity > 0.0f;
				});

			pointLightCollection.Do([&](entity e, PointLightComponent& plc)
				{
					if (entityManager.Has<HiddenInGameComponent>(e))
						return;

					ShaderInterop::Light& light = lights.emplace_back();
					light.Intensity = plc.Intensity;
					light.IsPoint = true;
					light.IsDirectional = light.IsSpot = false;
					light.Color = Vector3(plc.Color.x, plc.Color.y, plc.Color.z);
					if (plc.UseTemperature)
					{
						const Color tempColor = Math::MakeFromColorTemperature(plc.Temperature);
						const Vector3 vTempColor = Vector3(tempColor.x, tempColor.y, tempColor.z);
						light.Color *= vTempColor;
					}

					light.Position = m_pCurrentScene->GetWorldLocation(e);
					light.IsEnabled = plc.Intensity > 0.0f;
					light.Range = plc.AttenuationRadius;
				});

			spotLightCollection.Do([&](entity e, SpotLightComponent& slc)
				{
					if (entityManager.Has<HiddenInGameComponent>(e))
						return;

					ShaderInterop::Light& light = lights.emplace_back();
					light.Intensity = slc.Intensity;
					light.IsSpot = true;
					light.IsDirectional = light.IsPoint = false;
					light.Color = Vector3(slc.Color.x, slc.Color.y, slc.Color.z);
					if (slc.UseTemperature)
					{
						const Color tempColor = Math::MakeFromColorTemperature(slc.Temperature);
						const Vector3 vTempColor = Vector3(tempColor.x, tempColor.y, tempColor.z);
						light.Color *= vTempColor;
					}

					light.Position = m_pCurrentScene->GetWorldLocation(e);
					light.IsEnabled = slc.Intensity > 0.0f;
					light.Range = slc.AttenuationRadius;
					light.SpotlightAngles.x = std::cos(slc.InnerConeAngle / 2.0f);
					light.SpotlightAngles.y = std::cos(slc.OuterConeAngle / 2.0f);
					light.Direction = m_pCurrentScene->GetWorldForward(e);
				});


			CopyBufferData((uint32)lights.size(), sizeof(ShaderInterop::Light), "Lights", lights.data(), m_LightsBuffer);
		}

		//Environment
		{
			ShaderInterop::Environment environment;
			environment.BackgroundColor = Vector3(Colors::Black.x, Colors::Black.y, Colors::Black.z);
			CopyBufferData(1u, sizeof(ShaderInterop::Environment), "Environment", &environment, m_EnvironmentBuffer);
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