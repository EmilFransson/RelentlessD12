#include "SceneRenderer.h"
#include "../D3D12Core.h"
#include "MasterRenderer.h"
#include "RenderCommand.h"
#include "RenderUtility.h"
#include "../MemoryManager.h"
#include "../Shaders/ShaderLibrary.h"
#include "../../Core/Window.h"


namespace Relentless
{
	SceneRenderer::SceneRenderer(std::shared_ptr<Scene> pScene) noexcept
		: m_pScene{pScene}
	{
		RLS_ASSERT(pScene, "No valid scene submitted for scene renderer.");
		Initialize();
	}

	void SceneRenderer::Initialize() noexcept
	{
		InitializeHBAOPlus();

		m_EnvironmentCBHandle = MemoryManager::Get().CreateConstantBuffer(sizeof(DirectX::XMFLOAT3));
		m_BRDFLutTextureHandle = AssetManager::Load<Texture2D>(std::string(ENGINE_ASSET_DIRECTORY) + "Textures\\ibl_brdf_lut.png");

		//Pre-Z Render pass:
		{
			DepthAttachment depthAttachment;
			depthAttachment.Format = TextureFormat::R32TYPELESS;
			depthAttachment.ShouldResize = true;
			depthAttachment.Transfer = true;
			depthAttachment.OperatorOnLoad = OperatorOnLoad::Clear;

			FrameBufferSpecification frameBufferSpecification{};
			frameBufferSpecification.DebugName = "Pre-Z Framebuffer";
			frameBufferSpecification.Attachments.DepthAttachment = depthAttachment;
			frameBufferSpecification.MSAASamples = m_Options.MSAASamples;
			frameBufferSpecification.DepthComparisonFunction = DepthComparisonFunction::LESS;

			PipelineSpecification pipelineSpecification{};
			pipelineSpecification.DebugName = "Pre-Z Pipeline";
			pipelineSpecification.pVertexShader = MasterRenderer::GetShaderLibrary().Get("VertexShader");
			pipelineSpecification.pFrameBuffer = FrameBuffer::Create(frameBufferSpecification);
			pipelineSpecification.MSAAEligible = true;
			pipelineSpecification.DepthWrite = true;

			RenderPassSpecification renderpassDescriptor{};
			renderpassDescriptor.DebugName = "Pre-Z Pass";
			renderpassDescriptor.RenderPipeline = Pipeline::Create(pipelineSpecification);

			m_PreZRenderPass = RenderPass::Create(renderpassDescriptor);
		}

		//Opaque Geometry Render pass:
		{
			ColorAttachment colorAttachment;
			colorAttachment.Format = TextureFormat::RGBA32F;
			colorAttachment.ClearColor = DirectX::XMFLOAT4(DirectX::Colors::LightSkyBlue);
			colorAttachment.Transfer = true;
			colorAttachment.ShouldResize = true;

			DepthAttachment depthAttachment;
			depthAttachment.Format = TextureFormat::Depth;
			depthAttachment.OperatorOnLoad = OperatorOnLoad::LoadOnly;
			depthAttachment.Output = m_PreZRenderPass->GetPipeline()->GetFrameBuffer()->GetDepthOutput();
			depthAttachment.pOutputDependency = m_PreZRenderPass->GetPipeline()->GetFrameBuffer();
			depthAttachment.ShouldResize = false;

			FrameBufferSpecification frameBufferSpecification{};
			frameBufferSpecification.DebugName = "Opaque Geometry Framebuffer";
			frameBufferSpecification.Attachments.ColorAttachments = { colorAttachment };
			frameBufferSpecification.Attachments.DepthAttachment = depthAttachment;
			frameBufferSpecification.MSAASamples = m_Options.MSAASamples;
			frameBufferSpecification.DepthComparisonFunction = DepthComparisonFunction::EQUAL;

			PipelineSpecification pipelineSpecification{};
			pipelineSpecification.DebugName = "Opaque Geometry Pipeline";
			pipelineSpecification.pVertexShader = MasterRenderer::GetShaderLibrary().Get("VertexShader");
			pipelineSpecification.pPixelShader = MasterRenderer::GetShaderLibrary().Get("PBRPixelShader");
			pipelineSpecification.pFrameBuffer = FrameBuffer::Create(frameBufferSpecification);
			pipelineSpecification.MSAAEligible = true;
			pipelineSpecification.DepthWrite = false;

			RenderPassSpecification renderpassDescriptor{};
			renderpassDescriptor.DebugName = "Opaque Geometry Pass";
			renderpassDescriptor.RenderPipeline = Pipeline::Create(pipelineSpecification);

			m_OpaqueGeometryRenderPass = RenderPass::Create(renderpassDescriptor);
		}

		//CutOut Geometry Render pass:
		{
			ColorAttachment colorAttachment;
			colorAttachment.Format = TextureFormat::RGBA32F;
			colorAttachment.Transfer = true;
			colorAttachment.OperatorOnLoad = OperatorOnLoad::LoadOnly;
			colorAttachment.Output = m_OpaqueGeometryRenderPass->GetOutput(0);
			colorAttachment.pOutputDependency = m_OpaqueGeometryRenderPass->GetPipeline()->GetFrameBuffer();
			colorAttachment.ShouldResize = false;

			DepthAttachment depthAttachment;
			depthAttachment.Format = TextureFormat::Depth;
			depthAttachment.OperatorOnLoad = OperatorOnLoad::LoadOnly;
			depthAttachment.Output = m_PreZRenderPass->GetPipeline()->GetFrameBuffer()->GetDepthOutput();
			depthAttachment.pOutputDependency = m_PreZRenderPass->GetPipeline()->GetFrameBuffer();
			depthAttachment.ShouldResize = false;

			FrameBufferSpecification frameBufferSpecification{};
			frameBufferSpecification.DebugName = "CutOut Geometry Framebuffer";
			frameBufferSpecification.Attachments.ColorAttachments = { colorAttachment };
			frameBufferSpecification.Attachments.DepthAttachment = depthAttachment;
			frameBufferSpecification.MSAASamples = m_Options.MSAASamples;
			frameBufferSpecification.DepthComparisonFunction = DepthComparisonFunction::LESS;

			PipelineSpecification pipelineSpecification{};
			pipelineSpecification.DebugName = "CutOut Geometry Pipeline";
			pipelineSpecification.pVertexShader = MasterRenderer::GetShaderLibrary().Get("VertexShader");
			pipelineSpecification.pPixelShader = MasterRenderer::GetShaderLibrary().Get("CutOutPBRPixelShader");
			pipelineSpecification.pFrameBuffer = FrameBuffer::Create(frameBufferSpecification);
			pipelineSpecification.MSAAEligible = true;
			pipelineSpecification.DepthWrite = true;

			RenderPassSpecification renderpassDescriptor{};
			renderpassDescriptor.DebugName = "CutOut Geometry Pass";
			renderpassDescriptor.RenderPipeline = Pipeline::Create(pipelineSpecification);

			m_CutOutGeometryRenderPass = RenderPass::Create(renderpassDescriptor);
		}

		//Transparent Geometry Render pass:
		{
			ColorAttachment colorAttachment;
			colorAttachment.Format = TextureFormat::RGBA32F;
			colorAttachment.Transfer = true;
			colorAttachment.OperatorOnLoad = OperatorOnLoad::LoadOnly;
			colorAttachment.Output = m_OpaqueGeometryRenderPass->GetOutput(0);
			colorAttachment.pOutputDependency = m_OpaqueGeometryRenderPass->GetPipeline()->GetFrameBuffer();
			colorAttachment.ShouldResize = false;
			colorAttachment.Blend = true;

			DepthAttachment depthAttachment;
			depthAttachment.Format = TextureFormat::Depth;
			depthAttachment.OperatorOnLoad = OperatorOnLoad::LoadOnly;
			depthAttachment.Output = m_PreZRenderPass->GetPipeline()->GetFrameBuffer()->GetDepthOutput();
			depthAttachment.pOutputDependency = m_PreZRenderPass->GetPipeline()->GetFrameBuffer();
			depthAttachment.ShouldResize = false;

			FrameBufferSpecification frameBufferSpecification{};
			frameBufferSpecification.DebugName = "Transparent Geometry Framebuffer";
			frameBufferSpecification.Attachments.ColorAttachments = { colorAttachment };
			frameBufferSpecification.Attachments.DepthAttachment = depthAttachment;
			frameBufferSpecification.MSAASamples = m_Options.MSAASamples;
			frameBufferSpecification.DepthComparisonFunction = DepthComparisonFunction::LESS;

			PipelineSpecification pipelineSpecification{};
			pipelineSpecification.DebugName = "Transparent Geometry Pipeline";
			pipelineSpecification.pVertexShader = MasterRenderer::GetShaderLibrary().Get("VertexShader");
			pipelineSpecification.pPixelShader = MasterRenderer::GetShaderLibrary().Get("TransparentPBRPixelShader");
			pipelineSpecification.pFrameBuffer = FrameBuffer::Create(frameBufferSpecification);
			pipelineSpecification.MSAAEligible = true;
			pipelineSpecification.DepthWrite = true;

			RenderPassSpecification renderpassDescriptor{};
			renderpassDescriptor.DebugName = "Transparent Geometry Pass";
			renderpassDescriptor.RenderPipeline = Pipeline::Create(pipelineSpecification);

			m_TransparentGeometryRenderPass = RenderPass::Create(renderpassDescriptor);
		}

		//Combined Geometry and Picking Render pass:
		{
			ColorAttachment colorAttachment;
			colorAttachment.Format = TextureFormat::RGBA32F;
			colorAttachment.ClearColor = DirectX::XMFLOAT4(DirectX::Colors::LightSkyBlue);
			colorAttachment.Transfer = true;
			colorAttachment.ShouldResize = true;

			ColorAttachment colorAttachment2;
			colorAttachment2.Format = TextureFormat::R32UINT;
			static constexpr float clearVal = static_cast<float>(NULL_ENTITY);
			colorAttachment2.ClearColor = DirectX::XMFLOAT4(clearVal, clearVal, clearVal, clearVal);
			colorAttachment2.Transfer = true;
			colorAttachment2.IsSRGB = false;
			colorAttachment2.ShouldResize = true;

			DepthAttachment depthAttachment;
			depthAttachment.Format = TextureFormat::Depth;
			depthAttachment.ShouldResize = false;
			depthAttachment.OperatorOnLoad = OperatorOnLoad::LoadOnly;
			depthAttachment.Output = m_PreZRenderPass->GetPipeline()->GetFrameBuffer()->GetDepthOutput();
			depthAttachment.pOutputDependency = m_PreZRenderPass->GetPipeline()->GetFrameBuffer();

			FrameBufferSpecification frameBufferSpecification{};
			frameBufferSpecification.DebugName = "GeometryAndPicking Framebuffer";
			frameBufferSpecification.Attachments.ColorAttachments = { colorAttachment, colorAttachment2 };
			frameBufferSpecification.Attachments.DepthAttachment = depthAttachment;
			frameBufferSpecification.DepthComparisonFunction = DepthComparisonFunction::EQUAL;

			PipelineSpecification pipelineSpecification{};
			pipelineSpecification.DebugName = "GeometryAndPicking Pipeline";
			pipelineSpecification.pVertexShader = MasterRenderer::GetShaderLibrary().Get("VertexShader");
			pipelineSpecification.pPixelShader = MasterRenderer::GetShaderLibrary().Get("GeometryAndPickingPixelShader");
			pipelineSpecification.pFrameBuffer = FrameBuffer::Create(frameBufferSpecification);
			pipelineSpecification.MSAAEligible = false;
			pipelineSpecification.DepthWrite = false;

			RenderPassSpecification renderpassDescriptor{};
			renderpassDescriptor.DebugName = "GeometryAndPicking Pass";
			renderpassDescriptor.RenderPipeline = Pipeline::Create(pipelineSpecification);

			m_CombinedGeometryAndPickingPass = RenderPass::Create(renderpassDescriptor);
		}

		//Wireframe Render pass:
		{
			ColorAttachment colorAttachment;
			colorAttachment.Format = TextureFormat::RGBA32F;
			colorAttachment.Transfer = true;
			colorAttachment.OperatorOnLoad = OperatorOnLoad::LoadOnly;
			colorAttachment.Output = m_OpaqueGeometryRenderPass->GetOutput(0);
			colorAttachment.pOutputDependency = m_OpaqueGeometryRenderPass->GetPipeline()->GetFrameBuffer();
			colorAttachment.ShouldResize = false;

			DepthAttachment depthAttachment;
			depthAttachment.Format = TextureFormat::Depth;
			depthAttachment.OperatorOnLoad = OperatorOnLoad::LoadOnly;
			depthAttachment.Output = m_OpaqueGeometryRenderPass->GetDepthOutput();
			depthAttachment.pOutputDependency = m_OpaqueGeometryRenderPass->GetPipeline()->GetFrameBuffer();
			depthAttachment.ShouldResize = false;

			FrameBufferSpecification frameBufferSpecification{};
			frameBufferSpecification.DebugName = "Wireframe Framebuffer";
			frameBufferSpecification.Attachments.ColorAttachments = { colorAttachment };
			frameBufferSpecification.Attachments.DepthAttachment = { depthAttachment };
			frameBufferSpecification.DepthComparisonFunction = DepthComparisonFunction::ALWAYS;

			PipelineSpecification pipelineSpecification{};
			pipelineSpecification.DebugName = "Wireframe Pipeline";
			pipelineSpecification.pVertexShader = MasterRenderer::GetShaderLibrary().Get("VertexShader");
			pipelineSpecification.pPixelShader = MasterRenderer::GetShaderLibrary().Get("PixelShaderOrangeOutput");
			pipelineSpecification.pFrameBuffer = FrameBuffer::Create(frameBufferSpecification);
			pipelineSpecification.FillMode = FillMode::Wireframe;
			pipelineSpecification.BackfaceCulling = false;
			pipelineSpecification.MSAAEligible = true;

			RenderPassSpecification renderpassDescriptor{};
			renderpassDescriptor.DebugName = "Wireframe Pass";
			renderpassDescriptor.RenderPipeline = Pipeline::Create(pipelineSpecification);

			m_WireFrameRenderPass = RenderPass::Create(renderpassDescriptor);
		}

		//Picking:
		{
			ColorAttachment colorAttachment;
			colorAttachment.Format = TextureFormat::R32UINT;
			static constexpr float clearVal = static_cast<float>(NULL_ENTITY);
			colorAttachment.ClearColor = DirectX::XMFLOAT4(clearVal, clearVal, clearVal, clearVal);
			colorAttachment.Transfer = true;
			colorAttachment.IsSRGB = false;
			colorAttachment.ShouldResize = true;

			DepthAttachment depthAttachment;
			depthAttachment.Format = TextureFormat::Depth;

			FrameBufferSpecification frameBufferSpecification{};
			frameBufferSpecification.DebugName = "Picking Framebuffer";
			frameBufferSpecification.Attachments.ColorAttachments = { colorAttachment };
			frameBufferSpecification.Attachments.DepthAttachment = { depthAttachment };

			PipelineSpecification pipelineSpecification{};
			pipelineSpecification.DebugName = "Picking Pipeline";
			pipelineSpecification.pVertexShader = MasterRenderer::GetShaderLibrary().Get("PickingVertexShader");
			pipelineSpecification.pPixelShader = MasterRenderer::GetShaderLibrary().Get("PickingPixelShader");
			pipelineSpecification.pFrameBuffer = FrameBuffer::Create(frameBufferSpecification);
			pipelineSpecification.MSAAEligible = false;

			RenderPassSpecification renderpassDescriptor{};
			renderpassDescriptor.DebugName = "Picking Pass";
			renderpassDescriptor.RenderPipeline = Pipeline::Create(pipelineSpecification);

			m_GeometryPickingRenderPass = RenderPass::Create(renderpassDescriptor);
		}

		//Editor Grid Render pass:
		{
			ColorAttachment colorAttachment;
			colorAttachment.Format = TextureFormat::RGBA32F;
			colorAttachment.OperatorOnLoad = OperatorOnLoad::LoadOnly;
			colorAttachment.Transfer = true;
			colorAttachment.Blend = true;
			colorAttachment.Output = m_OpaqueGeometryRenderPass->GetOutput(0);
			colorAttachment.pOutputDependency = m_OpaqueGeometryRenderPass->GetPipeline()->GetFrameBuffer();
			colorAttachment.ShouldResize = false;

			DepthAttachment depthAttachment;
			depthAttachment.Format = TextureFormat::Depth;
			depthAttachment.OperatorOnLoad = OperatorOnLoad::LoadOnly;
			depthAttachment.Output = m_OpaqueGeometryRenderPass->GetPipeline()->GetFrameBuffer()->GetDepthOutput();
			depthAttachment.pOutputDependency = m_OpaqueGeometryRenderPass->GetPipeline()->GetFrameBuffer();
			depthAttachment.ShouldResize = false;

			FrameBufferSpecification frameBufferSpecification{};
			frameBufferSpecification.DebugName = "Editor Grid Framebuffer";
			frameBufferSpecification.Attachments.ColorAttachments = { colorAttachment };
			frameBufferSpecification.Attachments.DepthAttachment = depthAttachment;
			frameBufferSpecification.MSAASamples = m_Options.MSAASamples;
			frameBufferSpecification.DepthComparisonFunction = DepthComparisonFunction::LESS;

			PipelineSpecification pipelineSpecification{};
			pipelineSpecification.DebugName = "Editor Grid Pipeline";
			pipelineSpecification.pVertexShader = MasterRenderer::GetShaderLibrary().Get("VertexShaderEditorGrid");
			pipelineSpecification.pPixelShader = MasterRenderer::GetShaderLibrary().Get("PixelShaderEditorGrid");
			pipelineSpecification.pFrameBuffer = FrameBuffer::Create(frameBufferSpecification);
			pipelineSpecification.Topology = Topology::Line;
			pipelineSpecification.MSAAEligible = true;
			pipelineSpecification.DepthWrite = false;

			RenderPassSpecification renderpassDescriptor{};
			renderpassDescriptor.DebugName = "Editor Grid Pass";
			renderpassDescriptor.RenderPipeline = Pipeline::Create(pipelineSpecification);

			m_EditorGridRenderPass = RenderPass::Create(renderpassDescriptor);
		}

		//Composite
		{
			ColorAttachment colorAttachment;
			colorAttachment.Format = TextureFormat::RGBA32F;
			colorAttachment.OperatorOnLoad = OperatorOnLoad::LoadOnly;
			colorAttachment.Output = MasterRenderer::GetFrameBuffer()->GetOutput(0);
			colorAttachment.pOutputDependency = MasterRenderer::GetFrameBuffer();
			colorAttachment.ShouldResize = false;

			FrameBufferSpecification frameBufferSpecification{};
			frameBufferSpecification.DebugName = "Composite Framebuffer";
			frameBufferSpecification.Attachments.ColorAttachments = { colorAttachment };
			frameBufferSpecification.ShouldResize = false;

			PipelineSpecification pipelineSpecification{};
			pipelineSpecification.DebugName = "Composite Pipeline";
			pipelineSpecification.DepthWrite = false;
			pipelineSpecification.pVertexShader = MasterRenderer::GetShaderLibrary().Get("FullScreenTriVertexShader");
			pipelineSpecification.pPixelShader = MasterRenderer::GetShaderLibrary().Get("PostProcessPixelShader");
			pipelineSpecification.pFrameBuffer = FrameBuffer::Create(frameBufferSpecification);
			pipelineSpecification.MSAAEligible = false;
			pipelineSpecification.DepthWrite = false;

			RenderPassSpecification renderpassDescriptor{};
			renderpassDescriptor.DebugName = "Composite Pass";
			renderpassDescriptor.RenderPipeline = Pipeline::Create(pipelineSpecification);

			m_CompositeRenderPass = RenderPass::Create(renderpassDescriptor);
		}

		//Readback buffer:
		{
			m_pIdentifierReadbackBuffer = ReadBackBuffer::Create(sizeof(uint32_t), "Identifier ReadbackBuffer");
		}

		//Editor Grid:
		{
			m_EditorGridVertices.at(0).Position = DirectX::XMFLOAT3(-0.5f, 0.0f, 0.0f);
			m_EditorGridVertices.at(1).Position = DirectX::XMFLOAT3(0.5f, 0.0f, 0.0f);

			VertexBuffer::Specification vbSpec{};
			vbSpec.Name = "EditorGridVertexBuffer";
			vbSpec.NrOfVertices = EDITOR_GRID_VERTEX_COUNT;
			vbSpec.Stride = sizeof(Vertex_Basic);
			vbSpec.TotalSizeInBytes = sizeof(Vertex_Basic) * EDITOR_GRID_VERTEX_COUNT;
			vbSpec.pBuffer = m_EditorGridVertices.data();
			m_pEditorGridVertexBuffer = std::make_unique<VertexBuffer>(&vbSpec);

			m_pEditorGridInstanceDataStructuredBuffer = std::make_unique<StructuredBuffer>(EDITOR_GRID_INSTANCE_COUNT, sizeof(InstanceData));

			const uint32_t nrOfFrames = D3D12Core::GetNrOfBufferedFrames();
			for (int i{ -EDITOR_GRID_INSTANCE_COUNT / 2 }; i < EDITOR_GRID_INSTANCE_COUNT / 2; ++i)
			{
				int index = i + EDITOR_GRID_INSTANCE_COUNT / 2;
				InstanceData instanceData{};
				instanceData.Position = DirectX::XMFLOAT3(0.0f, 0.0f, static_cast<float>(i));
				instanceData.Color.R = 0.15f;
				instanceData.Color.G = 0.15f;
				instanceData.Color.B = 0.15f;

				if (i % 10 == 0)
				{
					instanceData.Color.R = 0.06f;
					instanceData.Color.G = 0.06f;
					instanceData.Color.B = 0.06f;
				}

				for (uint32_t frameIndex{ 0u }; frameIndex < nrOfFrames; ++frameIndex)
				{
					MemoryManager::Get().UpdateStructuredBuffer(*m_pEditorGridInstanceDataStructuredBuffer, &instanceData, index, frameIndex);
				}
			}

			m_EditorGridTransformComponent1.ConstantBufferID = MemoryManager::Get().CreateConstantBuffer(sizeof(DirectX::XMFLOAT4X4));
			m_EditorGridTransformComponent2.ConstantBufferID = MemoryManager::Get().CreateConstantBuffer(sizeof(DirectX::XMFLOAT4X4));
		}
		
		RenderTextureSpecification resolveRTSpec{};
		resolveRTSpec.MultiSampleCount = 1u;
		resolveRTSpec.CreateSRV = true;
		resolveRTSpec.Width = 800u;
		resolveRTSpec.Height = 600u;
		resolveRTSpec.Flags = D3D12_RESOURCE_FLAG_NONE;
		resolveRTSpec.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;

		m_pResolvedTexture = RenderTexture::Create(resolveRTSpec, "MSAA Resolve RenderTexture");

		MasterRenderer::ExecuteCommands();
		MasterRenderer::WaitForGPU();
		MasterRenderer::ResetFrameCommandUnits(0u);
	}

