#include "Renderer3D.h"
#include "../Resources/Texture.h"
#include "../Resources/DepthStencil.h"
#include "../D3D12Core.h"
#include "RenderCommand.h"
#include "../../Core/Window.h"
#include "../../ImGui/ImguiLayer.h"
#include "../MemoryManager.h"
#include "Camera/PerspectiveCamera.h"
#include "../Shaders/ShaderLibrary.h"
#include "Assets/AssetManager.h"
#include "../../Scene/Scene.h"
#include "RenderPass.h"

namespace Relentless
{
	struct Renderer3dData 
	{
		//Swap to fence-object:
		Microsoft::WRL::ComPtr<ID3D12Fence1> pFence{nullptr};
		HANDLE fenceEvent{ nullptr };
		std::unique_ptr<uint64_t[]> pFenceValues{ nullptr };

		uint32_t currentFrameIndex{0u};

		D3D12_VIEWPORT viewPort{};
		RECT scissorRect{};

		Microsoft::WRL::ComPtr<ID3D12RootSignature> pRootSignature{ nullptr };
		Microsoft::WRL::ComPtr<ID3D12RootSignature> pPickingRootSignature{ nullptr };
		Microsoft::WRL::ComPtr<ID3D12RootSignature> pPostProcessRootSignature{ nullptr };

		ShaderLibrary m_ShaderLibrary;

		std::vector<entity> m_ForwardPassEntities;
		std::vector<entity> m_PickingPassEntities;

		uint32_t totalBytes;
		entity hoveredEntity { NULL_ENTITY };

		EditorGrid* pEditorGrid{ nullptr };

		std::shared_ptr<Pipeline> MainPipeline{ nullptr };
		std::shared_ptr<Pipeline> PickingPipeline{ nullptr };
		std::shared_ptr<Pipeline> CompositePipeline{ nullptr };

	};
	static Renderer3dData s_RendererData = {};
	
	void Renderer3D::Initialize() noexcept
	{
		//Fence:
		s_RendererData.pFenceValues = std::make_unique<uint64_t[]>(D3D12Core::GetNrOfBufferedFrames());
		DXCall(D3D12Core::GetDevice()->CreateFence(0u, D3D12_FENCE_FLAGS::D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&s_RendererData.pFence)));
		s_RendererData.pFenceValues[s_RendererData.currentFrameIndex]++;
		s_RendererData.fenceEvent = ::CreateEvent(nullptr, false, false, nullptr);
		RLS_ASSERT(s_RendererData.fenceEvent, "Fence event creation failed.");

		//Viewport:
		s_RendererData.viewPort.TopLeftX = 0.0f;
		s_RendererData.viewPort.TopLeftY = 0.0f;
		s_RendererData.viewPort.Width = 800.0f;
		s_RendererData.viewPort.Height = 600.0f;
		s_RendererData.viewPort.MinDepth = 0.0f;
		s_RendererData.viewPort.MaxDepth = 1.0f;
		
		//ScissorRect:
		s_RendererData.scissorRect.left = 0u;
		s_RendererData.scissorRect.top = 0u;
		s_RendererData.scissorRect.right = static_cast<LONG>(s_RendererData.viewPort.Width);
		s_RendererData.scissorRect.bottom = static_cast<LONG>(s_RendererData.viewPort.Height);

		s_RendererData.m_ShaderLibrary.Initialize();
		
		CreateMainRootSignature();
		CreatePickingRootSignature();

