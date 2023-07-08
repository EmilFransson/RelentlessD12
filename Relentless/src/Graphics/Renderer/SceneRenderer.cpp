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
		//Geometry Render pass:
		{
			ColorAttachment colorAttachment;
			colorAttachment.Format = TextureFormat::RGBA32F;
			colorAttachment.ClearColor = DirectX::XMFLOAT4(DirectX::Colors::CornflowerBlue);
			colorAttachment.Transfer = true;

			DepthAttachment depthAttachment;
			depthAttachment.Format = TextureFormat::Depth;

			FrameBufferSpecification frameBufferSpecification{};
			frameBufferSpecification.DebugName = "Geometry Framebuffer";
			frameBufferSpecification.Attachments.ColorAttachments = { colorAttachment };
			frameBufferSpecification.Attachments.DepthAttachment = depthAttachment;
			frameBufferSpecification.MSAASamples = 8u;

			PipelineSpecification pipelineSpecification{};
			pipelineSpecification.DebugName = "Geometry Pipeline";
			pipelineSpecification.pVertexShader = MasterRenderer::GetShaderLibrary().Get("VertexShader");
			pipelineSpecification.pPixelShader = MasterRenderer::GetShaderLibrary().Get("PBRPixelShader");
			pipelineSpecification.pFrameBuffer = FrameBuffer::Create(frameBufferSpecification);
			pipelineSpecification.MSAAEligible = true;

			RenderPassSpecification renderpassDescriptor{};
			renderpassDescriptor.DebugName = "Geometry Pass";
			renderpassDescriptor.RenderPipeline = Pipeline::Create(pipelineSpecification);

			m_GeometryRenderPass = RenderPass::Create(renderpassDescriptor);
		}

		//Combined Geometry and Picking Render pass:
		{
			ColorAttachment colorAttachment;
			colorAttachment.Format = TextureFormat::RGBA32F;
			colorAttachment.ClearColor = DirectX::XMFLOAT4(DirectX::Colors::CornflowerBlue);
			colorAttachment.Transfer = true;

			ColorAttachment colorAttachment2;
			colorAttachment2.Format = TextureFormat::R32UINT;
			static constexpr float clearVal = static_cast<float>(NULL_ENTITY);
			colorAttachment2.ClearColor = DirectX::XMFLOAT4(clearVal, clearVal, clearVal, clearVal);
			colorAttachment2.Transfer = true;
			colorAttachment2.IsSRGB = false;

			DepthAttachment depthAttachment;
			depthAttachment.Format = TextureFormat::Depth;

			FrameBufferSpecification frameBufferSpecification{};
			frameBufferSpecification.DebugName = "GeometryAndPicking Framebuffer";
			frameBufferSpecification.Attachments.ColorAttachments = { colorAttachment, colorAttachment2 };
			frameBufferSpecification.Attachments.DepthAttachment = depthAttachment;

			PipelineSpecification pipelineSpecification{};
			pipelineSpecification.DebugName = "GeometryAndPicking Pipeline";
			pipelineSpecification.pVertexShader = MasterRenderer::GetShaderLibrary().Get("VertexShader");
			pipelineSpecification.pPixelShader = MasterRenderer::GetShaderLibrary().Get("GeometryAndPickingPixelShader");
			pipelineSpecification.pFrameBuffer = FrameBuffer::Create(frameBufferSpecification);
			pipelineSpecification.MSAAEligible = false;

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
			colorAttachment.Output = m_GeometryRenderPass->GetOutput(0);
			colorAttachment.pOutputDependency = m_GeometryRenderPass->GetPipeline()->GetFrameBuffer();

			DepthAttachment depthAttachment;
			depthAttachment.Format = TextureFormat::Depth;
			depthAttachment.OperatorOnLoad = OperatorOnLoad::LoadOnly;
			depthAttachment.Output = m_GeometryRenderPass->GetDepthOutput();
			depthAttachment.pOutputDependency = m_GeometryRenderPass->GetPipeline()->GetFrameBuffer();

			FrameBufferSpecification frameBufferSpecification{};
			frameBufferSpecification.DebugName = "Wireframe Framebuffer";
			frameBufferSpecification.Attachments.ColorAttachments = { colorAttachment };
			frameBufferSpecification.Attachments.DepthAttachment = { depthAttachment };
			frameBufferSpecification.DepthComparisonFunction = DepthComparisonFunction::ALWAYS;
			frameBufferSpecification.ShouldResize = false;

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

			m_WireFrameRenderPass= RenderPass::Create(renderpassDescriptor);
		}

		//Picking:
		{
			ColorAttachment colorAttachment;
			colorAttachment.Format = TextureFormat::R32UINT;
			static constexpr float clearVal = static_cast<float>(NULL_ENTITY);
			colorAttachment.ClearColor = DirectX::XMFLOAT4(clearVal, clearVal, clearVal, clearVal);
			colorAttachment.Transfer = true;
			colorAttachment.IsSRGB = false;

			DepthAttachment depthAttachment;
			depthAttachment.Format = TextureFormat::Depth;

			FrameBufferSpecification frameBufferSpecification{};
			frameBufferSpecification.DebugName = "Picking Framebuffer";
			frameBufferSpecification.Attachments.ColorAttachments = { colorAttachment };
			frameBufferSpecification.Attachments.DepthAttachment = { depthAttachment };

			PipelineSpecification pipelineSpecification{};
			pipelineSpecification.DebugName = "Picking Pipeline";
			pipelineSpecification.pVertexShader = MasterRenderer::GetShaderLibrary().Get("VertexShader");
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
			colorAttachment.Output = m_GeometryRenderPass->GetOutput(0);
			colorAttachment.pOutputDependency = m_GeometryRenderPass->GetPipeline()->GetFrameBuffer();

			DepthAttachment depthAttachment;
			depthAttachment.Format = TextureFormat::Depth;
			depthAttachment.OperatorOnLoad = OperatorOnLoad::LoadOnly;
			depthAttachment.Output = m_GeometryRenderPass->GetPipeline()->GetFrameBuffer()->GetDepthOutput();
			depthAttachment.pOutputDependency = m_GeometryRenderPass->GetPipeline()->GetFrameBuffer();

			FrameBufferSpecification frameBufferSpecification{};
			frameBufferSpecification.DebugName = "Editor Grid Framebuffer";
			frameBufferSpecification.Attachments.ColorAttachments = { colorAttachment };
			frameBufferSpecification.Attachments.DepthAttachment = depthAttachment;
			frameBufferSpecification.MSAASamples = 8u;
			frameBufferSpecification.DepthComparisonFunction = DepthComparisonFunction::LESS_EQUAL;
			frameBufferSpecification.ShouldResize = false;

			PipelineSpecification pipelineSpecification{};
			pipelineSpecification.DebugName = "Editor Grid Pipeline";
			pipelineSpecification.pVertexShader = MasterRenderer::GetShaderLibrary().Get("VertexShaderEditorGrid");
			pipelineSpecification.pPixelShader = MasterRenderer::GetShaderLibrary().Get("PixelShaderEditorGrid");
			pipelineSpecification.pFrameBuffer = FrameBuffer::Create(frameBufferSpecification);
			pipelineSpecification.Topology = Topology::Line;
			pipelineSpecification.MSAAEligible = true;

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
			colorAttachment.Transfer = true;
			colorAttachment.Output = MasterRenderer::GetFrameBuffer()->GetOutput(0);
			colorAttachment.pOutputDependency = MasterRenderer::GetFrameBuffer();

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

		MasterRenderer::Begin();

		RenderCommand::SetDescriptorHeap(MemoryManager::Get().GetShaderBindableDescriptorHeap());
		RenderCommand::SetViewport(m_pScene->GetViewport());
		RenderCommand::SetScissorRect(m_pScene->GetScissorRect());
	}

	void SceneRenderer::IssueRenderPasses() noexcept
	{
		PROFILE_FUNC;

		if (m_Options.MSAASamples > 1)
		{
			GeometryPass();
			PickingPass();
		}
		else
		{
			CombinedGeometryAndPickingPass();
		}
		EditorGridPass();
		WireframePass();

		if (m_Options.MSAASamples > 1)
		{
			//MSAA Resolve:
			auto pGeometryPassColorOutput = m_GeometryRenderPass->GetOutput(0);
			RenderCommand::TransitionResource(pGeometryPassColorOutput, D3D12_RESOURCE_STATE_RESOLVE_SOURCE);
			RenderCommand::TransitionResource(m_pResolvedTexture, D3D12_RESOURCE_STATE_RESOLVE_DEST);
			RenderCommand::ResolveMSAA(pGeometryPassColorOutput, m_pResolvedTexture);
		}

		CompositePass();

		//Set UI-texture as pixel shader resource and prepare back buffer as render target for imgui:
		{
			BackBuffer backBuffer = Window::GetCurrentBackBuffer();
			RenderCommand::TransitionResource(backBuffer.pBackBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
			RenderCommand::TransitionResource(MasterRenderer::GetFrameBuffer()->GetOutput(0), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			RenderCommand::SetRenderTarget(backBuffer);
		}
	}

	void SceneRenderer::End() noexcept
	{
		MasterRenderer::End();
	}

	void SceneRenderer::GeometryPass() noexcept
	{
		PROFILE_FUNC;
		MemoryManager& memoryManager = MemoryManager::Get();

		MasterRenderer::BeginRenderPass(m_GeometryRenderPass);

		//Terrible and wrong for runtime:
		//Camera:
		auto vpMatrix = DirectX::XMLoadFloat4x4(&(m_pScene->GetEditorCamera()->GetViewProjectionMatrix()));
		DirectX::XMStoreFloat4x4(&m_VPData.VPMatrix, vpMatrix);
		m_GeometryRenderPass->Upload("vpConstantBuffer", &m_VPData);

		auto frameIndex = MasterRenderer::GetCurrentFrameIndex();

		m_PerFrameOpaqueGeometryData.cameraDataIndex = m_pScene->GetEditorCamera()->m_pConstantBuffer->m_VisibleHandles[frameIndex].Index;
		m_PerFrameOpaqueGeometryData.pointLightStructuredBufferIndex = m_pScene->GetLightManager().GetPointLights()->m_VisibleHandles[frameIndex].Index;
		m_PerFrameOpaqueGeometryData.directionalLightStructuredBufferIndex = m_pScene->GetLightManager().GetDirectionalLights()->m_VisibleHandles[frameIndex].Index;
		m_PerFrameOpaqueGeometryData.nrOfDirectionalLights = static_cast<uint32_t>(m_pScene->GetEntityManager().GetEntityCountForPool<DirectionalLightComponent>());
		m_PerFrameOpaqueGeometryData.nrOfPointLights = static_cast<uint32_t>(m_pScene->GetEntityManager().GetEntityCountForPool<PointLightComponent>());
		m_GeometryRenderPass->Upload("perFrameData", &m_PerFrameOpaqueGeometryData);

		EntityManager& entityManager = m_pScene->GetEntityManager();
		AssetManager& assetManager = AssetManager::Get();

		auto verticesIndex = m_GeometryRenderPass->GetInputSlot("vertices");
		auto indicesIndex = m_GeometryRenderPass->GetInputSlot("indices");
		auto perDrawIndex = m_GeometryRenderPass->GetInputSlot("perDrawData");

		entityManager.Collect<ForwardPassComponent, MeshFilterComponent, MeshRendererComponent>().Do([&](entity e, MeshFilterComponent& mfc)
			{
				if (assetManager.Exists(mfc.VertexBufferID))
				{
					VertexBuffer* vb = assetManager.GetAsset<VertexBuffer>(mfc.VertexBufferID);
					IndexBuffer* ib = assetManager.GetAsset<IndexBuffer>(mfc.IndexBufferID);
		
					DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRootShaderResourceView(verticesIndex, vb->GetInterface()->GetGPUVirtualAddress()));
					DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRootShaderResourceView(indicesIndex, ib->GetInterface()->GetGPUVirtualAddress()));
		
					auto& mrc = entityManager.Get<MeshRendererComponent>(e);
					const Material& material = AssetManager::Get().Get<Material>(mrc.MaterialHandle);
					m_PerDrawData.materialIndex = material.GetConstantBufferIndex();
					
					m_PerDrawData.worldMatrixIndex = memoryManager.GetCBDescriptorIndex(entityManager.Get<TransformComponent>(e).ConstantBufferID);
		
					DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRoot32BitConstants(perDrawIndex, (uint32_t)sizeof(PerDrawData) / sizeof(uint32_t), &m_PerDrawData, 0u));

					RenderCommand::DrawInstanced(ib->GetNrOfIndices());
				}
			});

		MasterRenderer::EndRenderPass();
	}

	void SceneRenderer::WireframePass() noexcept
	{
		PROFILE_FUNC;
		EntityManager& entityManager = m_pScene->GetEntityManager();
		if (entityManager.GetEntityCountForPool<SelectedInEditorComponent>() == 0)
		{
			return;
		}

		MemoryManager& memoryManager = MemoryManager::Get();

		MasterRenderer::BeginRenderPass(m_WireFrameRenderPass);

		//Terrible and wrong for runtime:
		//Camera:
		auto vpMatrix = DirectX::XMLoadFloat4x4(&(m_pScene->GetEditorCamera()->GetViewProjectionMatrix()));
		DirectX::XMStoreFloat4x4(&m_VPData.VPMatrix, vpMatrix);
		m_WireFrameRenderPass->Upload("vpConstantBuffer", &m_VPData);

		AssetManager& assetManager = AssetManager::Get();

		auto verticesIndex = m_WireFrameRenderPass->GetInputSlot("vertices");
		auto indicesIndex = m_WireFrameRenderPass->GetInputSlot("indices");
		auto perDrawIndex = m_WireFrameRenderPass->GetInputSlot("perDrawData");

		entityManager.Collect<ForwardPassComponent, SelectedInEditorComponent, MeshFilterComponent, MeshRendererComponent>().Do([&](entity e, MeshFilterComponent& mfc)
			{
				if (assetManager.Exists(mfc.VertexBufferID))
				{
					VertexBuffer* vb = assetManager.GetAsset<VertexBuffer>(mfc.VertexBufferID);
					IndexBuffer* ib = assetManager.GetAsset<IndexBuffer>(mfc.IndexBufferID);

					DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRootShaderResourceView(verticesIndex, vb->GetInterface()->GetGPUVirtualAddress()));
					DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRootShaderResourceView(indicesIndex, ib->GetInterface()->GetGPUVirtualAddress()));

					auto& mrc = entityManager.Get<MeshRendererComponent>(e);
					const Material& material = AssetManager::Get().Get<Material>(mrc.MaterialHandle);
					m_PerDrawData.materialIndex = material.GetConstantBufferIndex();
					m_PerDrawData.worldMatrixIndex = memoryManager.GetCBDescriptorIndex(entityManager.Get<TransformComponent>(e).ConstantBufferID);

					DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRoot32BitConstants(perDrawIndex, (uint32_t)sizeof(PerDrawData) / sizeof(uint32_t), &m_PerDrawData, 0u));

					RenderCommand::DrawInstanced(ib->GetNrOfIndices());
				}
			});

		MasterRenderer::EndRenderPass();

		
	}

	void SceneRenderer::EditorGridPass() noexcept
	{
		PROFILE_FUNC;
		MemoryManager& memoryManager = MemoryManager::Get();

		MasterRenderer::BeginRenderPass(m_EditorGridRenderPass);

		//Terrible and wrong for runtime:
		//Camera:
		auto vpMatrix = DirectX::XMLoadFloat4x4(&(m_pScene->GetEditorCamera()->GetViewProjectionMatrix()));
		DirectX::XMStoreFloat4x4(&m_VPData.VPMatrix, vpMatrix);
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
		AssetManager& assetManager = AssetManager::Get();
		MemoryManager& memoryManager = MemoryManager::Get();

		auto verticesIndex = m_GeometryPickingRenderPass->GetInputSlot("vertices");
		auto indicesIndex = m_GeometryPickingRenderPass->GetInputSlot("indices");
		auto identifierIndex = m_GeometryPickingRenderPass->GetInputSlot("Identifier");
		auto perDrawIndex = m_GeometryPickingRenderPass->GetInputSlot("perDrawData");

		entityManager.Collect<ForwardPassComponent, MeshFilterComponent, MeshRendererComponent>().Do([&](entity e, MeshFilterComponent& mfc)
			{
				if (assetManager.Exists(mfc.VertexBufferID))
				{
					VertexBuffer* vb = assetManager.GetAsset<VertexBuffer>(mfc.VertexBufferID);
					IndexBuffer* ib = assetManager.GetAsset<IndexBuffer>(mfc.IndexBufferID);
		
					DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRootShaderResourceView(verticesIndex, vb->GetInterface()->GetGPUVirtualAddress()));
					DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRootShaderResourceView(indicesIndex, ib->GetInterface()->GetGPUVirtualAddress()));

					m_PickingData.entityID = e;
					DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRoot32BitConstants(identifierIndex, 1u, &m_PickingData, 0u));

					auto& mrc = entityManager.Get<MeshRendererComponent>(e);
					const Material& material = AssetManager::Get().Get<Material>(mrc.MaterialHandle);
					m_PerDrawData.materialIndex = material.GetConstantBufferIndex();
					m_PerDrawData.worldMatrixIndex = memoryManager.GetCBDescriptorIndex(entityManager.Get<TransformComponent>(e).ConstantBufferID);
					DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRoot32BitConstants(perDrawIndex, (uint32_t)sizeof(PerDrawData) / sizeof(uint32_t), &m_PerDrawData, 0u));

					RenderCommand::DrawInstanced(ib->GetNrOfIndices());
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


		auto textureDataIndex = m_CompositeRenderPass->GetInputSlot("TextureData");
		if (m_Options.MSAASamples > 1)
		{
			RenderCommand::TransitionResource(m_pResolvedTexture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			m_CompositeData.PostProcessTextureIndex = m_pResolvedTexture->GetSRVDescriptorHandle().Index;
		}
		else
		{
			RenderCommand::TransitionResource(m_CombinedGeometryAndPickingPass->GetOutput(0), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			m_CompositeData.PostProcessTextureIndex = m_CombinedGeometryAndPickingPass->GetOutput(0)->GetSRVDescriptorHandle().Index;
		}
		DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRoot32BitConstants(textureDataIndex, 1u, &m_CompositeData, 0u));

		RenderCommand::DrawInstanced(3u);

		MasterRenderer::EndRenderPass();
	}

	void SceneRenderer::CombinedGeometryAndPickingPass() noexcept
	{
		PROFILE_FUNC;

		MasterRenderer::BeginRenderPass(m_CombinedGeometryAndPickingPass);

		MemoryManager& memoryManager = MemoryManager::Get();

		//Terrible and wrong for runtime:
		//Camera:
		auto vpMatrix = DirectX::XMLoadFloat4x4(&(m_pScene->GetEditorCamera()->GetViewProjectionMatrix()));
		DirectX::XMStoreFloat4x4(&m_VPData.VPMatrix, vpMatrix);
		m_CombinedGeometryAndPickingPass->Upload("vpConstantBuffer", &m_VPData);

		auto frameIndex = MasterRenderer::GetCurrentFrameIndex();

		m_PerFrameOpaqueGeometryData.cameraDataIndex = m_pScene->GetEditorCamera()->m_pConstantBuffer->m_VisibleHandles[frameIndex].Index;
		m_PerFrameOpaqueGeometryData.pointLightStructuredBufferIndex = m_pScene->GetLightManager().GetPointLights()->m_VisibleHandles[frameIndex].Index;
		m_PerFrameOpaqueGeometryData.directionalLightStructuredBufferIndex = m_pScene->GetLightManager().GetDirectionalLights()->m_VisibleHandles[frameIndex].Index;
		m_PerFrameOpaqueGeometryData.nrOfDirectionalLights = static_cast<uint32_t>(m_pScene->GetEntityManager().GetEntityCountForPool<DirectionalLightComponent>());
		m_PerFrameOpaqueGeometryData.nrOfPointLights = static_cast<uint32_t>(m_pScene->GetEntityManager().GetEntityCountForPool<PointLightComponent>());
		m_CombinedGeometryAndPickingPass->Upload("perFrameData", &m_PerFrameOpaqueGeometryData);

		EntityManager& entityManager = m_pScene->GetEntityManager();
		AssetManager& assetManager = AssetManager::Get();

		auto verticesIndex = m_CombinedGeometryAndPickingPass->GetInputSlot("vertices");
		auto indicesIndex = m_CombinedGeometryAndPickingPass->GetInputSlot("indices");
		auto perDrawIndex = m_CombinedGeometryAndPickingPass->GetInputSlot("perDrawData");
		auto identifierIndex = m_CombinedGeometryAndPickingPass->GetInputSlot("Identifier");

		entityManager.Collect<ForwardPassComponent, MeshFilterComponent, MeshRendererComponent>().Do([&](entity e, MeshFilterComponent& mfc)
			{
				if (assetManager.Exists(mfc.VertexBufferID))
				{
					VertexBuffer* vb = assetManager.GetAsset<VertexBuffer>(mfc.VertexBufferID);
					IndexBuffer* ib = assetManager.GetAsset<IndexBuffer>(mfc.IndexBufferID);

					DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRootShaderResourceView(verticesIndex, vb->GetInterface()->GetGPUVirtualAddress()));
					DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRootShaderResourceView(indicesIndex, ib->GetInterface()->GetGPUVirtualAddress()));

					m_PickingData.entityID = e;
					DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRoot32BitConstants(identifierIndex, 1u, &m_PickingData, 0u));

					auto& mrc = entityManager.Get<MeshRendererComponent>(e);
					const Material material = AssetManager::Get().Get<Material>(mrc.MaterialHandle);
					m_PerDrawData.materialIndex = material.GetConstantBufferIndex();
					m_PerDrawData.worldMatrixIndex = memoryManager.GetCBDescriptorIndex(entityManager.Get<TransformComponent>(e).ConstantBufferID);

					DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRoot32BitConstants(perDrawIndex, (uint32_t)sizeof(PerDrawData) / sizeof(uint32_t), &m_PerDrawData, 0u));

					RenderCommand::DrawInstanced(ib->GetNrOfIndices());
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

	void SceneRenderer::OnSceneViewportChanged(const uint32_t width, const uint32_t height) noexcept
	{
		MasterRenderer::Resize(width, height);
		m_GeometryRenderPass->Resize(width, height);
		m_CombinedGeometryAndPickingPass->Resize(width, height);
		m_WireFrameRenderPass->Resize(width, height);
		m_EditorGridRenderPass->Resize(width, height);
		m_GeometryPickingRenderPass->Resize(width, height);
		m_CompositeRenderPass->Resize(width, height);

		RenderTextureSpecification resolveRTSpec{};
		resolveRTSpec.MultiSampleCount = 1u;
		resolveRTSpec.CreateSRV = true;
		resolveRTSpec.Width = width;
		resolveRTSpec.Height = height;
		resolveRTSpec.Flags = D3D12_RESOURCE_FLAG_NONE;
		resolveRTSpec.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;

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
				m_EditorGridRenderPass->GetPipeline()->GetFrameBuffer()->SetOutputDependency(m_GeometryRenderPass->GetPipeline()->GetFrameBuffer(), 0);
				m_EditorGridRenderPass->GetPipeline()->GetFrameBuffer()->SetDepthDependency(m_GeometryRenderPass->GetPipeline()->GetFrameBuffer());
				m_WireFrameRenderPass->GetPipeline()->GetFrameBuffer()->SetOutputDependency(m_GeometryRenderPass->GetPipeline()->GetFrameBuffer(), 0);
				m_WireFrameRenderPass->GetPipeline()->GetFrameBuffer()->SetDepthDependency(m_GeometryRenderPass->GetPipeline()->GetFrameBuffer());
			}

			m_Options.MSAASamples = samples;

			m_GeometryRenderPass->OnMSAAReconfiguration(samples);
			m_CombinedGeometryAndPickingPass->OnMSAAReconfiguration(samples);
			m_WireFrameRenderPass->OnMSAAReconfiguration(samples);
			m_GeometryPickingRenderPass->OnMSAAReconfiguration(samples);
			m_EditorGridRenderPass->OnMSAAReconfiguration(samples);
			m_CompositeRenderPass->OnMSAAReconfiguration(samples);
		}
	}

	//entity SceneRenderer::GetHoveredEntity(const uint32_t x, const uint32_t y) noexcept
	//{
	//	auto pPickingColorOutput = m_GeometryPickingRenderPass->GetPipeline()->GetFrameBuffer()->GetColorOutput(0);
	//
	//	uint64_t totalBytes = RenderUtility::GetTextureSizeInBytes(pPickingColorOutput);
	//	D3D12_RANGE readBackBufferRange{ 0, totalBytes};
	//	uint32_t* pReadBackBufferData{};
	//	DXCall(m_pIdentifierReadbackTexture->GetInterface()->Map
	//	(
	//		0u,
	//		&readBackBufferRange,
	//		reinterpret_cast<void**>(&pReadBackBufferData)
	//	));
	//
	//	auto pPickingRT = m_GeometryPickingRenderPass->GetPipeline()->GetFrameBuffer()->GetColorOutput(0);
	//	auto desc = pPickingRT->GetInterface()->GetDesc();
	//
	//	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footPrint{};
	//	DXCall_STD(D3D12Core::GetDevice()->GetCopyableFootprints(&desc, 0u, 1u, 0u, &footPrint, nullptr, nullptr, nullptr));
	//	const uint32_t index = (y * (footPrint.Footprint.RowPitch / 4)) + x;
	//	const uint32_t indexExpressedAsBytes = index * 4;
	//
	//	bool outsideOfViewport = indexExpressedAsBytes > totalBytes;
	//	if (outsideOfViewport)
	//	{
	//		D3D12_RANGE emptyRange{ 0, 0 };
	//		DXCall_STD(m_pIdentifierReadbackTexture->GetInterface()->Unmap
	//		(
	//			0,
	//			&emptyRange
	//		));
	//		return NULL_ENTITY;
	//	}
	//
	//	m_HoveredEntity = pReadBackBufferData[index];
	//
	//	D3D12_RANGE emptyRange{ 0, 0 };
	//	DXCall_STD(m_pIdentifierReadbackTexture->GetInterface()->Unmap
	//	(
	//		0,
	//		&emptyRange
	//	));
	//
	//	return m_HoveredEntity;
	//}

	entity SceneRenderer::GetHoveredEntity() noexcept
	{
		uint32_t* pEntity = nullptr;
		DXCall(m_pIdentifierReadbackBuffer->GetInterface()->Map(0, nullptr, reinterpret_cast<void**>(&pEntity)));
		DXCall_STD(m_pIdentifierReadbackBuffer->GetInterface()->Unmap(0, nullptr));

		return *pEntity;
	}
}