	void SceneRenderer::Begin() noexcept
	{
		PROFILE_FUNC;

		m_OpaqueRenderModeEntities.clear();
		m_CutOutRenderModeEntities.clear();
		m_TransparentRenderModeEntities.clear();
		m_ResourceBarrierBatch.clear();

		MaterialManager& materialManager = AssetManager::GetMaterialManager();
		m_pScene->GetEntityManager().Collect<MeshRendererComponent, MeshFilterComponent>().Do([&](entity e, MeshRendererComponent& mrc, MeshFilterComponent& mfc)
			{
				if (mrc.MaterialHandle == NULL_HANDLE || mfc.MeshHandle == NULL_HANDLE)
					return;

				const Material& material = materialManager.GetMaterial(mrc.MaterialHandle);
				switch (material.GetRenderMode())
				{
				case RenderMode::Opaque:
					m_OpaqueRenderModeEntities.emplace_back(e);
					break;
				case RenderMode::CutOut:
					m_CutOutRenderModeEntities.emplace_back(e);
					break;
				case RenderMode::Transparent:
					m_TransparentRenderModeEntities.emplace_back(e);
					break;
				}
			});

		{
			PROFILE_SCOPE("SceneRenderer::Begin::SortAllEntities");

			//Sort transparent entities back to front:
			if (m_TransparentRenderModeEntities.size() > 0)
			{
				auto& mgr = m_pScene->GetEntityManager();
				std::shared_ptr<PerspectiveCamera> pEditorCamera = m_pScene->GetEditorCamera();
				std::sort(m_TransparentRenderModeEntities.begin(), m_TransparentRenderModeEntities.end(), [&](const entity a, const entity b)
					{
						TransformComponent& tcA = mgr.Get<TransformComponent>(a);
						TransformComponent& tcB = mgr.Get<TransformComponent>(b);

						double squaredDistanceA = std::pow(tcA.Translation.x - pEditorCamera->GetPosition().x, 2) + std::pow(tcA.Translation.y - pEditorCamera->GetPosition().y, 2) + std::pow(tcA.Translation.z - pEditorCamera->GetPosition().z, 2);
						double squaredDistanceB = std::pow(tcB.Translation.x - pEditorCamera->GetPosition().x, 2) + std::pow(tcB.Translation.y - pEditorCamera->GetPosition().y, 2) + std::pow(tcB.Translation.z - pEditorCamera->GetPosition().z, 2);

						return squaredDistanceA > squaredDistanceB;
					});
			}
			//Sort opaque entities front to back:
			if (m_OpaqueRenderModeEntities.size() > 0)
			{
				auto& mgr = m_pScene->GetEntityManager();
				std::shared_ptr<PerspectiveCamera> pEditorCamera = m_pScene->GetEditorCamera();
				std::sort(m_OpaqueRenderModeEntities.begin(), m_OpaqueRenderModeEntities.end(), [&](const entity a, const entity b)
					{
						TransformComponent& tcA = mgr.Get<TransformComponent>(a);
						TransformComponent& tcB = mgr.Get<TransformComponent>(b);

						double squaredDistanceA = std::pow(tcA.Translation.x - pEditorCamera->GetPosition().x, 2) + std::pow(tcA.Translation.y - pEditorCamera->GetPosition().y, 2) + std::pow(tcA.Translation.z - pEditorCamera->GetPosition().z, 2);
						double squaredDistanceB = std::pow(tcB.Translation.x - pEditorCamera->GetPosition().x, 2) + std::pow(tcB.Translation.y - pEditorCamera->GetPosition().y, 2) + std::pow(tcB.Translation.z - pEditorCamera->GetPosition().z, 2);

						return squaredDistanceA < squaredDistanceB;
					});
			}
		}
		
		auto frameIndex = MasterRenderer::GetCurrentFrameIndex();
		m_PerFrameOpaqueGeometryData.cameraDataIndex = m_pScene->GetEditorCamera()->m_pConstantBuffer->m_VisibleHandles[frameIndex].Index;
		m_PerFrameOpaqueGeometryData.pointLightStructuredBufferIndex = m_pScene->GetLightManager().GetPointLights()->m_VisibleHandles[frameIndex].Index;
		m_PerFrameOpaqueGeometryData.directionalLightStructuredBufferIndex = m_pScene->GetLightManager().GetDirectionalLights()->m_VisibleHandles[frameIndex].Index;
		m_PerFrameOpaqueGeometryData.nrOfDirectionalLights = static_cast<uint32_t>(m_pScene->GetEntityManager().GetEntityCountForPool<DirectionalLightComponent>());
		m_PerFrameOpaqueGeometryData.nrOfPointLights = static_cast<uint32_t>(m_pScene->GetEntityManager().GetEntityCountForPool<PointLightComponent>());
		m_PerFrameOpaqueGeometryData.environmentIndex = MemoryManager::Get().GetCBDescriptorIndex(m_EnvironmentCBHandle);
		m_PerFrameOpaqueGeometryData.brdfLutTextureIndex = AssetManager::Get<Texture2D>(m_BRDFLutTextureHandle).GetSRVDescriptorHandle().Index;

		//Terrible and wrong for runtime:
		//Camera:
		auto vpMatrix = DirectX::XMLoadFloat4x4(&(m_pScene->GetEditorCamera()->GetViewProjectionMatrix()));
		DirectX::XMStoreFloat4x4(&m_VPData.VPMatrix, vpMatrix);

		//TODO: Should not be done every frame!
		static DirectX::XMFLOAT3 bgColor = DirectX::XMFLOAT3(DirectX::Colors::LightSkyBlue);
		MemoryManager::Get().UpdateConstantBuffer(m_EnvironmentCBHandle, (void*)&bgColor);

		MasterRenderer::Begin();

		RenderCommand::SetDescriptorHeap(MemoryManager::Get().GetShaderBindableDescriptorHeap());
		RenderCommand::SetViewport(m_pScene->GetViewport());
		RenderCommand::SetScissorRect(m_pScene->GetScissorRect());
	}