		//MSAA Geometry:
		//{
		//	FrameBufferSpecification frameBufferSpecification{};
		//	frameBufferSpecification.DebugName = "Main MSAA Framebuffer";
		//	frameBufferSpecification.Attachments = { TextureFormat::RGBA32F, TextureFormat::Depth };
		//	frameBufferSpecification.ClearColor = DirectX::XMFLOAT4(DirectX::Colors::CornflowerBlue);
		//	frameBufferSpecification.MSAA = { true, 8u, 0u };
		//	frameBufferSpecification.Transfer = true;
		//
		//	PipelineSpecification pipelineSpecification{};
		//	pipelineSpecification.DebugName = "Main Pipeline";
		//	pipelineSpecification.pVertexShader = s_RendererData.m_ShaderLibrary.Get("VertexShader");
		//	pipelineSpecification.pPixelShader = s_RendererData.m_ShaderLibrary.Get("PixelShader");
		//	pipelineSpecification.pFrameBuffer = FrameBuffer::Create(frameBufferSpecification);
		//
		//	s_RendererData.MainPipeline = Pipeline::Create(pipelineSpecification);
		//}
		//
		////Picking:
		//{
		//	FrameBufferSpecification frameBufferSpecification{};
		//	frameBufferSpecification.DebugName = "Picking Framebuffer";
		//	frameBufferSpecification.Attachments = { TextureFormat::R32UINT, TextureFormat::Depth };
		//	static constexpr float clearVal = static_cast<float>(NULL_ENTITY);
		//	frameBufferSpecification.ClearColor = DirectX::XMFLOAT4(clearVal, clearVal, clearVal, clearVal);
		//	frameBufferSpecification.IsSRGB = false;
		//
		//	PipelineSpecification pipelineSpecification{};
		//	pipelineSpecification.DebugName = "Picking Pipeline";
		//	pipelineSpecification.pVertexShader = s_RendererData.m_ShaderLibrary.Get("VertexShader");
		//	pipelineSpecification.pPixelShader = s_RendererData.m_ShaderLibrary.Get("PickingPixelShader");
		//	pipelineSpecification.pFrameBuffer = FrameBuffer::Create(frameBufferSpecification);
		//
		//	s_RendererData.PickingPipeline = Pipeline::Create(pipelineSpecification);
		//}
		//
		////Composite
		//{
		//	FrameBufferSpecification frameBufferSpecification{};
		//	frameBufferSpecification.DebugName = "Composite Framebuffer";
		//	frameBufferSpecification.Attachments = { TextureFormat::RGBA32F };
		//	frameBufferSpecification.ClearColor = DirectX::XMFLOAT4(DirectX::Colors::Black);
		//	frameBufferSpecification.Transfer = true;
		//	frameBufferSpecification.Flags = D3D12_RESOURCE_FLAG_NONE;
		//
		//	PipelineSpecification pipelineSpecification{};
		//	pipelineSpecification.DebugName = "Composite Pipeline";
		//	pipelineSpecification.DepthWrite = false;
		//	pipelineSpecification.pVertexShader = s_RendererData.m_ShaderLibrary.Get("FullScreenTriVertexShader");
		//	pipelineSpecification.pPixelShader = s_RendererData.m_ShaderLibrary.Get("PostProcessPixelShader");
		//	pipelineSpecification.pFrameBuffer = FrameBuffer::Create(frameBufferSpecification);
		//
		//	s_RendererData.CompositePipeline = Pipeline::Create(pipelineSpecification);
		//}
		//
		//Renderer3D::ExecuteCommands();
		//Renderer3D::WaitForGPU();
		//RenderCommand::ResetFrameCommandUnits(0u); //TO BE CHANGED! UPLOAD BUFFER SHOULD UPLOAD EVERYTHING SEQUENTIALLY!
	}

	void Renderer3D::Begin(const std::shared_ptr<PerspectiveCamera>& pSceneCamera, Scene& scene) noexcept
	{
		PROFILE_FUNC;

		s_RendererData.m_ForwardPassEntities.clear();
		s_RendererData.m_PickingPassEntities.clear();

		RenderCommand::ResetFrameCommandUnits(s_RendererData.currentFrameIndex);

		RenderCommand::SetViewport(s_RendererData.viewPort);
		RenderCommand::SetScissorRect(s_RendererData.scissorRect);

		//auto& pMainRT = s_RendererData.MainPipeline->GetFrameBuffer()->GetColorBuffer();
		//auto& pMainDS = s_RendererData.MainPipeline->GetFrameBuffer()->GetDepthBuffer();

		//RenderCommand::TransitionResource(pMainRT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		//RenderCommand::ClearRenderTarget(pMainRT->GetRTVDescriptorHandle().CPUHandle, pMainRT->GetClearColor());
		//RenderCommand::ClearDepthStencil(pMainDS);
		//RenderCommand::SetRenderTarget(pMainRT, pMainDS);
		
		DXCall_STD(D3D12Core::GetCommandList()->SetDescriptorHeaps(1u, MemoryManager::Get().GetShaderBindableDescriptorHeap()->GetDescriptorHeapInterface().GetAddressOf()));
		RenderCommand::SetRootSignature(s_RendererData.MainPipeline->GetRootSig());
		RenderCommand::SetPipelineState(s_RendererData.MainPipeline->GetInterface2());
		RenderCommand::SetTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		static VP vpMatrixCBuffer;
		auto vpMatrix = DirectX::XMLoadFloat4x4(&(pSceneCamera->GetViewProjectionMatrix()));
		DirectX::XMStoreFloat4x4(&vpMatrixCBuffer.VPMatrix, vpMatrix);
		DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRoot32BitConstants(2u, 4 * 4, &vpMatrixCBuffer, 0u));

		auto frameIndex = D3D12Core::GetCurrentFrame() % D3D12Core::GetNrOfBufferedFrames();

		static PerFrameDataOpaque perFrameDataOpaque;
		perFrameDataOpaque.cameraDataIndex = pSceneCamera->m_pConstantBuffer->m_VisibleHandles[frameIndex].Index;
		perFrameDataOpaque.pointLightStructuredBufferIndex = scene.GetLightManager().GetPointLights()->m_VisibleHandles[frameIndex].Index;
		perFrameDataOpaque.directionalLightStructuredBufferIndex = scene.GetLightManager().GetDirectionalLights()->m_VisibleHandles[frameIndex].Index;
		perFrameDataOpaque.nrOfDirectionalLights = static_cast<uint32_t>(scene.GetEntityManager().GetEntityCountForPool<DirectionalLightComponent>());
		perFrameDataOpaque.nrOfPointLights = static_cast<uint32_t>(scene.GetEntityManager().GetEntityCountForPool<PointLightComponent>());

		DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRoot32BitConstants(4, (uint32_t)sizeof(PerFrameDataOpaque) / sizeof(uint32_t), &perFrameDataOpaque, 0u));
	}

	void Renderer3D::Submit(const entity e) noexcept
	{
		PROFILE_FUNC;
		
		s_RendererData.m_ForwardPassEntities.push_back(e);
		s_RendererData.m_PickingPassEntities.push_back(e);
	}

	void Renderer3D::End([[maybe_unused]] EntityManager& entityManager) noexcept
	{
		PROFILE_FUNC;

		//AssetManager& assetManager = AssetManager::Get();
		//auto frameIndex = D3D12Core::GetCurrentFrame() % D3D12Core::GetNrOfBufferedFrames();
		//
		//static PerDrawData2 perDrawData2;
		////Forward pass:
		//for (auto e : s_RendererData.m_ForwardPassEntities)
		//{
		//	auto& mfc = entityManager.Get<MeshFilterComponent>(e);
		//	if (assetManager.Exists(mfc.VertexBufferID))
		//	{
		//		VertexBuffer* vb = assetManager.GetAsset<VertexBuffer>(mfc.VertexBufferID);
		//		IndexBuffer* ib = assetManager.GetAsset<IndexBuffer>(mfc.IndexBufferID);
		//
		//		DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRootShaderResourceView(0u, vb->GetInterface()->GetGPUVirtualAddress()));
		//		DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRootShaderResourceView(1u, ib->GetInterface()->GetGPUVirtualAddress()));
		//
		//		auto& mrc = entityManager.Get<MeshRendererComponent>(e);
		//		perDrawData2.materialIndex = MemoryManager::Get().GetConstantBuffer(mrc.constantBufferID)->m_VisibleHandles[frameIndex].Index;
		//		perDrawData2.worldMatrixIndex = MemoryManager::Get().GetConstantBuffer(entityManager.Get<TransformComponent>(e).ConstantBufferID)->m_VisibleHandles[frameIndex].Index;
		//
		//		DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRoot32BitConstants(3, (uint32_t)sizeof(PerDrawData2) / sizeof(uint32_t), &perDrawData2, 0u));
		//
		//		RenderCommand::DrawInstanced(ib->GetNrOfIndices());
		//	}
		//}

		//Picking pass:
		//States:
		{
			//auto& pPickingRT = s_RendererData.PickingPipeline->GetFrameBuffer()->GetColorBuffer();
			//auto& pPickingDS = s_RendererData.PickingPipeline->GetFrameBuffer()->GetDepthBuffer();
			//
			//RenderCommand::TransitionResource(pPickingRT, D3D12_RESOURCE_STATE_RENDER_TARGET);
			//RenderCommand::ClearRenderTarget(pPickingRT->GetRTVDescriptorHandle().CPUHandle, pPickingRT->GetClearColor());
			//RenderCommand::ClearDepthStencil(pPickingDS);

			RenderCommand::SetRootSignature(s_RendererData.PickingPipeline->GetRootSig());
			RenderCommand::SetPipelineState(s_RendererData.PickingPipeline->GetInterface2());
			//RenderCommand::SetRenderTarget(pPickingRT, pPickingDS);
		}
		static Identifier ID;

		//for (auto e : s_RendererData.m_PickingPassEntities)
		//{
		//	auto& mfc = entityManager.Get<MeshFilterComponent>(e);
		//	if (assetManager.Exists(mfc.VertexBufferID))
		//	{
		//		VertexBuffer* vb = assetManager.GetAsset<VertexBuffer>(mfc.VertexBufferID);
		//		IndexBuffer* ib = assetManager.GetAsset<IndexBuffer>(mfc.IndexBufferID);
		//
		//		DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRootShaderResourceView(0u, vb->GetInterface()->GetGPUVirtualAddress()));
		//		DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRootShaderResourceView(1u, ib->GetInterface()->GetGPUVirtualAddress()));
		//
		//		ID.entityID = e;
		//		DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRoot32BitConstants(3u, 1u, &ID, 0u));
		//
		//		auto& mrc = entityManager.Get<MeshRendererComponent>(e);
		//		perDrawData2.materialIndex = MemoryManager::Get().GetConstantBuffer(mrc.constantBufferID)->m_VisibleHandles[frameIndex].Index;
		//		perDrawData2.worldMatrixIndex = MemoryManager::Get().GetConstantBuffer(entityManager.Get<TransformComponent>(e).ConstantBufferID)->m_VisibleHandles[frameIndex].Index;
		//		DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRoot32BitConstants(4, (uint32_t)sizeof(PerDrawData2) / sizeof(uint32_t), &perDrawData2, 0u));
		//
		//		RenderCommand::DrawInstanced(ib->GetNrOfIndices());
		//	}
		//}
		
		//auto& pPickingRT = s_RendererData.PickingPipeline->GetFrameBuffer()->GetColorBuffer();
		//RenderCommand::TransitionResource(pPickingRT, D3D12_RESOURCE_STATE_COPY_SOURCE);

		//D3D12_PLACED_SUBRESOURCE_FOOTPRINT footPrint = {};
		//UINT64 totalBytes{};
		//auto desc = pPickingRT->GetInterface()->GetDesc();

		//DXCall_STD(D3D12Core::GetDevice()->GetCopyableFootprints(&desc, 0u, 1u, 0u, &footPrint, nullptr, nullptr, &totalBytes));
		//
		//D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
		//dstLocation.pResource = s_RendererData.m_pIdentifierReadbackTexture->GetInterface().Get();
		//dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		//dstLocation.PlacedFootprint = footPrint;
		//
		//D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
		//srcLocation.pResource = pPickingRT->GetInterface().Get();
		//srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		//srcLocation.SubresourceIndex = 0u;
		//
		//DXCall_STD(D3D12Core::GetCommandList()->CopyTextureRegion(&dstLocation, 0u, 0u ,0u, &srcLocation, nullptr));
		//s_RendererData.totalBytes = static_cast<uint32_t>(totalBytes);

		//MSAA Resolve:
		{
			//auto& pMainRT = s_RendererData.MainPipeline->GetFrameBuffer()->GetColorBuffer();
			//auto& compositeRT = s_RendererData.CompositePipeline->GetFrameBuffer()->GetColorBuffer();

			//RenderCommand::TransitionResource(pMainRT, D3D12_RESOURCE_STATE_RESOLVE_SOURCE);
			//RenderCommand::TransitionResource(compositeRT, D3D12_RESOURCE_STATE_RESOLVE_DEST);
			//RenderCommand::ResolveMSAA(pMainRT, compositeRT);
		}

		//Set post process render texture as pixel shader resource and UI-texture as render target:
		{
			//auto& compositeRT = s_RendererData.CompositePipeline->GetFrameBuffer()->GetColorBuffer();
			//
			//RenderCommand::TransitionResource(compositeRT, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			//RenderCommand::TransitionResource(ImguiLayer::GetUITexture(), D3D12_RESOURCE_STATE_RENDER_TARGET);
			//RenderCommand::SetRenderTarget(ImguiLayer::GetUITexture(), nullptr);
		}

		//Set necessary d3d12 states:
		{
			//DXCall_STD(D3D12Core::GetCommandList()->SetDescriptorHeaps(1u, MemoryManager::Get().GetShaderBindableDescriptorHeap()->GetDescriptorHeapInterface().GetAddressOf()));
			RenderCommand::SetRootSignature(s_RendererData.CompositePipeline->GetRootSig());
			RenderCommand::SetPipelineState(s_RendererData.CompositePipeline->GetInterface2());
		}

		//Post process:
		{
			//static PerFrameData textureData;
			//auto& compositeRT = s_RendererData.CompositePipeline->GetFrameBuffer()->GetColorBuffer();
			//textureData.PostProcessTextureIndex = compositeRT->GetSRVDescriptorHandle().Index;
			//DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRoot32BitConstants(5, 1u, &textureData, 0u));
			//RenderCommand::DrawInstanced(3u);
		}

		//Set UI-texture as pixel shader resource and prepare back buffer as render target for imgui:
		{
			//BackBuffer backBuffer = Window::GetCurrentBackBuffer();
			//RenderCommand::TransitionResource(backBuffer.pBackBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
			//RenderCommand::TransitionResource(ImguiLayer::GetUITexture(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			//RenderCommand::SetRenderTarget(backBuffer);
		}
	}

	void Renderer3D::SubmitEditorGrid(EditorGrid* pEditorGrid) noexcept
	{
		RLS_ASSERT(pEditorGrid, "EditorGrid pointer is null.");
		s_RendererData.pEditorGrid = pEditorGrid;
	}

	void Renderer3D::PrepareBackBuffer() noexcept
	{
		//Transition the back buffer back to present state now that is is finished:
		BackBuffer& backBuffer{ Window::GetCurrentBackBuffer() };
		RenderCommand::TransitionResource(backBuffer.pBackBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	}

	void Renderer3D::ExecuteCommands() noexcept
	{
		PROFILE_FUNC;

		auto pCommandList{ D3D12Core::GetCommandList() };
		DXCall(pCommandList->Close());
		ID3D12CommandList* pCommandLists[] = { pCommandList.Get() };
		DXCall_STD(D3D12Core::GetCommandQueue()->ExecuteCommandLists(ARRAYSIZE(pCommandLists), pCommandLists));
	}

	void Renderer3D::WaitAndSync() noexcept
	{
		PROFILE_FUNC;

		// Schedule a Signal command in the queue.
		const UINT64 currentFenceValue = s_RendererData.pFenceValues[s_RendererData.currentFrameIndex];
		DXCall(D3D12Core::GetCommandQueue()->Signal(s_RendererData.pFence.Get(), currentFenceValue));

		s_RendererData.currentFrameIndex = D3D12Core::GetCurrentFrame() % D3D12Core::GetNrOfBufferedFrames();

		// If the next frame is not ready to be rendered yet, wait until it is ready.
		if (s_RendererData.pFence->GetCompletedValue() < s_RendererData.pFenceValues[s_RendererData.currentFrameIndex])
		{
			DXCall(s_RendererData.pFence->SetEventOnCompletion(s_RendererData.pFenceValues[s_RendererData.currentFrameIndex], s_RendererData.fenceEvent));
			::WaitForSingleObjectEx(s_RendererData.fenceEvent, INFINITE, FALSE);
		}

		// Set the fence value for the next frame.
		s_RendererData.pFenceValues[s_RendererData.currentFrameIndex] = currentFenceValue + 1;
	}

	void Renderer3D::WaitForGPU() noexcept
	{
		// Schedule a Signal command in the queue.
		DXCall(D3D12Core::GetCommandQueue()->Signal(s_RendererData.pFence.Get(), s_RendererData.pFenceValues[s_RendererData.currentFrameIndex]));
		
		// Wait until the fence has been processed.
		DXCall(s_RendererData.pFence->SetEventOnCompletion(s_RendererData.pFenceValues[s_RendererData.currentFrameIndex], s_RendererData.fenceEvent));
		WaitForSingleObjectEx(s_RendererData.fenceEvent, INFINITE, FALSE);
		
		// Increment the fence value for the current frame.
		s_RendererData.pFenceValues[s_RendererData.currentFrameIndex]++;
	}

	void Renderer3D::OnShutDown() noexcept
	{
		WaitForGPU();
	}

	void Renderer3D::OnSceneViewportChanged(const uint32_t width, const uint32_t height) noexcept
	{
 		s_RendererData.MainPipeline->GetFrameBuffer()->Resize(width, height);
		s_RendererData.PickingPipeline->GetFrameBuffer()->Resize(width, height);
		s_RendererData.CompositePipeline->GetFrameBuffer()->Resize(width, height);

		//MemoryManager& memoryManager = MemoryManager::Get();

		//auto& pPickingRT = s_RendererData.PickingPipeline->GetFrameBuffer()->GetColorBuffer();
		//auto desc = pPickingRT->GetInterface()->GetDesc();
		//UINT64 totalBytes{};
		//DXCall_STD(D3D12Core::GetDevice()->GetCopyableFootprints(&desc, 0u, 1u, 0u, nullptr, nullptr, nullptr, &totalBytes));


		s_RendererData.viewPort.Width = static_cast<float>(width);
		s_RendererData.viewPort.Height = static_cast<float>(height);
		s_RendererData.scissorRect.right = static_cast<LONG>(width);
		s_RendererData.scissorRect.bottom = static_cast<LONG>(height);
 	}

	void Renderer3D::CreateMainRootSignature() noexcept
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
		perDrawAll.Constants.Num32BitValues = sizeof(PerDrawData2) /  sizeof(uint32_t);
		perDrawAll.Constants.RegisterSpace = 0u;
		perDrawAll.Constants.ShaderRegister = 3u;
		perDrawAll.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		rootParameters.push_back(perDrawAll);

		D3D12_ROOT_PARAMETER perFramePS = {};
		perFramePS.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		perFramePS.Constants.Num32BitValues = (uint32_t)sizeof(PerFrameDataOpaque) / sizeof(uint32_t);
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
			IID_PPV_ARGS(&s_RendererData.pRootSignature)
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
			IID_PPV_ARGS(&s_RendererData.pPostProcessRootSignature)
		));
	}

	void Renderer3D::CreatePickingRootSignature() noexcept
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
		perDrawAll.Constants.Num32BitValues = sizeof(PerDrawData2) / sizeof(uint32_t);
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
			IID_PPV_ARGS(&s_RendererData.pPickingRootSignature)
		));
	}

	entity Renderer3D::GetHoveredEntity([[maybe_unused]] const uint32_t x, [[maybe_unused]] const uint32_t y) noexcept
	{
		return 0;
	}
}