#include "Renderer.h"

#include "Assets/AssetManager.h"
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

		std::vector<ImportRequest> requests;
		ImportRequest& request = requests.emplace_back();
		
		TextureImportSettings importSettings;
		importSettings.GenerateMipMaps = false;
		importSettings.IsSRGB = false;
		importSettings.TextureCompressionType = ETextureCompressionType::Uncompressed;
		importSettings.IsHDR = false;
		
		request.ImportSettings = importSettings;

		request.Filepath = FilepathUtils::Combine(FilePath::GetEngineWorkingDirectory(), "Assets/Textures/brdf_ibl_lut.dds");
		Ref<ImporterFeedbackContext> pFeedback = new ImporterFeedbackContext();
		pFeedback->OnAssetImported.Connect([this](const AssetHandle& handle, bool success)
			{
				m_BRDFLutTextureHandle = handle;
			});

		std::future<void> fut = Importer::RequestAsyncLoad(Application::Get().GetGraphicsDevice(), requests, pFeedback);
		fut.wait();
	}

	void Renderer::Render(Scene* pScene, ViewTransform* pViewTransform, const GraphicsOptions& graphicsOptions, Ref<TextureEx> pTarget) noexcept
	{
		m_pCurrentScene = pScene;

		const uint32 width = pTarget->GetWidth();
		const uint32 height = pTarget->GetHeight();

		//Set up render view:
		{
			m_MainView.Viewport				= FloatRect(0, 0, width, height);
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
		}
																													
		m_SceneTextures.pColorTarget = m_pDevice->CreateTexture(TextureDesc::Create2D(width, height, ResourceFormat::RGBA32_FLOAT), "Color Target");
		m_SceneTextures.pDepthTarget = m_pDevice->CreateTexture(TextureDesc::Create2D(width, height, ResourceFormat::D32_FLOAT), "Depth Target");

		{
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

		{
			// Sort
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

		bool drawGrid			= false;
		bool drawEntityMask		= false;
		RenderModeEx renderMode = RenderModeEx::Solid;

		ViewportRenderView* pViewportRenderView = dynamic_cast<ViewportRenderView*>(pViewTransform);
		if (pViewportRenderView)
		{
			drawGrid		= pViewportRenderView->DrawGrid;
			drawEntityMask	= pViewportRenderView->MouseHoverCoordinates != Vector2i(-1, -1);
			renderMode		= pViewportRenderView->RenderMode;
		}

		//Forward
		{
			CommandContext* pCommandContext = m_pDevice->AllocateCommandContext();
			m_pForwardRenderer->Render(*pCommandContext, m_MainView, m_SceneTextures, renderMode);
			pCommandContext->Execute();
		}

		//Editor Grid:
		if (drawGrid)
		{
			CommandContext* pCommandContext = m_pDevice->AllocateCommandContext();
			m_pEditorGrid->Render(*pCommandContext, m_MainView, m_SceneTextures);
			pCommandContext->Execute();
		}

		//Post processing:
		{
			m_pDevice->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE)->InsertWait(m_pDevice->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT));

			CommandContext* pCommandContext = m_pDevice->AllocateCommandContext(D3D12_COMMAND_LIST_TYPE_COMPUTE);
			m_pPostProcessing->Render(*pCommandContext, m_MainView, m_SceneTextures);
			pCommandContext->Execute();

			m_pDevice->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT)->InsertWait(m_pDevice->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE));
		}

		{
			CommandContext* pCommandContext = m_pDevice->AllocateCommandContext();

			pCommandContext->InsertResourceBarrier(pTarget, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
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
		outViewUniform.BRDFfLutTextureIndex		= m_BRDFLutTextureHandle.IsValid() ? AssetManager::Get<TextureEx>(m_BRDFLutTextureHandle)->GetSRVIndex() : ShaderInterop::INVALID_DESCRIPTOR_INDEX;

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
	}

	void Renderer::UploadSceneData(CommandContext& commandContext) noexcept
	{
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

			instanceCollection.Do([&](TransformComponent& tc, MeshRendererComponent& mrc, MeshFilterComponent& mc)
				{
					const Ref<Material> pMaterial = AssetManager::Get<Material>(mrc.AssetHandle);
					const Ref<Mesh> pMesh = AssetManager::Get<Mesh>(mc.AssetHandle);

					const uint32 materialIndex = materialStorage.GetPhysicalIndex(mrc.AssetHandle);
					const uint32 meshIndex = meshStorage.GetPhysicalIndex(mc.AssetHandle);

					auto&& GetBlendMode = [](RenderMode renderMode) -> Batch::Blending
						{
							switch (renderMode)
							{
							case RenderMode::Opaque:		return Batch::Blending::Opaque;
							case RenderMode::CutOut:		return Batch::Blending::AlphaMask;
							case RenderMode::Transparent:	return Batch::Blending::AlphaBlend;
							default:
								RLS_ASSERT(false, "Unreachable.");
								return Batch::Blending::Opaque;
							}
						};

					Batch& batch = batches.emplace_back();
					batch.Location = tc.WorldTransform.Location;
					batch.InstanceID = instanceID;
					batch.MaterialIndex = materialIndex;
					batch.MeshIndex = meshIndex;
					batch.pMesh = pMesh;
					batch.BlendMode = GetBlendMode(pMaterial->GetRenderMode());

					ShaderInterop::InstanceData& instanceData = instanceDatas.emplace_back();
					instanceData.ID = instanceID;
					instanceData.LocalToWorld = tc.WorldTransform.Matrix;
					instanceData.MaterialIndex = materialIndex;
					instanceData.MeshDataIndex = meshIndex;

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
				material.MetalnessIndex = pMaterial->HasMetallicTexture() ? pMaterial->GetRoughnessTexture()->GetSRVIndex() : ShaderInterop::INVALID_DESCRIPTOR_INDEX;
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

			std::vector<ShaderInterop::Light> lights;
			lights.reserve(directionalLightCollection.Size() + pointLightCollection.Size());

			directionalLightCollection.Do([&](DirectionalLightComponent& dlc)
				{
					ShaderInterop::Light& light = lights.emplace_back();
					light.Intensity = dlc.Intensity;
					light.IsDirectional = true;
					light.IsPoint = light.IsSpot = false;
					light.Color = dlc.Color;
					light.Direction = dlc.Direction;
					light.IsEnabled = dlc.Intensity > 0.0f;
				});

			pointLightCollection.Do([&](PointLightComponent& plc)
				{
					ShaderInterop::Light& light = lights.emplace_back();
					light.Intensity = plc.Intensity;
					light.IsPoint = true;
					light.IsDirectional = light.IsSpot = false;
					light.Color = plc.Color;
					light.Position = plc.Position;
					light.IsEnabled = plc.Intensity > 0.0f;
				});

			CopyBufferData((uint32)lights.size(), sizeof(ShaderInterop::Light), "Lights", lights.data(), m_LightsBuffer);
		}

		//Environment
		{
			ShaderInterop::Environment environment;
			environment.BackgroundColor = Vector3(Colors::LightSkyBlue.x, Colors::LightSkyBlue.y, Colors::LightSkyBlue.z);
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