	void SceneRenderer::SetContext(const std::shared_ptr<Scene>& pScene) noexcept
	{
		RLS_ASSERT(pScene, "Scene is invalid.");
		m_pScene = pScene;
	}

	void SceneRenderer::IssueRenderPasses() noexcept
	{
		PROFILE_FUNC;

		PreZPass();
		if (m_Options.MSAASamples > 1)
		{
			OpaqueGeometryPass();
			CutOutGeometryPass();
			if (m_Options.ContactShadowType == ContactShadows::HBAO_PLUS)
			{
				HBAOPlusRenderPass();
			}
			EditorGridPass();
			TransparentGeometryPass();
			PickingPass();
		}
		else
		{
			CombinedGeometryAndPickingPass();
			if (m_Options.ContactShadowType == ContactShadows::HBAO_PLUS)
			{
				HBAOPlusRenderPass();
			}
			EditorGridPass();
		}
		WireframePass();

		if (m_Options.MSAASamples > 1)
		{
			AddToResourceTransitionBatch(m_OpaqueGeometryRenderPass->GetOutput(0), D3D12_RESOURCE_STATE_RESOLVE_SOURCE);
			AddToResourceTransitionBatch(m_pResolvedTexture, D3D12_RESOURCE_STATE_RESOLVE_DEST);
		}
		AddToResourceTransitionBatch(Window::GetCurrentBackBuffer().pBackBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		RenderCommand::FlushResourceTransitionBatch(m_ResourceBarrierBatch);

		if (m_Options.MSAASamples > 1)
		{
			//MSAA Resolve:
			auto pGeometryPassColorOutput = m_OpaqueGeometryRenderPass->GetOutput(0);
			RenderCommand::ResolveMSAA(pGeometryPassColorOutput, m_pResolvedTexture);
		}

		CompositePass();

		if (m_Options.MSAASamples > 1)
		{
			AddToResourceTransitionBatch(m_pResolvedTexture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		}
		else
		{

		}
		//RenderCommand::TransitionResource(m_pResolvedTexture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		AddToResourceTransitionBatch(MasterRenderer::GetFrameBuffer()->GetOutput(0), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		RenderCommand::FlushResourceTransitionBatch(m_ResourceBarrierBatch);

		//Set UI-texture as pixel shader resource and prepare back buffer as render target for imgui:
		{
			//RenderCommand::TransitionResource(MasterRenderer::GetFrameBuffer()->GetOutput(0), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			RenderCommand::SetRenderTarget(Window::GetCurrentBackBuffer());
		}
	}

	void SceneRenderer::End() noexcept
	{
		MasterRenderer::End();
	}

	void SceneRenderer::SetContactShadowsType(const ContactShadows contactShadowsType) noexcept
	{
		m_Options.ContactShadowType = contactShadowsType;
	}

	void SceneRenderer::ToggleSelectionWireframe() noexcept
	{
		m_Options.DisplaySelectionWireframe = !m_Options.DisplaySelectionWireframe;
	}

	void SceneRenderer::ToggleEditorGrid() noexcept
	{
		m_Options.DisplayEditorGrid = !m_Options.DisplayEditorGrid;
	}

	void SceneRenderer::PreZPass() noexcept
	{
		PROFILE_FUNC;
		MemoryManager& memoryManager = MemoryManager::Get();

		MasterRenderer::BeginRenderPass(m_PreZRenderPass);

		m_PreZRenderPass->Upload("vpConstantBuffer", &m_VPData);

		EntityManager& entityManager = m_pScene->GetEntityManager();
		MeshManager& m = AssetManager::GetMeshManager();

		auto verticesIndex = m_PreZRenderPass->GetInputSlot("vertices");
		auto indicesIndex = m_PreZRenderPass->GetInputSlot("indices");
		auto perDrawIndex = m_PreZRenderPass->GetInputSlot("perDrawData");

		const uint32_t count = (uint32_t)sizeof(PerDrawData) / sizeof(uint32_t);

		for (entity currentEntity : m_OpaqueRenderModeEntities)
		{
			const auto& [tc, mfc, mrc] = entityManager.Get<TransformComponent, MeshFilterComponent, MeshRendererComponent>(currentEntity);

			Mesh& mesh = m.GetMesh(mfc.MeshHandle);

			DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRootShaderResourceView(verticesIndex, mesh.GetVertexBuffer()->GetInterface()->GetGPUVirtualAddress()));
			DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRootShaderResourceView(indicesIndex, mesh.GetIndexBuffer()->GetInterface()->GetGPUVirtualAddress()));

			const Material& material = AssetManager::Get<Material>(mrc.MaterialHandle);
			m_PerDrawData.materialIndex = material.GetConstantBufferIndex();

			m_PerDrawData.worldMatrixIndex = memoryManager.GetCBDescriptorIndex(tc.ConstantBufferID);

			DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRoot32BitConstants(perDrawIndex, count, &m_PerDrawData, 0u));

			RenderCommand::DrawInstanced(mesh.GetIndexBuffer()->GetNrOfIndices());
		}

