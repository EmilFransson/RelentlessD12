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
		//Temporary: Should be results of shader reflection etc
		CreateRootSignatures();

		//Geometry Render pass:
		{
			FrameBufferSpecification frameBufferSpecification{};
			frameBufferSpecification.DebugName = "Geometry Framebuffer";
			frameBufferSpecification.Attachments = { TextureFormat::RGBA32F, TextureFormat::Depth };
			frameBufferSpecification.ClearColor = DirectX::XMFLOAT4(DirectX::Colors::CornflowerBlue);
			frameBufferSpecification.MSAA = { true, 8u, 0u };
			frameBufferSpecification.Transfer = true;

			PipelineSpecification pipelineSpecification{};
			pipelineSpecification.DebugName = "Geometry Pipeline";
			pipelineSpecification.pVertexShader = MasterRenderer::GetShaderLibrary().Get("VertexShader");
			pipelineSpecification.pPixelShader = MasterRenderer::GetShaderLibrary().Get("PixelShader");
			pipelineSpecification.pFrameBuffer = FrameBuffer::Create(frameBufferSpecification);
			pipelineSpecification.pRootSignature = m_pGeometryRootSignature;

			RenderPassSpecification renderpassDescriptor{};
			renderpassDescriptor.DebugName = "Geometry Pass";
			renderpassDescriptor.RenderPipeline = Pipeline::Create(pipelineSpecification);

			m_GeometryRenderPass = RenderPass::Create(renderpassDescriptor);
		}

		//Picking:
		{
			FrameBufferSpecification frameBufferSpecification{};
			frameBufferSpecification.DebugName = "Picking Framebuffer";
			frameBufferSpecification.Attachments = { TextureFormat::R32UINT, TextureFormat::Depth };
			static constexpr float clearVal = static_cast<float>(NULL_ENTITY);
			frameBufferSpecification.ClearColor = DirectX::XMFLOAT4(clearVal, clearVal, clearVal, clearVal);
			frameBufferSpecification.IsSRGB = false;
			frameBufferSpecification.Transfer = true;

			PipelineSpecification pipelineSpecification{};
			pipelineSpecification.DebugName = "Picking Pipeline";
			pipelineSpecification.pVertexShader = MasterRenderer::GetShaderLibrary().Get("VertexShader");
			pipelineSpecification.pPixelShader = MasterRenderer::GetShaderLibrary().Get("PickingPixelShader");
			pipelineSpecification.pFrameBuffer = FrameBuffer::Create(frameBufferSpecification);
			pipelineSpecification.pRootSignature = m_pGeometryPickingRootSignature;

			RenderPassSpecification renderpassDescriptor{};
			renderpassDescriptor.DebugName = "Picking Pass";
			renderpassDescriptor.RenderPipeline = Pipeline::Create(pipelineSpecification);

			m_GeometryPickingRenderPass = RenderPass::Create(renderpassDescriptor);
		}

		//Composite
		{
			FrameBufferSpecification frameBufferSpecification{};
			frameBufferSpecification.DebugName = "Composite Framebuffer";
			frameBufferSpecification.Attachments = { TextureFormat::RGBA32F };
			frameBufferSpecification.ClearColor = DirectX::XMFLOAT4(DirectX::Colors::Black);
			frameBufferSpecification.Transfer = true;
			frameBufferSpecification.Flags = D3D12_RESOURCE_FLAG_NONE;

			PipelineSpecification pipelineSpecification{};
			pipelineSpecification.DebugName = "Composite Pipeline";
			pipelineSpecification.DepthWrite = false;
			pipelineSpecification.pVertexShader = MasterRenderer::GetShaderLibrary().Get("FullScreenTriVertexShader");
			pipelineSpecification.pPixelShader = MasterRenderer::GetShaderLibrary().Get("PostProcessPixelShader");
			pipelineSpecification.pFrameBuffer = FrameBuffer::Create(frameBufferSpecification);
			pipelineSpecification.pRootSignature = m_pCompositeRootSignature;

			RenderPassSpecification renderpassDescriptor{};
			renderpassDescriptor.DebugName = "Composite Pass";
			renderpassDescriptor.RenderPipeline = Pipeline::Create(pipelineSpecification);

			m_CompositeRenderPass = RenderPass::Create(renderpassDescriptor);
		}

		//Identifier Readback Texture:
		ReadbackTextureSpecification RBTextureSpecification = {};
		RBTextureSpecification.Width = static_cast<uint32_t>(RenderUtility::GetTextureSizeInBytes(m_GeometryPickingRenderPass->GetPipeline()->GetFrameBuffer()->GetColorBuffer())); //Placeholder
		RBTextureSpecification.Height = 1u;
		RBTextureSpecification.Format = DXGI_FORMAT::DXGI_FORMAT_R32_UINT;
		RBTextureSpecification.MultiSampleCount = 1u;
		RBTextureSpecification.ClearColor = DirectX::XMFLOAT4(DirectX::Colors::Black);
		m_pIdentifierReadbackTexture = ReadbackTexture::Create(RBTextureSpecification, "Identifier ReadbackTexture");

		MasterRenderer::ExecuteCommands();
		MasterRenderer::WaitForGPU();
		MasterRenderer::ResetFrameCommandUnits(0u);
	}

	void SceneRenderer::Begin() noexcept
	{
		PROFILE_FUNC;

		MasterRenderer::ResetFrameCommandUnits(MasterRenderer::GetCurrentFrameIndex());

		RenderCommand::SetDescriptorHeap(MemoryManager::Get().GetShaderBindableDescriptorHeap());
		RenderCommand::SetViewport(m_pScene->GetViewport());
		RenderCommand::SetScissorRect(m_pScene->GetScissorRect());
	}

	void SceneRenderer::IssueRenderPasses() noexcept
	{
		PROFILE_FUNC;

		//PreRender();

		GeometryPass();
		PickingPass();
		CompositePass();
	}

	void SceneRenderer::End() noexcept
	{

	}

	void SceneRenderer::CreateRootSignatures() noexcept
	{
		//Geometry and Composite:
		{
			std::vector<D3D12_ROOT_PARAMETER> rootParameters;
			D3D12_ROOT_PARAMETER vertexBufferSRVParameter = {};
			vertexBufferSRVParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
			vertexBufferSRVParameter.Descriptor.ShaderRegister = 0u;
			vertexBufferSRVParameter.Descriptor.RegisterSpace = 0u;
			vertexBufferSRVParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
			rootParameters.push_back(vertexBufferSRVParameter);

			D3D12_ROOT_PARAMETER indexBufferSRVParameter = {};
			indexBufferSRVParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
			indexBufferSRVParameter.Descriptor.ShaderRegister = 1u;
			indexBufferSRVParameter.Descriptor.RegisterSpace = 0u;
			indexBufferSRVParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
			rootParameters.push_back(indexBufferSRVParameter);

			D3D12_ROOT_PARAMETER vpRootParameterVS = {};
			vpRootParameterVS.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
			vpRootParameterVS.Constants.Num32BitValues = 4 * 4;
			vpRootParameterVS.Constants.ShaderRegister = 0u;
			vpRootParameterVS.Constants.RegisterSpace = 0u;
			vpRootParameterVS.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
			rootParameters.push_back(vpRootParameterVS);

			D3D12_ROOT_PARAMETER perDrawAll = {};
			perDrawAll.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
			perDrawAll.Constants.Num32BitValues = sizeof(PerDrawData) / sizeof(uint32_t);
			perDrawAll.Constants.RegisterSpace = 0u;
			perDrawAll.Constants.ShaderRegister = 3u;
			perDrawAll.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
			rootParameters.push_back(perDrawAll);

			D3D12_ROOT_PARAMETER perFramePS = {};
			perFramePS.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
			perFramePS.Constants.Num32BitValues = (uint32_t)sizeof(PerFrameOpaqueGeometryData) / sizeof(uint32_t);
			perFramePS.Constants.RegisterSpace = 0u;
			perFramePS.Constants.ShaderRegister = 4u;
			perFramePS.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
			rootParameters.push_back(perFramePS);

			D3D12_STATIC_SAMPLER_DESC anisotropicSamplerDescriptor = {};
			anisotropicSamplerDescriptor.AddressU = anisotropicSamplerDescriptor.AddressV = anisotropicSamplerDescriptor.AddressW = D3D12_TEXTURE_ADDRESS_MODE::D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			anisotropicSamplerDescriptor.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
			anisotropicSamplerDescriptor.Filter = D3D12_FILTER::D3D12_FILTER_ANISOTROPIC;
			anisotropicSamplerDescriptor.MaxAnisotropy = 16u;
			anisotropicSamplerDescriptor.MaxLOD = D3D12_FLOAT32_MAX;
			anisotropicSamplerDescriptor.MinLOD = 0.0f;
			anisotropicSamplerDescriptor.MipLODBias = 0;
			anisotropicSamplerDescriptor.ShaderRegister = 0u;
			anisotropicSamplerDescriptor.RegisterSpace = 0u;
			anisotropicSamplerDescriptor.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

			D3D12_ROOT_SIGNATURE_DESC rootSignatureDescriptor = {};
			rootSignatureDescriptor.NumParameters = static_cast<UINT>(rootParameters.size());
			rootSignatureDescriptor.pParameters = rootParameters.data();
			rootSignatureDescriptor.NumStaticSamplers = 1;
			rootSignatureDescriptor.pStaticSamplers = &anisotropicSamplerDescriptor;
			rootSignatureDescriptor.Flags =
				D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS
				| D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS
				| D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS
				| D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS
				| D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS
				| D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED
				| D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED;

			Microsoft::WRL::ComPtr<ID3DBlob> pRootSignatureBlob{ nullptr };
			SERIALIZE_ROOT_SIGNATURE(rootSignatureDescriptor, pRootSignatureBlob);
			DXCall(D3D12Core::GetDevice()->CreateRootSignature
			(
				0u,
				pRootSignatureBlob->GetBufferPointer(),
				pRootSignatureBlob->GetBufferSize(),
				IID_PPV_ARGS(&m_pGeometryRootSignature)
			));

			D3D12_STATIC_SAMPLER_DESC samplerDescriptor = {};
			samplerDescriptor.AddressU = samplerDescriptor.AddressV = samplerDescriptor.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
			samplerDescriptor.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
			samplerDescriptor.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
			samplerDescriptor.MaxAnisotropy = 0u;
			samplerDescriptor.MaxLOD = D3D12_FLOAT32_MAX;
			samplerDescriptor.MinLOD = 0.0f;
			samplerDescriptor.MipLODBias = 0;
			samplerDescriptor.ShaderRegister = 0u;
			samplerDescriptor.RegisterSpace = 0u;
			samplerDescriptor.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

			D3D12_ROOT_PARAMETER textureIndexRootParameterPS = {};
			textureIndexRootParameterPS.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
			textureIndexRootParameterPS.Constants.Num32BitValues = 1;
			textureIndexRootParameterPS.Constants.RegisterSpace = 0u;
			textureIndexRootParameterPS.Constants.ShaderRegister = 2u;
			textureIndexRootParameterPS.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
			rootParameters.push_back(textureIndexRootParameterPS);

			rootSignatureDescriptor.NumParameters = static_cast<UINT>(rootParameters.size());
			rootSignatureDescriptor.pParameters = rootParameters.data();
			rootSignatureDescriptor.NumStaticSamplers = 1u;
			rootSignatureDescriptor.pStaticSamplers = &samplerDescriptor;
			SERIALIZE_ROOT_SIGNATURE(rootSignatureDescriptor, pRootSignatureBlob);
			DXCall(D3D12Core::GetDevice()->CreateRootSignature
			(
				0u,
				pRootSignatureBlob->GetBufferPointer(),
				pRootSignatureBlob->GetBufferSize(),
				IID_PPV_ARGS(&m_pCompositeRootSignature)
			));
		}

		//Picking:
		{
			std::vector<D3D12_ROOT_PARAMETER> rootParameters;
			D3D12_ROOT_PARAMETER vertexBufferSRVParameter = {};
			vertexBufferSRVParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
			vertexBufferSRVParameter.Descriptor.ShaderRegister = 0u;
			vertexBufferSRVParameter.Descriptor.RegisterSpace = 0u;
			vertexBufferSRVParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
			rootParameters.push_back(vertexBufferSRVParameter);

			D3D12_ROOT_PARAMETER indexBufferSRVParameter = {};
			indexBufferSRVParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
			indexBufferSRVParameter.Descriptor.ShaderRegister = 1u;
			indexBufferSRVParameter.Descriptor.RegisterSpace = 0u;
			indexBufferSRVParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
			rootParameters.push_back(indexBufferSRVParameter);

			D3D12_ROOT_PARAMETER vpRootParameterVS = {};
			vpRootParameterVS.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
			vpRootParameterVS.Constants.Num32BitValues = 4 * 4;
			vpRootParameterVS.Constants.ShaderRegister = 0u;
			vpRootParameterVS.Constants.RegisterSpace = 0u;
			vpRootParameterVS.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
			rootParameters.push_back(vpRootParameterVS);

			D3D12_ROOT_PARAMETER textureIndexIdentifierRootParameterPS = {};
			textureIndexIdentifierRootParameterPS.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
			textureIndexIdentifierRootParameterPS.Constants.Num32BitValues = 1;
			textureIndexIdentifierRootParameterPS.Constants.RegisterSpace = 1u;
			textureIndexIdentifierRootParameterPS.Constants.ShaderRegister = 0u;
			textureIndexIdentifierRootParameterPS.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
			rootParameters.push_back(textureIndexIdentifierRootParameterPS);

			D3D12_ROOT_PARAMETER perDrawAll = {};
			perDrawAll.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
			perDrawAll.Constants.Num32BitValues = sizeof(PerDrawData) / sizeof(uint32_t);
			perDrawAll.Constants.RegisterSpace = 0u;
			perDrawAll.Constants.ShaderRegister = 3u;
			perDrawAll.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
			rootParameters.push_back(perDrawAll);

			D3D12_ROOT_SIGNATURE_DESC rootSignatureDescriptor = {};
			rootSignatureDescriptor.NumParameters = static_cast<UINT>(rootParameters.size());
			rootSignatureDescriptor.pParameters = rootParameters.data();
			rootSignatureDescriptor.NumStaticSamplers = 0u;
			rootSignatureDescriptor.pStaticSamplers = nullptr;
			rootSignatureDescriptor.Flags =
				D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS
				| D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS
				| D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS
				| D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS
				| D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS
				| D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED
				| D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED;

			Microsoft::WRL::ComPtr<ID3DBlob> pRootSignatureBlob{ nullptr };
			SERIALIZE_ROOT_SIGNATURE(rootSignatureDescriptor, pRootSignatureBlob);
			DXCall(D3D12Core::GetDevice()->CreateRootSignature
			(
				0u,
				pRootSignatureBlob->GetBufferPointer(),
				pRootSignatureBlob->GetBufferSize(),
				IID_PPV_ARGS(&m_pGeometryPickingRootSignature)
			));
		}
	}
	
	void SceneRenderer::PreRender() noexcept
	{
		//Empty for now
	}

	void SceneRenderer::GeometryPass() noexcept
	{
		PROFILE_FUNC;

		MasterRenderer::BeginRenderPass(m_GeometryRenderPass);

		//Terrible and wrong for runtime:
		//Camera:
		auto vpMatrix = DirectX::XMLoadFloat4x4(&(m_pScene->GetEditorCamera()->GetViewProjectionMatrix()));
		DirectX::XMStoreFloat4x4(&m_VPData.VPMatrix, vpMatrix);
		DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRoot32BitConstants(2u, 4 * 4, &m_VPData, 0u));

		auto frameIndex = MasterRenderer::GetCurrentFrameIndex();

		m_PerFrameOpaqueGeometryData.cameraDataIndex = m_pScene->GetEditorCamera()->m_pConstantBuffer->m_VisibleHandles[frameIndex].Index;
		m_PerFrameOpaqueGeometryData.pointLightStructuredBufferIndex = m_pScene->GetLightManager().GetPointLights()->m_VisibleHandles[frameIndex].Index;
		m_PerFrameOpaqueGeometryData.directionalLightStructuredBufferIndex = m_pScene->GetLightManager().GetDirectionalLights()->m_VisibleHandles[frameIndex].Index;
		m_PerFrameOpaqueGeometryData.nrOfDirectionalLights = static_cast<uint32_t>(m_pScene->GetEntityManager().GetEntityCountForPool<DirectionalLightComponent>());
		m_PerFrameOpaqueGeometryData.nrOfPointLights = static_cast<uint32_t>(m_pScene->GetEntityManager().GetEntityCountForPool<PointLightComponent>());

		DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRoot32BitConstants(4, (uint32_t)sizeof(PerFrameOpaqueGeometryData) / sizeof(uint32_t), &m_PerFrameOpaqueGeometryData, 0u));

		EntityManager& entityManager = m_pScene->GetEntityManager();
		AssetManager& assetManager = AssetManager::Get();

		//Forward pass:
		entityManager.Collect<ForwardPassComponent, MeshFilterComponent, MeshRendererComponent>().Do([&](entity e, MeshFilterComponent& mfc)
			{
				if (assetManager.Exists(mfc.VertexBufferID))
				{
					VertexBuffer* vb = assetManager.GetAsset<VertexBuffer>(mfc.VertexBufferID);
					IndexBuffer* ib = assetManager.GetAsset<IndexBuffer>(mfc.IndexBufferID);
		
					DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRootShaderResourceView(0u, vb->GetInterface()->GetGPUVirtualAddress()));
					DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRootShaderResourceView(1u, ib->GetInterface()->GetGPUVirtualAddress()));
		
					auto& mrc = entityManager.Get<MeshRendererComponent>(e);
					m_PerDrawData.materialIndex = MemoryManager::Get().GetConstantBuffer(mrc.constantBufferID)->m_VisibleHandles[frameIndex].Index;
					m_PerDrawData.worldMatrixIndex = MemoryManager::Get().GetConstantBuffer(entityManager.Get<TransformComponent>(e).ConstantBufferID)->m_VisibleHandles[frameIndex].Index;
		
					DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRoot32BitConstants(3, (uint32_t)sizeof(PerDrawData) / sizeof(uint32_t), &m_PerDrawData, 0u));
		
					RenderCommand::DrawInstanced(ib->GetNrOfIndices());
				}
			});

		MasterRenderer::EndRenderPass();
	}

	void SceneRenderer::PickingPass() noexcept
	{
		PROFILE_FUNC;

		MasterRenderer::BeginRenderPass(m_GeometryPickingRenderPass);

		EntityManager& entityManager = m_pScene->GetEntityManager();
		AssetManager& assetManager = AssetManager::Get();
		auto frameIndex = MasterRenderer::GetCurrentFrameIndex();

		entityManager.Collect<ForwardPassComponent, MeshFilterComponent, MeshRendererComponent>().Do([&](entity e, MeshFilterComponent& mfc)
			{
				if (assetManager.Exists(mfc.VertexBufferID))
				{
					VertexBuffer* vb = assetManager.GetAsset<VertexBuffer>(mfc.VertexBufferID);
					IndexBuffer* ib = assetManager.GetAsset<IndexBuffer>(mfc.IndexBufferID);
		
					DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRootShaderResourceView(0u, vb->GetInterface()->GetGPUVirtualAddress()));
					DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRootShaderResourceView(1u, ib->GetInterface()->GetGPUVirtualAddress()));
		
					m_PickingData.entityID = e;
					DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRoot32BitConstants(3u, 1u, &m_PickingData, 0u));
		
					auto& mrc = entityManager.Get<MeshRendererComponent>(e);
					m_PerDrawData.materialIndex = MemoryManager::Get().GetConstantBuffer(mrc.constantBufferID)->m_VisibleHandles[frameIndex].Index;
					m_PerDrawData.worldMatrixIndex = MemoryManager::Get().GetConstantBuffer(entityManager.Get<TransformComponent>(e).ConstantBufferID)->m_VisibleHandles[frameIndex].Index;
					DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRoot32BitConstants(4, (uint32_t)sizeof(PerDrawData) / sizeof(uint32_t), &m_PerDrawData, 0u));
		
					RenderCommand::DrawInstanced(ib->GetNrOfIndices());
				}
			});

		RenderCommand::CopyTextureToTexture(m_GeometryPickingRenderPass->GetPipeline()->GetFrameBuffer()->GetColorBuffer(), m_pIdentifierReadbackTexture);

		MasterRenderer::EndRenderPass();
	}

	void SceneRenderer::CompositePass() noexcept
	{
		PROFILE_FUNC;

		//MSAA Resolve:
		{
			auto& pGeometryPassColorOutput = m_GeometryRenderPass->GetPipeline()->GetFrameBuffer()->GetColorBuffer();
			auto& compositeColorOutput = m_CompositeRenderPass->GetPipeline()->GetFrameBuffer()->GetColorBuffer();

			RenderCommand::TransitionResource(pGeometryPassColorOutput, D3D12_RESOURCE_STATE_RESOLVE_SOURCE);
			RenderCommand::TransitionResource(compositeColorOutput, D3D12_RESOURCE_STATE_RESOLVE_DEST);
			RenderCommand::ResolveMSAA(pGeometryPassColorOutput, compositeColorOutput);
		}

		auto& compositePipeline = m_CompositeRenderPass->GetPipeline();

		//Set composite render texture as pixel shader resource and UI-texture as render target:
		{
			auto& compositeRT = compositePipeline->GetFrameBuffer()->GetColorBuffer();

			RenderCommand::TransitionResource(compositeRT, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			RenderCommand::TransitionResource(ImguiLayer::GetUITexture(), D3D12_RESOURCE_STATE_RENDER_TARGET);
			RenderCommand::SetRenderTarget(ImguiLayer::GetUITexture(), nullptr);
		}

		//Set necessary d3d12 states:
		{
			RenderCommand::SetRootSignature(compositePipeline->GetRootSig());
			RenderCommand::SetPipelineState(compositePipeline->GetInterface2());
		}

		//Composite:
		{
			auto& compositeRT = compositePipeline->GetFrameBuffer()->GetColorBuffer();
			m_CompositeData.PostProcessTextureIndex = compositeRT->GetSRVDescriptorHandle().Index;
			DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRoot32BitConstants(5, 1u, &m_CompositeData, 0u));
			RenderCommand::DrawInstanced(3u);
		}

		//Set UI-texture as pixel shader resource and prepare back buffer as render target for imgui:
		{
			BackBuffer backBuffer = Window::GetCurrentBackBuffer();
			RenderCommand::TransitionResource(backBuffer.pBackBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
			RenderCommand::TransitionResource(ImguiLayer::GetUITexture(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			RenderCommand::SetRenderTarget(backBuffer);
		}
	}

	void SceneRenderer::OnSceneViewportChanged(const uint32_t width, const uint32_t height) noexcept
	{
		m_GeometryRenderPass->GetPipeline()->GetFrameBuffer()->Resize(width, height);
		m_GeometryPickingRenderPass->GetPipeline()->GetFrameBuffer()->Resize(width, height);
		m_CompositeRenderPass->GetPipeline()->GetFrameBuffer()->Resize(width, height);

		MemoryManager& memoryManager = MemoryManager::Get();

		auto& pPickingRT = m_GeometryPickingRenderPass->GetPipeline()->GetFrameBuffer()->GetColorBuffer();

		ReadbackTextureSpecification RBTextureSpecification = {};
		RBTextureSpecification.Width = static_cast<uint32_t>(RenderUtility::GetTextureSizeInBytes(pPickingRT));
		RBTextureSpecification.Height = 1u;
		RBTextureSpecification.Format = DXGI_FORMAT::DXGI_FORMAT_R32_UINT;
		RBTextureSpecification.MultiSampleCount = 1u;
		static constexpr float clearVal = static_cast<float>(NULL_ENTITY);
		RBTextureSpecification.ClearColor = DirectX::XMFLOAT4(clearVal, clearVal, clearVal, clearVal);
		memoryManager.DestroyResource(std::move(m_pIdentifierReadbackTexture));
		m_pIdentifierReadbackTexture = ReadbackTexture::Create(RBTextureSpecification, "Identifier ReadbackTexture");
	}

	entity SceneRenderer::GetHoveredEntity(const uint32_t x, const uint32_t y) noexcept
	{
		auto& pPickingColorOutput = m_GeometryPickingRenderPass->GetPipeline()->GetFrameBuffer()->GetColorBuffer();

		uint64_t totalBytes = RenderUtility::GetTextureSizeInBytes(pPickingColorOutput);
		D3D12_RANGE readBackBufferRange{ 0, totalBytes};
		uint32_t* pReadBackBufferData{};
		DXCall(m_pIdentifierReadbackTexture->GetInterface()->Map
		(
			0u,
			&readBackBufferRange,
			reinterpret_cast<void**>(&pReadBackBufferData)
		));

		auto& pPickingRT = m_GeometryPickingRenderPass->GetPipeline()->GetFrameBuffer()->GetColorBuffer();
		auto desc = pPickingRT->GetInterface()->GetDesc();

		D3D12_PLACED_SUBRESOURCE_FOOTPRINT footPrint{};
		DXCall_STD(D3D12Core::GetDevice()->GetCopyableFootprints(&desc, 0u, 1u, 0u, &footPrint, nullptr, nullptr, nullptr));
		const uint32_t index = (y * (footPrint.Footprint.RowPitch / 4)) + x;
		const uint32_t indexExpressedAsBytes = index * 4;

		bool outsideOfViewport = indexExpressedAsBytes > totalBytes;
		if (outsideOfViewport)
		{
			D3D12_RANGE emptyRange{ 0, 0 };
			DXCall_STD(m_pIdentifierReadbackTexture->GetInterface()->Unmap
			(
				0,
				&emptyRange
			));
			return NULL_ENTITY;
		}

		m_HoveredEntity = pReadBackBufferData[index];

		D3D12_RANGE emptyRange{ 0, 0 };
		DXCall_STD(m_pIdentifierReadbackTexture->GetInterface()->Unmap
		(
			0,
			&emptyRange
		));

		return m_HoveredEntity;
	}
}