		MasterRenderer::EndRenderPass();
	}

	void SceneRenderer::HBAOPlusRenderPass() noexcept
	{
		PROFILE_FUNC;

		GFSDK_SSAO_InputData_D3D12 InputData = {};
		InputData.DepthData.DepthTextureType = GFSDK_SSAO_HARDWARE_DEPTHS;
		InputData.DepthData.FullResDepthTextureSRV.pResource = m_PreZRenderPass->GetDepthOutput()->GetInterface().Get();
		InputData.DepthData.FullResDepthTextureSRV.GpuHandle = m_PreZRenderPass->GetDepthOutput()->GetSRVDescriptorHandle().GPUHandle.ptr;
		InputData.DepthData.ProjectionMatrix.Data = GFSDK_SSAO_Float4x4((const GFSDK_SSAO_FLOAT*)&m_pScene->GetEditorCamera()->GetProjectionMatrix());
		InputData.DepthData.ProjectionMatrix.Layout = GFSDK_SSAO_ROW_MAJOR_ORDER;
		InputData.DepthData.MetersToViewSpaceUnits = 1.0f;
		InputData.NormalData.Enable = false;

		GFSDK_SSAO_RenderMask RenderMask = GFSDK_SSAO_RENDER_AO;

		GFSDK_SSAO_RenderTargetView_D3D12 rtv{};
		const auto& output = m_Options.MSAASamples > 1 ? m_OpaqueGeometryRenderPass->GetOutput(0) : m_CombinedGeometryAndPickingPass->GetOutput(0);
		const auto& depthOutput = m_Options.MSAASamples > 1 ? m_OpaqueGeometryRenderPass->GetDepthOutput() : m_CombinedGeometryAndPickingPass->GetDepthOutput();
		rtv.pResource = output->GetInterface().Get();
		rtv.CpuHandle = output->GetRTVDescriptorHandle().CPUHandle.ptr;

		GFSDK_SSAO_Output_D3D12 Output;
		Output.pRenderTargetView = &rtv;
		Output.Blend.Mode = GFSDK_SSAO_MULTIPLY_RGB;

		if (output->GetCurrentState() != D3D12_RESOURCE_STATE_RENDER_TARGET)
			RenderCommand::TransitionResource(output, D3D12_RESOURCE_STATE_RENDER_TARGET);
		
		if (depthOutput->GetCurrentState() != D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
			RenderCommand::TransitionResource(depthOutput, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	#if defined RLS_DEBUG
		GFSDK_SSAO_Status status = m_SSAOContext->RenderAO(D3D12Core::GetCommandQueue().Get(), D3D12Core::GetCommandList().Get(), InputData, m_HBAOPlusParameters, Output, RenderMask);
		RLS_ASSERT(status == GFSDK_SSAO_OK, "Failed to issue HBAOPlus render command.");
	#else
		m_SSAOContext->RenderAO(D3D12Core::GetCommandQueue().Get(), D3D12Core::GetCommandList().Get(), InputData, m_HBAOPlusParameters, Output, RenderMask);
	#endif
	}

	void SceneRenderer::OpaqueGeometryPass() noexcept
	{
		PROFILE_FUNC;

		MemoryManager& memoryManager = MemoryManager::Get();

		MasterRenderer::BeginRenderPass(m_OpaqueGeometryRenderPass);

		if (m_OpaqueRenderModeEntities.size() > 0)
		{
			m_OpaqueGeometryRenderPass->Upload("vpConstantBuffer", &m_VPData);
			m_OpaqueGeometryRenderPass->Upload("perFrameData", &m_PerFrameOpaqueGeometryData);

			EntityManager& entityManager = m_pScene->GetEntityManager();
			MeshManager& m = AssetManager::GetMeshManager();

			auto verticesIndex = m_OpaqueGeometryRenderPass->GetInputSlot("vertices");
			auto indicesIndex = m_OpaqueGeometryRenderPass->GetInputSlot("indices");
			auto perDrawIndex = m_OpaqueGeometryRenderPass->GetInputSlot("perDrawData");

			const uint32_t count = (uint32_t)sizeof(PerDrawData) / sizeof(uint32_t);

			for (entity currentEntity : m_OpaqueRenderModeEntities)
			{
				const auto& [tc, mfc, mrc] = entityManager.Get<TransformComponent, MeshFilterComponent, MeshRendererComponent>(currentEntity);

				Mesh& mesh = m.GetMesh(mfc.MeshHandle);

				DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRootShaderResourceView(verticesIndex, mesh.GetVertexBuffer()->GetInterface()->GetGPUVirtualAddress()));
				DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRootShaderResourceView(indicesIndex, mesh.GetIndexBuffer()->GetInterface()->GetGPUVirtualAddress()));

				const Material& material = AssetManager::Get<Material>(mrc.MaterialHandle);
				m_PerDrawData.materialIndex = material.GetConstantBufferIndex();

				m_PerDrawData.worldMatrixIndex = memoryManager.GetCBDescriptorIndex(tc.ConstantBufferID);

				DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRoot32BitConstants(perDrawIndex, count, &m_PerDrawData, 0u));

				RenderCommand::DrawInstanced(mesh.GetIndexBuffer()->GetNrOfIndices());
			}
		}

		MasterRenderer::EndRenderPass();
	}

	void SceneRenderer::CutOutGeometryPass() noexcept
	{
		PROFILE_FUNC;

		if (m_CutOutRenderModeEntities.empty())
			return;

		MemoryManager& memoryManager = MemoryManager::Get();

		MasterRenderer::BeginRenderPass(m_CutOutGeometryRenderPass);

		m_CutOutGeometryRenderPass->Upload("vpConstantBuffer", &m_VPData);
		m_CutOutGeometryRenderPass->Upload("perFrameData", &m_PerFrameOpaqueGeometryData);

		EntityManager& entityManager = m_pScene->GetEntityManager();
		MeshManager& m = AssetManager::GetMeshManager();

		auto verticesIndex = m_CutOutGeometryRenderPass->GetInputSlot("vertices");
		auto indicesIndex = m_CutOutGeometryRenderPass->GetInputSlot("indices");
		auto perDrawIndex = m_CutOutGeometryRenderPass->GetInputSlot("perDrawData");

		const uint32_t count = (uint32_t)sizeof(PerDrawData) / sizeof(uint32_t);

		for (entity currentEntity : m_CutOutRenderModeEntities)
		{
			const auto& [tc, mfc, mrc] = entityManager.Get<TransformComponent, MeshFilterComponent, MeshRendererComponent>(currentEntity);

			Mesh& mesh = m.GetMesh(mfc.MeshHandle);

			DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRootShaderResourceView(verticesIndex, mesh.GetVertexBuffer()->GetInterface()->GetGPUVirtualAddress()));
			DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRootShaderResourceView(indicesIndex, mesh.GetIndexBuffer()->GetInterface()->GetGPUVirtualAddress()));

			const Material& material = AssetManager::Get<Material>(mrc.MaterialHandle);
			m_PerDrawData.materialIndex = material.GetConstantBufferIndex();

			m_PerDrawData.worldMatrixIndex = memoryManager.GetCBDescriptorIndex(tc.ConstantBufferID);

			DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRoot32BitConstants(perDrawIndex, count, &m_PerDrawData, 0u));

			RenderCommand::DrawInstanced(mesh.GetIndexBuffer()->GetNrOfIndices());
		}

		MasterRenderer::EndRenderPass();
	}

	void SceneRenderer::TransparentGeometryPass() noexcept
	{
		PROFILE_FUNC;

		if (m_TransparentRenderModeEntities.empty())
			return;

		MemoryManager& memoryManager = MemoryManager::Get();

		MasterRenderer::BeginRenderPass(m_TransparentGeometryRenderPass);

		m_TransparentGeometryRenderPass->Upload("vpConstantBuffer", &m_VPData);
		m_TransparentGeometryRenderPass->Upload("perFrameData", &m_PerFrameOpaqueGeometryData);

		EntityManager& entityManager = m_pScene->GetEntityManager();
		MeshManager& m = AssetManager::GetMeshManager();

		auto verticesIndex = m_TransparentGeometryRenderPass->GetInputSlot("vertices");
		auto indicesIndex = m_TransparentGeometryRenderPass->GetInputSlot("indices");
		auto perDrawIndex = m_TransparentGeometryRenderPass->GetInputSlot("perDrawData");

		const uint32_t count = (uint32_t)sizeof(PerDrawData) / sizeof(uint32_t);

		for (entity currentEntity : m_TransparentRenderModeEntities)
		{
			const auto& [tc, mfc, mrc] = entityManager.Get<TransformComponent, MeshFilterComponent, MeshRendererComponent>(currentEntity);

			Mesh& mesh = m.GetMesh(mfc.MeshHandle);

			DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRootShaderResourceView(verticesIndex, mesh.GetVertexBuffer()->GetInterface()->GetGPUVirtualAddress()));
			DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRootShaderResourceView(indicesIndex, mesh.GetIndexBuffer()->GetInterface()->GetGPUVirtualAddress()));

			const Material& material = AssetManager::Get<Material>(mrc.MaterialHandle);
			m_PerDrawData.materialIndex = material.GetConstantBufferIndex();

			m_PerDrawData.worldMatrixIndex = memoryManager.GetCBDescriptorIndex(tc.ConstantBufferID);

			DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRoot32BitConstants(perDrawIndex, count, &m_PerDrawData, 0u));

			RenderCommand::DrawInstanced(mesh.GetIndexBuffer()->GetNrOfIndices());
		}

		MasterRenderer::EndRenderPass();
	}

	void SceneRenderer::WireframePass() noexcept
	{
		PROFILE_FUNC;
		EntityManager& entityManager = m_pScene->GetEntityManager();
		if (entityManager.GetEntityCountForPool<SelectedInEditorComponent>() == 0 || !m_Options.DisplaySelectionWireframe)
		{
			return;
		}

		MemoryManager& memoryManager = MemoryManager::Get();

		MasterRenderer::BeginRenderPass(m_WireFrameRenderPass);

		m_WireFrameRenderPass->Upload("vpConstantBuffer", &m_VPData);

		MeshManager& m = AssetManager::GetMeshManager();

		auto verticesIndex = m_WireFrameRenderPass->GetInputSlot("vertices");
		auto indicesIndex = m_WireFrameRenderPass->GetInputSlot("indices");
		auto perDrawIndex = m_WireFrameRenderPass->GetInputSlot("perDrawData");

		const uint32_t count = (uint32_t)sizeof(PerDrawData) / sizeof(uint32_t);

		entityManager.Collect<OpaquePassComponent, SelectedInEditorComponent, MeshFilterComponent, MeshRendererComponent>().Do([&](entity e, MeshFilterComponent& mfc)
			{
				if (mfc.MeshHandle != NULL_HANDLE)
				{
					Mesh& mesh = m.GetMesh(mfc.MeshHandle);

					DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRootShaderResourceView(verticesIndex, mesh.GetVertexBuffer()->GetInterface()->GetGPUVirtualAddress()));
					DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRootShaderResourceView(indicesIndex, mesh.GetIndexBuffer()->GetInterface()->GetGPUVirtualAddress()));

					auto& mrc = entityManager.Get<MeshRendererComponent>(e);
					const Material& material = AssetManager::Get<Material>(mrc.MaterialHandle);
					m_PerDrawData.materialIndex = material.GetConstantBufferIndex();

					m_PerDrawData.worldMatrixIndex = memoryManager.GetCBDescriptorIndex(entityManager.Get<TransformComponent>(e).ConstantBufferID);

					DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRoot32BitConstants(perDrawIndex, count, &m_PerDrawData, 0u));

					RenderCommand::DrawInstanced(mesh.GetIndexBuffer()->GetNrOfIndices());
				}
			});

		MasterRenderer::EndRenderPass();
	}

	void SceneRenderer::EditorGridPass() noexcept
	{
		if (!m_Options.DisplayEditorGrid)
			return;

		PROFILE_FUNC;
		MemoryManager& memoryManager = MemoryManager::Get();

		MasterRenderer::BeginRenderPass(m_EditorGridRenderPass);

		m_EditorGridRenderPass->Upload("vpConstantBuffer", &m_VPData);

		auto frameIndex = MasterRenderer::GetCurrentFrameIndex();
		m_PerFrameEditorData.cameraDataIndex = m_pScene->GetEditorCamera()->m_pConstantBuffer->m_VisibleHandles[frameIndex].Index;
		m_EditorGridRenderPass->Upload("perFrameData", &m_PerFrameEditorData);

		auto verticesIndex = m_EditorGridRenderPass->GetInputSlot("vertices");
		auto batchIndex = m_EditorGridRenderPass->GetInputSlot("batchData");
		auto gridInstanceDataSBIndex = m_EditorGridRenderPass->GetInputSlot("instanceDataSBIndex");

		m_InstanceDataSBIndex.Index = m_pEditorGridInstanceDataStructuredBuffer->m_VisibleHandles[frameIndex].Index;
		DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRootShaderResourceView(verticesIndex, m_pEditorGridVertexBuffer->GetInterface()->GetGPUVirtualAddress()));
		DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRoot32BitConstants(gridInstanceDataSBIndex, 1, &m_InstanceDataSBIndex, 0u));

		DirectX::XMFLOAT3 offset = m_pScene->GetEditorCamera()->GetPosition();
		offset.x = static_cast<float>(std::floor(offset.x - fmod(offset.x, 100.0)));
		offset.z = static_cast<float>(std::floor(offset.z - fmod(offset.z, 100.0)));
		
		//Note: Could actually be just one constant buffer:
		DirectX::XMMATRIX world = DirectX::XMMatrixScaling(10000.0f, 1.0f, 1.0f) * DirectX::XMMatrixTranslation(offset.x, 0.0f, 200.0f + offset.z);
		DirectX::XMStoreFloat4x4(&m_EditorGridTransformComponent1.Transform, world);
		MemoryManager::Get().UpdateConstantBuffer(m_EditorGridTransformComponent1.ConstantBufferID, &m_EditorGridTransformComponent1.Transform);

		world = DirectX::XMMatrixScaling(10000.0f, 1.0f, 1.0f) * DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(90.0f)) * DirectX::XMMatrixTranslation(offset.x - 200.0f, 0.0f, offset.z);
		DirectX::XMStoreFloat4x4(&m_EditorGridTransformComponent2.Transform, world);
		MemoryManager::Get().UpdateConstantBuffer(m_EditorGridTransformComponent2.ConstantBufferID, &m_EditorGridTransformComponent2.Transform);

		m_EditorBatchData.worldMatrixIndex1 = memoryManager.GetCBDescriptorIndex(m_EditorGridTransformComponent1.ConstantBufferID);
		m_EditorBatchData.worldMatrixIndex2 = memoryManager.GetCBDescriptorIndex(m_EditorGridTransformComponent2.ConstantBufferID);
		DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRoot32BitConstants(batchIndex, (uint32_t)sizeof(EditorBatchData) / sizeof(uint32_t), &m_EditorBatchData, 0u));

		RenderCommand::DrawInstanced(EDITOR_GRID_VERTEX_COUNT, EDITOR_GRID_INSTANCE_COUNT);

		MasterRenderer::EndRenderPass();
	}

	void SceneRenderer::PickingPass() noexcept
	{
		PROFILE_FUNC;

		MasterRenderer::BeginRenderPass(m_GeometryPickingRenderPass);

		EntityManager& entityManager = m_pScene->GetEntityManager();
		MemoryManager& memoryManager = MemoryManager::Get();

		auto verticesIndex = m_GeometryPickingRenderPass->GetInputSlot("vertices");
		auto indicesIndex = m_GeometryPickingRenderPass->GetInputSlot("indices");
		auto identifierIndex = m_GeometryPickingRenderPass->GetInputSlot("Identifier");
		auto perDrawIndex = m_GeometryPickingRenderPass->GetInputSlot("perDrawData");

		const uint32_t count = (uint32_t)sizeof(PerDrawData) / sizeof(uint32_t);

		entityManager.Collect<OpaquePassComponent, MeshFilterComponent, MeshRendererComponent>().Do([&](entity e, MeshFilterComponent& mfc)
			{
				if (mfc.MeshHandle != NULL_HANDLE)
				{
					Mesh& mesh = AssetManager::Get<Mesh>(mfc.MeshHandle);

					DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRootShaderResourceView(verticesIndex, mesh.GetVertexBuffer()->GetInterface()->GetGPUVirtualAddress()));
					DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRootShaderResourceView(indicesIndex, mesh.GetIndexBuffer()->GetInterface()->GetGPUVirtualAddress()));

					m_PickingData.entityID = e;
					DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRoot32BitConstants(identifierIndex, 1u, &m_PickingData, 0u));

					auto& mrc = entityManager.Get<MeshRendererComponent>(e);
					const Material& material = AssetManager::Get<Material>(mrc.MaterialHandle);
					m_PerDrawData.materialIndex = material.GetConstantBufferIndex();
					m_PerDrawData.worldMatrixIndex = memoryManager.GetCBDescriptorIndex(entityManager.Get<TransformComponent>(e).ConstantBufferID);
					DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRoot32BitConstants(perDrawIndex, count, &m_PerDrawData, 0u));

					RenderCommand::DrawInstanced(mesh.GetIndexBuffer()->GetNrOfIndices());
				}
			});

		MasterRenderer::EndRenderPass();
		
		auto mousePos = m_pScene->GetMousePosition();
		if (mousePos.x > 0.0f
			&& (uint32_t)mousePos.x < m_GeometryPickingRenderPass->GetOutput(0)->GetWidth()
			&& mousePos.y > 0.0f && (uint32_t)mousePos.y < m_GeometryPickingRenderPass->GetOutput(0)->GetHeight())
		{
			RenderCommand::CopyTexelToBuffer(m_GeometryPickingRenderPass->GetOutput(0), m_pIdentifierReadbackBuffer, static_cast<uint32_t>(mousePos.x), static_cast<uint32_t>(mousePos.y), sizeof(uint32_t));
		}
	}

	void SceneRenderer::CompositePass() noexcept
	{
		PROFILE_FUNC;
		
		MasterRenderer::BeginRenderPass(m_CompositeRenderPass);

		if (m_Options.MSAASamples > 1)
		{
			//RenderCommand::TransitionResource(m_pResolvedTexture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			m_CompositeData.PostProcessTextureIndex = m_pResolvedTexture->GetSRVDescriptorHandle().Index;
		}
		else
		{
			RenderCommand::TransitionResource(m_CombinedGeometryAndPickingPass->GetOutput(0), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			m_CompositeData.PostProcessTextureIndex = m_CombinedGeometryAndPickingPass->GetOutput(0)->GetSRVDescriptorHandle().Index;
		}
		
		auto textureDataIndex = m_CompositeRenderPass->GetInputSlot("TextureData");
		DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRoot32BitConstants(textureDataIndex, 1u, &m_CompositeData, 0u));

		RenderCommand::DrawInstanced(3u);

		MasterRenderer::EndRenderPass();
	}

	void SceneRenderer::CombinedGeometryAndPickingPass() noexcept
	{
		PROFILE_FUNC;

		MasterRenderer::BeginRenderPass(m_CombinedGeometryAndPickingPass);

		MemoryManager& memoryManager = MemoryManager::Get();

		m_CombinedGeometryAndPickingPass->Upload("vpConstantBuffer", &m_VPData);
		m_CombinedGeometryAndPickingPass->Upload("perFrameData", &m_PerFrameOpaqueGeometryData);

		EntityManager& entityManager = m_pScene->GetEntityManager();

		auto verticesIndex = m_CombinedGeometryAndPickingPass->GetInputSlot("vertices");
		auto indicesIndex = m_CombinedGeometryAndPickingPass->GetInputSlot("indices");
		auto perDrawIndex = m_CombinedGeometryAndPickingPass->GetInputSlot("perDrawData");
		auto identifierIndex = m_CombinedGeometryAndPickingPass->GetInputSlot("Identifier");

		entityManager.Collect<OpaquePassComponent, MeshFilterComponent, MeshRendererComponent>().Do([&](entity e, MeshFilterComponent& mfc)
			{
				if (mfc.MeshHandle != NULL_HANDLE)
				{
					Mesh& mesh = AssetManager::Get<Mesh>(mfc.MeshHandle);

					DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRootShaderResourceView(verticesIndex, mesh.GetVertexBuffer()->GetInterface()->GetGPUVirtualAddress()));
					DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRootShaderResourceView(indicesIndex, mesh.GetIndexBuffer()->GetInterface()->GetGPUVirtualAddress()));

					m_PickingData.entityID = e;
					DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRoot32BitConstants(identifierIndex, 1u, &m_PickingData, 0u));

					auto& mrc = entityManager.Get<MeshRendererComponent>(e);
					const Material& material = AssetManager::Get<Material>(mrc.MaterialHandle);
					m_PerDrawData.materialIndex = material.GetConstantBufferIndex();
					m_PerDrawData.worldMatrixIndex = memoryManager.GetCBDescriptorIndex(entityManager.Get<TransformComponent>(e).ConstantBufferID);

					DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRoot32BitConstants(perDrawIndex, (uint32_t)sizeof(PerDrawData) / sizeof(uint32_t), &m_PerDrawData, 0u));

					RenderCommand::DrawInstanced(mesh.GetIndexBuffer()->GetNrOfIndices());
				}
			});
		
		MasterRenderer::EndRenderPass();

		auto mousePos = m_pScene->GetMousePosition();
		if (mousePos.x > 0.0f
			&& (uint32_t)mousePos.x < m_CombinedGeometryAndPickingPass->GetOutput(0)->GetWidth()
			&& mousePos.y > 0.0f && (uint32_t)mousePos.y < m_CombinedGeometryAndPickingPass->GetOutput(0)->GetHeight())
		{
			RenderCommand::CopyTexelToBuffer(m_CombinedGeometryAndPickingPass->GetOutput(1), m_pIdentifierReadbackBuffer, static_cast<uint32_t>(mousePos.x), static_cast<uint32_t>(mousePos.y), sizeof(uint32_t));
		}
	}

	void SceneRenderer::AddToResourceTransitionBatch(const std::shared_ptr<IResource>& pResource, D3D12_RESOURCE_STATES stateAfter) noexcept
	{
		D3D12_RESOURCE_BARRIER resourceTransitionBarrier{};
		resourceTransitionBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		resourceTransitionBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		resourceTransitionBarrier.Transition.pResource = pResource->GetInterface().Get();
		resourceTransitionBarrier.Transition.StateBefore = pResource->GetCurrentState();
		resourceTransitionBarrier.Transition.StateAfter = stateAfter;
		resourceTransitionBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		m_ResourceBarrierBatch.emplace_back(resourceTransitionBarrier);

		pResource->SetCurrentState(stateAfter);
	}

	void SceneRenderer::AddToResourceTransitionBatch(const Microsoft::WRL::ComPtr<ID3D12Resource>& pResource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter) noexcept
	{
		D3D12_RESOURCE_BARRIER resourceTransitionBarrier{};
		resourceTransitionBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		resourceTransitionBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		resourceTransitionBarrier.Transition.pResource = pResource.Get();
		resourceTransitionBarrier.Transition.StateBefore = stateBefore;
		resourceTransitionBarrier.Transition.StateAfter = stateAfter;
		resourceTransitionBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		m_ResourceBarrierBatch.emplace_back(resourceTransitionBarrier);
	}

	void SceneRenderer::OnSceneViewportChanged(const uint32_t width, const uint32_t height) noexcept
	{
		MasterRenderer::Resize(width, height);
		m_PreZRenderPass->Resize(width, height);
		m_OpaqueGeometryRenderPass->Resize(width, height);
		m_CutOutGeometryRenderPass->Resize(width, height);
		m_TransparentGeometryRenderPass->Resize(width, height);
		m_CombinedGeometryAndPickingPass->Resize(width, height);
		m_WireFrameRenderPass->Resize(width, height);
		m_EditorGridRenderPass->Resize(width, height);
		m_GeometryPickingRenderPass->Resize(width, height);
		m_CompositeRenderPass->Resize(width, height);

		auto& memoryManager = MemoryManager::Get();
		
		RenderTextureSpecification resolveRTSpec{};
		resolveRTSpec.MultiSampleCount = 1u;
		resolveRTSpec.CreateSRV = true;
		resolveRTSpec.Width = width;
		resolveRTSpec.Height = height;
		resolveRTSpec.Flags = D3D12_RESOURCE_FLAG_NONE;
		resolveRTSpec.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		memoryManager.DestroyDescriptorHandle(m_pResolvedTexture->GetSRVDescriptorHandle());
		memoryManager.DestroyResource(std::move(m_pResolvedTexture));
		m_pResolvedTexture = RenderTexture::Create(resolveRTSpec, "MSAA Resolve RenderTexture");
	}

	void SceneRenderer::SetMSAASamples(const uint8_t samples) noexcept
	{
		if (samples != m_Options.MSAASamples)
		{
			MasterRenderer::WaitAndSyncAllFramesInFlight();
			if (samples == 1)
			{
				m_EditorGridRenderPass->GetPipeline()->GetFrameBuffer()->SetOutputDependency(m_CombinedGeometryAndPickingPass->GetPipeline()->GetFrameBuffer(), 0);
				m_EditorGridRenderPass->GetPipeline()->GetFrameBuffer()->SetDepthDependency(m_CombinedGeometryAndPickingPass->GetPipeline()->GetFrameBuffer());
				m_WireFrameRenderPass->GetPipeline()->GetFrameBuffer()->SetOutputDependency(m_CombinedGeometryAndPickingPass->GetPipeline()->GetFrameBuffer(), 0);
				m_WireFrameRenderPass->GetPipeline()->GetFrameBuffer()->SetDepthDependency(m_CombinedGeometryAndPickingPass->GetPipeline()->GetFrameBuffer());
			}
			else if (m_Options.MSAASamples == 1)
			{
				m_EditorGridRenderPass->GetPipeline()->GetFrameBuffer()->SetOutputDependency(m_OpaqueGeometryRenderPass->GetPipeline()->GetFrameBuffer(), 0);
				m_EditorGridRenderPass->GetPipeline()->GetFrameBuffer()->SetDepthDependency(m_OpaqueGeometryRenderPass->GetPipeline()->GetFrameBuffer());
				m_WireFrameRenderPass->GetPipeline()->GetFrameBuffer()->SetOutputDependency(m_OpaqueGeometryRenderPass->GetPipeline()->GetFrameBuffer(), 0);
				m_WireFrameRenderPass->GetPipeline()->GetFrameBuffer()->SetDepthDependency(m_OpaqueGeometryRenderPass->GetPipeline()->GetFrameBuffer());
			}

			m_Options.MSAASamples = samples;

			m_PreZRenderPass->OnMSAAReconfiguration(samples);
			m_OpaqueGeometryRenderPass->OnMSAAReconfiguration(samples);
			m_CutOutGeometryRenderPass->OnMSAAReconfiguration(samples);
			m_TransparentGeometryRenderPass->OnMSAAReconfiguration(samples);
			m_CombinedGeometryAndPickingPass->OnMSAAReconfiguration(samples);
			m_WireFrameRenderPass->OnMSAAReconfiguration(samples);
			m_GeometryPickingRenderPass->OnMSAAReconfiguration(samples);
			m_EditorGridRenderPass->OnMSAAReconfiguration(samples);
			m_CompositeRenderPass->OnMSAAReconfiguration(samples);
		}
	}

	entity SceneRenderer::GetHoveredEntity() noexcept
	{
		uint32_t* pEntity = nullptr;
		DXCall(m_pIdentifierReadbackBuffer->GetInterface()->Map(0, nullptr, reinterpret_cast<void**>(&pEntity)));
		DXCall_STD(m_pIdentifierReadbackBuffer->GetInterface()->Unmap(0, nullptr));

		return *pEntity;
	}

	void SceneRenderer::InitializeHBAOPlus() noexcept
	{
		GFSDK_SSAO_CustomHeap CustomHeap;
		CustomHeap.new_ = ::operator new;
		CustomHeap.delete_ = ::operator delete;

		GFSDK_SSAO_DescriptorHeaps_D3D12 DescriptorHeaps;
		DescriptorHeaps.CBV_SRV_UAV.pDescHeap = MemoryManager::Get().GetShaderBindableDescriptorHeap()->GetDescriptorHeapInterface().Get();
		DescriptorHeaps.CBV_SRV_UAV.BaseIndex = 10'000; //TEMP

		DescriptorHeaps.RTV.pDescHeap = MemoryManager::Get().GetRTVDescriptorHeap()->GetDescriptorHeapInterface().Get();
		DescriptorHeaps.RTV.BaseIndex = 10'000; //TEMP

#if defined RLS_DEBUG
		GFSDK_SSAO_Status status = GFSDK_SSAO_CreateContext_D3D12
		(
			D3D12Core::GetDevice().Get(), 
			1u, 
			DescriptorHeaps, 
			&m_SSAOContext,
			&CustomHeap
		);
		RLS_ASSERT(status == GFSDK_SSAO_OK, "Failed to initialize HBAO+.");
#else
		GFSDK_SSAO_CreateContext_D3D12
		(
			D3D12Core::GetDevice().Get(),
			1u,
			DescriptorHeaps,
			&m_SSAOContext,
			&CustomHeap
		);
#endif

		m_HBAOPlusParameters.Radius = 2.f;
		m_HBAOPlusParameters.Bias = 0.1f;
		m_HBAOPlusParameters.PowerExponent = 2.f;
		m_HBAOPlusParameters.Blur.Enable = true;
		m_HBAOPlusParameters.Blur.Sharpness = 16.f;
		m_HBAOPlusParameters.Blur.Radius = GFSDK_SSAO_BLUR_RADIUS_4;
		m_HBAOPlusParameters.DepthStorage = GFSDK_SSAO_FP32_VIEW_DEPTHS;
		m_HBAOPlusParameters.EnableDualLayerAO = false;
		m_HBAOPlusParameters.StepCount = GFSDK_SSAO_STEP_COUNT_8;
	}
}