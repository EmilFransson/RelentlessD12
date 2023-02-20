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
#include "../Resources/AssetManager.h"
#include "../../Scene/Scene.h"

namespace Relentless
{
	struct Renderer3dData 
	{
		//Swap to fence-object:
		Microsoft::WRL::ComPtr<ID3D12Fence1> pFence{nullptr};
		HANDLE fenceEvent{ nullptr };
		std::unique_ptr<uint64_t[]> pFenceValues{ nullptr };
		uint32_t currentFrameIndex{0u};
		std::shared_ptr<RenderTexture> m_pMSAARenderTexture{ nullptr };
		std::shared_ptr<RenderTexture> m_pPostProcessRenderTexture{ nullptr };
		std::shared_ptr<RenderTexture> m_pIdentifierRenderTexture{ nullptr };
		std::shared_ptr<ReadbackTexture> m_pIdentifierReadbackTexture{ nullptr };

		std::shared_ptr<DepthStencil> m_pMSAADepthStencil{ nullptr };
		std::shared_ptr<DepthStencil> m_pPickingDepthStencil{ nullptr };
		D3D12_VIEWPORT viewPort{};
		RECT scissorRect{};
		Microsoft::WRL::ComPtr<ID3D12RootSignature> pRootSignature{ nullptr };
		Microsoft::WRL::ComPtr<ID3D12RootSignature> pPickingRootSignature{ nullptr };
		Microsoft::WRL::ComPtr<ID3D12RootSignature> pPostProcessRootSignature{ nullptr };
		Microsoft::WRL::ComPtr<ID3D12PipelineState> pPipelineState{ nullptr };
		Microsoft::WRL::ComPtr<ID3D12PipelineState> pPostProcessPipelineState{ nullptr };
		Microsoft::WRL::ComPtr<ID3D12PipelineState> pPickingPipelineState{ nullptr };

		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDescriptor{};
		D3D12_GRAPHICS_PIPELINE_STATE_DESC pickingPsoDescriptor{};

		ShaderLibrary m_ShaderLibrary;

		std::vector<entity> m_ForwardPassEntities;
		std::vector<entity> m_PickingPassEntities;

		uint32_t totalBytes;
		entity hoveredEntity { NULL_ENTITY };
	};
	static Renderer3dData s_RendererData = {};
	
	void Renderer3D::Initialize() noexcept
	{
		s_RendererData.pFenceValues = std::move(std::make_unique<uint64_t[]>(D3D12Core::GetNrOfBufferedFrames()));

		DXCall(D3D12Core::GetDevice()->CreateFence(0u, D3D12_FENCE_FLAGS::D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&s_RendererData.pFence)));
		s_RendererData.pFenceValues[s_RendererData.currentFrameIndex]++;
		s_RendererData.fenceEvent = ::CreateEvent(nullptr, false, false, nullptr);
		RLS_ASSERT(s_RendererData.fenceEvent, "Fence event creation failed.");

		RenderTextureSpecification textureSpecification = {};
		textureSpecification.Width = 800u;
		textureSpecification.Height = 600u;
		textureSpecification.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		textureSpecification.MultiSampleCount = 8u;
		textureSpecification.ClearColor = DirectX::XMFLOAT4(DirectX::Colors::CornflowerBlue);
		textureSpecification.CreateSRV = true;
		s_RendererData.m_pMSAARenderTexture = std::move(RenderTexture::Create(textureSpecification, "Main MSAA RenderTexture"));

		//Post Process Render Texture:
		textureSpecification.MultiSampleCount = 1u;
		textureSpecification.ClearColor = DirectX::XMFLOAT4(DirectX::Colors::Black);
		s_RendererData.m_pPostProcessRenderTexture = std::move(RenderTexture::Create(textureSpecification, "Post Process RenderTexture"));

		//Identifier Render Texture:
		textureSpecification.MultiSampleCount = 1u;
		textureSpecification.Format = DXGI_FORMAT::DXGI_FORMAT_R32_UINT;
		textureSpecification.CreateSRV = false;
		textureSpecification.isSRGB = false;

		static constexpr uint32_t clearVal = NULL_ENTITY;//10'000'000'000;

		textureSpecification.ClearColor = DirectX::XMFLOAT4(clearVal, clearVal, clearVal, clearVal);
		s_RendererData.m_pIdentifierRenderTexture = std::move(RenderTexture::Create(textureSpecification, "Identifier RenderTexture"));

		D3D12_PLACED_SUBRESOURCE_FOOTPRINT footPrint = {};
		UINT numRows{};
		UINT64 rowSizeInBytes{};
		UINT64 totalBytes{};

		auto desc = s_RendererData.m_pIdentifierRenderTexture->GetInterface()->GetDesc();

		DXCall_STD(D3D12Core::GetDevice()->GetCopyableFootprints(&desc, 0u, 1u, 0u, &footPrint, &numRows, &rowSizeInBytes, &totalBytes));

		//Identifier Readback Texture:
		ReadbackTextureSpecification RBTextureSpecification = {};
		RBTextureSpecification.Width = static_cast<uint32_t>(totalBytes);
		RBTextureSpecification.Height = 1u;
		RBTextureSpecification.Format = textureSpecification.Format;
		RBTextureSpecification.MultiSampleCount = textureSpecification.MultiSampleCount;
		RBTextureSpecification.ClearColor = textureSpecification.ClearColor;
		s_RendererData.m_pIdentifierReadbackTexture = std::move(ReadbackTexture::Create(RBTextureSpecification, "Identifier ReadbackTexture"));

		s_RendererData.m_pMSAADepthStencil = std::move(DepthStencil::Create(800u, 600u, 8u, "Main MSAA DepthStencil"));
		s_RendererData.m_pPickingDepthStencil = std::move(DepthStencil::Create(800u, 600u, 1u, "Picking DepthStencil"));

		s_RendererData.viewPort.TopLeftX = 0.0f;
		s_RendererData.viewPort.TopLeftY = 0.0f;
		s_RendererData.viewPort.Width = 800.0f;
		s_RendererData.viewPort.Height = 600.0f;
		s_RendererData.viewPort.MinDepth = 0.0f;
		s_RendererData.viewPort.MaxDepth = 1.0f;

		s_RendererData.scissorRect.left = 0u;
		s_RendererData.scissorRect.top = 0u;
		s_RendererData.scissorRect.right = static_cast<LONG>(s_RendererData.viewPort.Width);
		s_RendererData.scissorRect.bottom = static_cast<LONG>(s_RendererData.viewPort.Height);

		s_RendererData.m_ShaderLibrary.Initialize();
		
		CreateMainRootSignature();
		CreateMainPipelineState();
		CreatePickingRootSignature();
		CreatePickingPipelineState();

		Renderer3D::ExecuteCommands();
		Renderer3D::WaitForGPU();
		RenderCommand::ResetFrameCommandUnits(0u); //TO BE CHANGED! UPLOAD BUFFER SHOULD UPLOAD EVERYTHING SEQUENTIALLY!
	}

	void Renderer3D::Begin(const std::shared_ptr<PerspectiveCamera>& pSceneCamera, EntityManager& entityManager, Scene& scene) noexcept
	{
		s_RendererData.m_ForwardPassEntities.clear();
		s_RendererData.m_PickingPassEntities.clear();

		RenderCommand::ResetFrameCommandUnits(s_RendererData.currentFrameIndex);

		RenderCommand::SetViewport(s_RendererData.viewPort);
		RenderCommand::SetScissorRect(s_RendererData.scissorRect);

		RenderCommand::TransitionResource(s_RendererData.m_pMSAARenderTexture, D3D12_RESOURCE_STATE_RENDER_TARGET);

		RenderCommand::ClearRenderTarget(s_RendererData.m_pMSAARenderTexture->GetRTVDescriptorHandle().CPUHandle, s_RendererData.m_pMSAARenderTexture->GetClearColor());

		RenderCommand::ClearDepthStencil(s_RendererData.m_pMSAADepthStencil);

		RenderCommand::SetRenderTarget(s_RendererData.m_pMSAARenderTexture, s_RendererData.m_pMSAADepthStencil);

		DXCall_STD(D3D12Core::GetCommandList()->SetDescriptorHeaps(1u, MemoryManager::Get().GetShaderBindableDescriptorHeap()->GetDescriptorHeapInterface().GetAddressOf()));
		RenderCommand::SetRootSignature(s_RendererData.pRootSignature);
		RenderCommand::SetPipelineState(s_RendererData.pPipelineState);
		RenderCommand::SetTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		static VP vpMatrixCBuffer;
		auto vpMatrix = DirectX::XMLoadFloat4x4(&(pSceneCamera->GetViewProjectionMatrix()));
		DirectX::XMStoreFloat4x4(&vpMatrixCBuffer.VPMatrix, vpMatrix);
		DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRoot32BitConstants(2u, 4 * 4, &vpMatrixCBuffer, 0u));

		//auto frameIndex = Window::GetCurrentBackbufferIndex() % D3D12Core::GetNrOfBufferedFrames();
		auto frameIndex = D3D12Core::GetCurrentFrame() % D3D12Core::GetNrOfBufferedFrames();

		static PerFrameData2 perFrameData2;
		perFrameData2.cameraDataIndex = pSceneCamera->m_pConstantBuffer->m_VisibleHandles[frameIndex].Index;
		perFrameData2.pointLightStructuredBufferIndex = scene.GetLightManager().GetPointLights()->m_VisibleHandles[frameIndex].Index;
		perFrameData2.directionalLightStructuredBufferIndex = scene.GetLightManager().GetDirectionalLights()->m_VisibleHandles[frameIndex].Index;

		{
			uint32_t i = 0u;
			entityManager.Collect<DirectionalLightComponent>().Do([&](DirectionalLightComponent&)
				{
					i++;
				});
			perFrameData2.nrOfDirectionalLights = i;
		}
		{
			uint32_t i = 0u;
			entityManager.Collect<PointLightComponent>().Do([&](PointLightComponent&)
				{
					i++;
				});
			perFrameData2.nrOfPointLights = i;
		}

		DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRoot32BitConstants(5, (uint32_t)sizeof(PerFrameData2) / sizeof(uint32_t), &perFrameData2, 0u));
	}

	void Renderer3D::Submit(const entity e) noexcept
	{
		s_RendererData.m_ForwardPassEntities.push_back(e);
		s_RendererData.m_PickingPassEntities.push_back(e);
	}

	void Renderer3D::End(const EntityManager& entityManager) noexcept
	{
		AssetManager& assetManager = AssetManager::Get();

		//Forward pass:
		for (auto e : s_RendererData.m_ForwardPassEntities)
		{
			auto& mfc = entityManager.Get<MeshFilterComponent>(e);
			VertexBuffer* vb = assetManager.GetAsset<VertexBuffer>(mfc.VertexBufferID);
			IndexBuffer* ib = assetManager.GetAsset<IndexBuffer>(mfc.IndexBufferID);

			DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRootShaderResourceView(0u, vb->GetInterface()->GetGPUVirtualAddress()));
			DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRootShaderResourceView(1u, ib->GetInterface()->GetGPUVirtualAddress()));

			static World worldMatrixCBuffer;
			worldMatrixCBuffer.WorldMatrix = entityManager.Get<TransformComponent>(e).Transform;
			DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRoot32BitConstants(3u, 4 * 4, &worldMatrixCBuffer, 0u));
			static PerDrawData2 perDrawData2;
			auto& mrc = entityManager.Get<MeshRendererComponent>(e);

			auto frameIndex = D3D12Core::GetCurrentFrame() % D3D12Core::GetNrOfBufferedFrames();
			
			perDrawData2.colorIndex = mrc.constantBuffer->m_VisibleHandles[frameIndex].Index;
			DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRoot32BitConstants(4, 1, &perDrawData2, 0u));

			RenderCommand::DrawInstanced(ib->GetNrOfIndices());
		}

		//Picking pass:
		//States:
		{
			RenderCommand::TransitionResource(s_RendererData.m_pIdentifierRenderTexture, D3D12_RESOURCE_STATE_RENDER_TARGET);
			RenderCommand::ClearRenderTarget(s_RendererData.m_pIdentifierRenderTexture->GetRTVDescriptorHandle().CPUHandle, s_RendererData.m_pIdentifierRenderTexture->GetClearColor());
			RenderCommand::ClearDepthStencil(s_RendererData.m_pPickingDepthStencil);
			RenderCommand::SetRootSignature(s_RendererData.pPickingRootSignature);
			RenderCommand::SetPipelineState(s_RendererData.pPickingPipelineState);
			RenderCommand::SetRenderTarget(s_RendererData.m_pIdentifierRenderTexture, s_RendererData.m_pPickingDepthStencil);
		}
		static Identifier ID;

		for (auto e : s_RendererData.m_PickingPassEntities)
		{
			auto& mfc = entityManager.Get<MeshFilterComponent>(e);
			VertexBuffer* vb = assetManager.GetAsset<VertexBuffer>(mfc.VertexBufferID);
			IndexBuffer* ib = assetManager.GetAsset<IndexBuffer>(mfc.IndexBufferID);
			auto& worldMatrix = entityManager.Get<TransformComponent>(e).Transform;

			DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRootShaderResourceView(0u, vb->GetInterface()->GetGPUVirtualAddress()));
			DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRootShaderResourceView(1u, ib->GetInterface()->GetGPUVirtualAddress()));
			
			ID.entityID = e;
			DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRoot32BitConstants(4u, 1u, &ID, 0u));

			static World worldMatrixCBuffer;
			worldMatrixCBuffer.WorldMatrix = worldMatrix;
			DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRoot32BitConstants(3u, 4 * 4, &worldMatrixCBuffer, 0u));
			RenderCommand::DrawInstanced(ib->GetNrOfIndices());
		}
		RenderCommand::TransitionResource(s_RendererData.m_pIdentifierRenderTexture, D3D12_RESOURCE_STATE_COPY_SOURCE);

		D3D12_PLACED_SUBRESOURCE_FOOTPRINT footPrint = {};
		UINT numRows{};
		UINT64 rowSizeInBytes{};
		UINT64 totalBytes{};

		auto desc = s_RendererData.m_pIdentifierRenderTexture->GetInterface()->GetDesc();

		DXCall_STD(D3D12Core::GetDevice()->GetCopyableFootprints(&desc, 0u, 1u, 0u, &footPrint, &numRows, &rowSizeInBytes, &totalBytes));

		D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
		dstLocation.pResource = s_RendererData.m_pIdentifierReadbackTexture->GetInterface().Get();
		dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		dstLocation.PlacedFootprint = footPrint;

		D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
		srcLocation.pResource = s_RendererData.m_pIdentifierRenderTexture->GetInterface().Get();
		srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		srcLocation.SubresourceIndex = 0u;

		DXCall_STD(D3D12Core::GetCommandList()->CopyTextureRegion(&dstLocation, 0u, 0u ,0u, &srcLocation, nullptr));
		s_RendererData.totalBytes = static_cast<uint32_t>(totalBytes);

		//MSAA Resolve:
		{
			RenderCommand::TransitionResource(s_RendererData.m_pMSAARenderTexture, D3D12_RESOURCE_STATE_RESOLVE_SOURCE);
			RenderCommand::TransitionResource(s_RendererData.m_pPostProcessRenderTexture, D3D12_RESOURCE_STATE_RESOLVE_DEST);
			RenderCommand::ResolveMSAA(s_RendererData.m_pMSAARenderTexture, s_RendererData.m_pPostProcessRenderTexture);
		}

		//Set post process render texture as pixel shader resource and UI-texture as render target:
		{
			RenderCommand::TransitionResource(s_RendererData.m_pPostProcessRenderTexture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			RenderCommand::TransitionResource(ImguiLayer::GetUITexture(), D3D12_RESOURCE_STATE_RENDER_TARGET);
			RenderCommand::SetRenderTarget(ImguiLayer::GetUITexture(), nullptr);
		}

		//Set necessary d3d12 states:
		{
			DXCall_STD(D3D12Core::GetCommandList()->SetDescriptorHeaps(1u, MemoryManager::Get().GetShaderBindableDescriptorHeap()->GetDescriptorHeapInterface().GetAddressOf()));
			RenderCommand::SetRootSignature(s_RendererData.pPostProcessRootSignature);
			RenderCommand::SetPipelineState(s_RendererData.pPostProcessPipelineState);
		}

		//Post process:
		{
			static PerFrameData textureData;
			textureData.PostProcessTextureIndex = s_RendererData.m_pPostProcessRenderTexture->GetSRVDescriptorHandle().Index;
			constexpr uint32_t COUNT = 1u;
			DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRoot32BitConstants(6, COUNT, &textureData, 0u));
			RenderCommand::DrawInstanced(3u);
		}

		//Set UI-texture as pixel shader resource and prepare back buffer as render target for imgui:
		{
			RenderCommand::TransitionResource(ImguiLayer::GetUITexture(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			BackBuffer backBuffer = Window::GetCurrentBackBuffer();
			RenderCommand::TransitionResource(backBuffer.pBackBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
			RenderCommand::SetRenderTarget(backBuffer);
		}
	}

	void Renderer3D::PrepareBackBuffer() noexcept
	{
		//Transition the back buffer back to present state now that is is finished:
		BackBuffer& backBuffer{ Window::GetCurrentBackBuffer() };
		RenderCommand::TransitionResource(backBuffer.pBackBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	}

	void Renderer3D::ExecuteCommands() noexcept
	{
		auto pCommandList{ D3D12Core::GetCommandList() };
		DXCall(pCommandList->Close());
		ID3D12CommandList* pCommandLists[] = { pCommandList.Get() };
		DXCall_STD(D3D12Core::GetCommandQueue()->ExecuteCommandLists(ARRAYSIZE(pCommandLists), pCommandLists));
	}

	void Renderer3D::WaitAndSync() noexcept
	{
		// Schedule a Signal command in the queue.
		const UINT64 currentFenceValue = s_RendererData.pFenceValues[s_RendererData.currentFrameIndex];
		DXCall(D3D12Core::GetCommandQueue()->Signal(s_RendererData.pFence.Get(), currentFenceValue));

		// Update the frame index.
		//s_RendererData.currentFrameIndex = Window::GetCurrentBackbufferIndex();

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

	const std::shared_ptr<RenderTexture>& Renderer3D::GetViewportTexture() noexcept
	{
		return s_RendererData.m_pMSAARenderTexture;
	}

	void Renderer3D::OnSceneViewportChanged(const uint32_t width, const uint32_t height) noexcept
	{
		RenderTextureSpecification textureSpecification = {};
		textureSpecification.Width = width;
		textureSpecification.Height = height;
		textureSpecification.Format = s_RendererData.m_pMSAARenderTexture->GetFormat();
		textureSpecification.MultiSampleCount = s_RendererData.m_pMSAARenderTexture->GetMultiSampleCount();
		textureSpecification.ClearColor = DirectX::XMFLOAT4(DirectX::Colors::CornflowerBlue);
		textureSpecification.CreateSRV = true;
		
		MemoryManager::Get().DestroyDescriptorHandle(s_RendererData.m_pMSAARenderTexture->GetSRVDescriptorHandle());
		MemoryManager::Get().DestroyDescriptorHandle(s_RendererData.m_pMSAARenderTexture->GetRTVDescriptorHandle());
		MemoryManager::Get().DestroyResource(std::move(s_RendererData.m_pMSAARenderTexture));
		s_RendererData.m_pMSAARenderTexture = std::move(RenderTexture::Create(textureSpecification, "Main MSAA RenderTexture"));

		textureSpecification.MultiSampleCount = 1u;
		textureSpecification.ClearColor = DirectX::XMFLOAT4(DirectX::Colors::Black);
		MemoryManager::Get().DestroyDescriptorHandle(s_RendererData.m_pPostProcessRenderTexture->GetSRVDescriptorHandle());
		MemoryManager::Get().DestroyDescriptorHandle(s_RendererData.m_pPostProcessRenderTexture->GetRTVDescriptorHandle());
		MemoryManager::Get().DestroyResource(std::move(s_RendererData.m_pPostProcessRenderTexture));
		s_RendererData.m_pPostProcessRenderTexture = std::move(RenderTexture::Create(textureSpecification, "Post process RenderTexture"));

		textureSpecification.MultiSampleCount = 1u;
		textureSpecification.Format = DXGI_FORMAT::DXGI_FORMAT_R32_UINT;
		textureSpecification.CreateSRV = false;
		MemoryManager::Get().DestroyDescriptorHandle(s_RendererData.m_pIdentifierRenderTexture->GetRTVDescriptorHandle());
		MemoryManager::Get().DestroyResource(std::move(s_RendererData.m_pIdentifierRenderTexture));
		
		static constexpr uint32_t clearVal = NULL_ENTITY;//10'000'000'000;

		textureSpecification.ClearColor = DirectX::XMFLOAT4(clearVal, clearVal, clearVal, clearVal);
		textureSpecification.isSRGB = false;
		s_RendererData.m_pIdentifierRenderTexture = std::move(RenderTexture::Create(textureSpecification, "Identifier RenderTexture"));

		D3D12_PLACED_SUBRESOURCE_FOOTPRINT footPrint = {};
		UINT numRows{};
		UINT64 rowSizeInBytes{};
		UINT64 totalBytes{};

		auto desc = s_RendererData.m_pIdentifierRenderTexture->GetInterface()->GetDesc();

		DXCall_STD(D3D12Core::GetDevice()->GetCopyableFootprints(&desc, 0u, 1u, 0u, &footPrint, &numRows, &rowSizeInBytes, &totalBytes));

		ReadbackTextureSpecification RBTextureSpecification = {};
		RBTextureSpecification.Width = static_cast<uint32_t>(totalBytes);
		RBTextureSpecification.Height = 1u;
		RBTextureSpecification.Format = textureSpecification.Format;
		RBTextureSpecification.MultiSampleCount = textureSpecification.MultiSampleCount;
		RBTextureSpecification.ClearColor = textureSpecification.ClearColor;
		MemoryManager::Get().DestroyResource(std::move(s_RendererData.m_pIdentifierReadbackTexture));
		s_RendererData.m_pIdentifierReadbackTexture = std::move(ReadbackTexture::Create(RBTextureSpecification, "Identifier ReadbackTexture"));

		MemoryManager::Get().DestroyDescriptorHandle(s_RendererData.m_pMSAADepthStencil->GetDSVDescriptorHandle());
		MemoryManager::Get().DestroyResource(std::move(s_RendererData.m_pMSAADepthStencil));
		s_RendererData.m_pMSAADepthStencil = std::move(DepthStencil::Create(width, height, s_RendererData.m_pMSAARenderTexture->GetMultiSampleCount(), "Main MSAA DepthStencil"));

		MemoryManager::Get().DestroyDescriptorHandle(s_RendererData.m_pPickingDepthStencil->GetDSVDescriptorHandle());
		MemoryManager::Get().DestroyResource(std::move(s_RendererData.m_pPickingDepthStencil));
		s_RendererData.m_pPickingDepthStencil = std::move(DepthStencil::Create(width, height, 1, "Picking DepthStencil"));

		s_RendererData.viewPort.Width = static_cast<float>(width);
		s_RendererData.viewPort.Height = static_cast<float>(height);

		s_RendererData.scissorRect.right = static_cast<LONG>(s_RendererData.viewPort.Width);
		s_RendererData.scissorRect.bottom = static_cast<LONG>(s_RendererData.viewPort.Height);
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

		D3D12_ROOT_PARAMETER worldMatrixRootParameterVS = {};
		worldMatrixRootParameterVS.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		worldMatrixRootParameterVS.Constants.Num32BitValues = 4 * 4;
		worldMatrixRootParameterVS.Constants.RegisterSpace = 0u;
		worldMatrixRootParameterVS.Constants.ShaderRegister = 1u;
		worldMatrixRootParameterVS.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
		rootParameters.push_back(worldMatrixRootParameterVS);

		D3D12_ROOT_PARAMETER perDrawPS = {};
		perDrawPS.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		perDrawPS.Constants.Num32BitValues = 1u;
		perDrawPS.Constants.RegisterSpace = 0u;
		perDrawPS.Constants.ShaderRegister = 3u;
		perDrawPS.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		rootParameters.push_back(perDrawPS);

		D3D12_ROOT_PARAMETER perFramePS = {};
		perFramePS.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		perFramePS.Constants.Num32BitValues = (uint32_t)sizeof(PerFrameData2) / sizeof(uint32_t);
		perFramePS.Constants.RegisterSpace = 0u;
		perFramePS.Constants.ShaderRegister = 4u;
		perFramePS.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		rootParameters.push_back(perFramePS);

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

		D3D12_ROOT_PARAMETER worldMatrixRootParameterVS = {};
		worldMatrixRootParameterVS.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		worldMatrixRootParameterVS.Constants.Num32BitValues = 4 * 4;
		worldMatrixRootParameterVS.Constants.RegisterSpace = 0u;
		worldMatrixRootParameterVS.Constants.ShaderRegister = 1u;
		worldMatrixRootParameterVS.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
		rootParameters.push_back(worldMatrixRootParameterVS);

		D3D12_ROOT_PARAMETER textureIndexIdentifierRootParameterPS = {};
		textureIndexIdentifierRootParameterPS.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		textureIndexIdentifierRootParameterPS.Constants.Num32BitValues = 1;
		textureIndexIdentifierRootParameterPS.Constants.RegisterSpace = 1u;
		textureIndexIdentifierRootParameterPS.Constants.ShaderRegister = 0u;
		textureIndexIdentifierRootParameterPS.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		rootParameters.push_back(textureIndexIdentifierRootParameterPS);

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
	
	void Renderer3D::CreateMainPipelineState() noexcept
	{
		//We need a rasterizer descriptor:
		D3D12_RASTERIZER_DESC rasterizerDescriptor = {};
		rasterizerDescriptor.FillMode = D3D12_FILL_MODE_SOLID;
		rasterizerDescriptor.CullMode = D3D12_CULL_MODE_BACK;
		rasterizerDescriptor.FrontCounterClockwise = FALSE;
		rasterizerDescriptor.DepthBias = 0;
		rasterizerDescriptor.DepthBiasClamp = 0.0f;
		rasterizerDescriptor.SlopeScaledDepthBias = 0.0f;
		rasterizerDescriptor.DepthClipEnable = TRUE;
		rasterizerDescriptor.MultisampleEnable = TRUE;
		rasterizerDescriptor.AntialiasedLineEnable = TRUE;
		rasterizerDescriptor.ForcedSampleCount = 0u;
		rasterizerDescriptor.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

		//We also need a blend descriptor:
		D3D12_RENDER_TARGET_BLEND_DESC blendDescriptor = {};
		blendDescriptor.BlendEnable = FALSE;
		blendDescriptor.LogicOpEnable = FALSE;
		blendDescriptor.SrcBlend = D3D12_BLEND_ONE;
		blendDescriptor.DestBlend = D3D12_BLEND_ZERO;
		blendDescriptor.BlendOp = D3D12_BLEND_OP_ADD;
		blendDescriptor.SrcBlendAlpha = D3D12_BLEND_ONE;
		blendDescriptor.DestBlendAlpha = D3D12_BLEND_ZERO;
		blendDescriptor.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		blendDescriptor.LogicOp = D3D12_LOGIC_OP_NOOP;
		blendDescriptor.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		//And a depth stencil descriptor:
		D3D12_DEPTH_STENCIL_DESC depthStencilDescriptor = {};
		depthStencilDescriptor.DepthEnable = TRUE;
		depthStencilDescriptor.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		depthStencilDescriptor.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		depthStencilDescriptor.StencilEnable = FALSE;
		depthStencilDescriptor.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
		depthStencilDescriptor.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
		depthStencilDescriptor.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
		depthStencilDescriptor.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
		depthStencilDescriptor.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
		depthStencilDescriptor.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
		depthStencilDescriptor.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
		depthStencilDescriptor.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
		depthStencilDescriptor.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		depthStencilDescriptor.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

		//We also need a Stream Output Descriptor:
		D3D12_STREAM_OUTPUT_DESC streamOutputDescriptor = {};
		streamOutputDescriptor.pSODeclaration = nullptr;
		streamOutputDescriptor.NumEntries = 0u;
		streamOutputDescriptor.pBufferStrides = nullptr;
		streamOutputDescriptor.NumStrides = 0u;
		streamOutputDescriptor.RasterizedStream = 0u;

		//We now create the Graphics Pipe line state, the PSO:
		std::array<DXGI_FORMAT, 1> rtvFormats = { DXGI_FORMAT_R32G32B32A32_FLOAT };
		s_RendererData.psoDescriptor.pRootSignature = s_RendererData.pRootSignature.Get();
		s_RendererData.psoDescriptor.VS.pShaderBytecode = s_RendererData.m_ShaderLibrary.Get("VertexShader")->GetBuffer()->GetBufferPointer();
		s_RendererData.psoDescriptor.VS.BytecodeLength = s_RendererData.m_ShaderLibrary.Get("VertexShader")->GetBuffer()->GetBufferSize();
		s_RendererData.psoDescriptor.PS.pShaderBytecode = s_RendererData.m_ShaderLibrary.Get("PixelShader")->GetBuffer()->GetBufferPointer();
		s_RendererData.psoDescriptor.PS.BytecodeLength = s_RendererData.m_ShaderLibrary.Get("PixelShader")->GetBuffer()->GetBufferSize();

		s_RendererData.psoDescriptor.SampleMask = UINT_MAX;
		s_RendererData.psoDescriptor.RasterizerState = rasterizerDescriptor;
		s_RendererData.psoDescriptor.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		s_RendererData.psoDescriptor.NumRenderTargets = static_cast<UINT>(rtvFormats.size());

		s_RendererData.psoDescriptor.BlendState.AlphaToCoverageEnable = false;
		s_RendererData.psoDescriptor.BlendState.IndependentBlendEnable = false;
		s_RendererData.psoDescriptor.RTVFormats[0] = rtvFormats[0];
		s_RendererData.psoDescriptor.BlendState.RenderTarget[0] = blendDescriptor;

		s_RendererData.psoDescriptor.SampleDesc.Count = 8u;
		s_RendererData.psoDescriptor.SampleDesc.Quality = 0u;
		s_RendererData.psoDescriptor.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		s_RendererData.psoDescriptor.DepthStencilState = depthStencilDescriptor;
		s_RendererData.psoDescriptor.StreamOutput = streamOutputDescriptor;
		s_RendererData.psoDescriptor.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

		DXCall(D3D12Core::GetDevice()->CreateGraphicsPipelineState(&s_RendererData.psoDescriptor, IID_PPV_ARGS(&s_RendererData.pPipelineState)));
	
		//POST PROCESS!
		s_RendererData.psoDescriptor.pRootSignature = s_RendererData.pPostProcessRootSignature.Get();

		s_RendererData.psoDescriptor.VS.pShaderBytecode = s_RendererData.m_ShaderLibrary.Get("FullScreenTriVertexShader")->GetBuffer()->GetBufferPointer();
		s_RendererData.psoDescriptor.VS.BytecodeLength = s_RendererData.m_ShaderLibrary.Get("FullScreenTriVertexShader")->GetBuffer()->GetBufferSize();
		s_RendererData.psoDescriptor.PS.pShaderBytecode = s_RendererData.m_ShaderLibrary.Get("PostProcessPixelShader")->GetBuffer()->GetBufferPointer();
		s_RendererData.psoDescriptor.PS.BytecodeLength = s_RendererData.m_ShaderLibrary.Get("PostProcessPixelShader")->GetBuffer()->GetBufferSize();

		rasterizerDescriptor.MultisampleEnable = FALSE;
		rasterizerDescriptor.AntialiasedLineEnable = FALSE;
		depthStencilDescriptor.DepthEnable = FALSE;

		s_RendererData.psoDescriptor.SampleDesc.Count = 1u;
		s_RendererData.psoDescriptor.DepthStencilState = depthStencilDescriptor;

		DXCall(D3D12Core::GetDevice()->CreateGraphicsPipelineState(&s_RendererData.psoDescriptor, IID_PPV_ARGS(&s_RendererData.pPostProcessPipelineState)));
	}

	void Renderer3D::CreatePickingPipelineState() noexcept
	{
		//We need a rasterizer descriptor:
		D3D12_RASTERIZER_DESC rasterizerDescriptor = {};
		rasterizerDescriptor.FillMode = D3D12_FILL_MODE_SOLID;
		rasterizerDescriptor.CullMode = D3D12_CULL_MODE_BACK;
		rasterizerDescriptor.FrontCounterClockwise = FALSE;
		rasterizerDescriptor.DepthBias = 0;
		rasterizerDescriptor.DepthBiasClamp = 0.0f;
		rasterizerDescriptor.SlopeScaledDepthBias = 0.0f;
		rasterizerDescriptor.DepthClipEnable = TRUE;
		rasterizerDescriptor.MultisampleEnable = TRUE;
		rasterizerDescriptor.AntialiasedLineEnable = TRUE;
		rasterizerDescriptor.ForcedSampleCount = 0u;
		rasterizerDescriptor.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

		//We also need a blend descriptor:
		D3D12_RENDER_TARGET_BLEND_DESC blendDescriptor = {};
		blendDescriptor.BlendEnable = FALSE;
		blendDescriptor.LogicOpEnable = FALSE;
		blendDescriptor.SrcBlend = D3D12_BLEND_ONE;
		blendDescriptor.DestBlend = D3D12_BLEND_ZERO;
		blendDescriptor.BlendOp = D3D12_BLEND_OP_ADD;
		blendDescriptor.SrcBlendAlpha = D3D12_BLEND_ONE;
		blendDescriptor.DestBlendAlpha = D3D12_BLEND_ZERO;
		blendDescriptor.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		blendDescriptor.LogicOp = D3D12_LOGIC_OP_NOOP;
		blendDescriptor.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		//And a depth stencil descriptor:
		D3D12_DEPTH_STENCIL_DESC depthStencilDescriptor = {};
		depthStencilDescriptor.DepthEnable = TRUE;
		depthStencilDescriptor.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		depthStencilDescriptor.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		depthStencilDescriptor.StencilEnable = FALSE;
		depthStencilDescriptor.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
		depthStencilDescriptor.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
		depthStencilDescriptor.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
		depthStencilDescriptor.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
		depthStencilDescriptor.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
		depthStencilDescriptor.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
		depthStencilDescriptor.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
		depthStencilDescriptor.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
		depthStencilDescriptor.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		depthStencilDescriptor.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

		//We also need a Stream Output Descriptor:
		D3D12_STREAM_OUTPUT_DESC streamOutputDescriptor = {};
		streamOutputDescriptor.pSODeclaration = nullptr;
		streamOutputDescriptor.NumEntries = 0u;
		streamOutputDescriptor.pBufferStrides = nullptr;
		streamOutputDescriptor.NumStrides = 0u;
		streamOutputDescriptor.RasterizedStream = 0u;

		//We now create the Graphics Pipe line state, the PSO:
		std::array<DXGI_FORMAT, 1> rtvFormats = { DXGI_FORMAT_R32_UINT };
		s_RendererData.pickingPsoDescriptor.pRootSignature = s_RendererData.pPickingRootSignature.Get();
		s_RendererData.pickingPsoDescriptor.VS.pShaderBytecode = s_RendererData.m_ShaderLibrary.Get("VertexShader")->GetBuffer()->GetBufferPointer();
		s_RendererData.pickingPsoDescriptor.VS.BytecodeLength = s_RendererData.m_ShaderLibrary.Get("VertexShader")->GetBuffer()->GetBufferSize();
		s_RendererData.pickingPsoDescriptor.PS.pShaderBytecode = s_RendererData.m_ShaderLibrary.Get("PickingPixelShader")->GetBuffer()->GetBufferPointer();
		s_RendererData.pickingPsoDescriptor.PS.BytecodeLength = s_RendererData.m_ShaderLibrary.Get("PickingPixelShader")->GetBuffer()->GetBufferSize();

		s_RendererData.pickingPsoDescriptor.SampleMask = UINT_MAX;
		s_RendererData.pickingPsoDescriptor.RasterizerState = rasterizerDescriptor;
		s_RendererData.pickingPsoDescriptor.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		s_RendererData.pickingPsoDescriptor.NumRenderTargets = static_cast<UINT>(rtvFormats.size());

		s_RendererData.pickingPsoDescriptor.BlendState.AlphaToCoverageEnable = false;
		s_RendererData.pickingPsoDescriptor.BlendState.IndependentBlendEnable = false;
		s_RendererData.pickingPsoDescriptor.RTVFormats[0] = rtvFormats[0];
		s_RendererData.pickingPsoDescriptor.BlendState.RenderTarget[0] = blendDescriptor;

		s_RendererData.pickingPsoDescriptor.SampleDesc.Count = 1u;
		s_RendererData.pickingPsoDescriptor.SampleDesc.Quality = 0u;
		s_RendererData.pickingPsoDescriptor.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		s_RendererData.pickingPsoDescriptor.DepthStencilState = depthStencilDescriptor;
		s_RendererData.pickingPsoDescriptor.StreamOutput = streamOutputDescriptor;
		s_RendererData.pickingPsoDescriptor.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

		DXCall(D3D12Core::GetDevice()->CreateGraphicsPipelineState(&s_RendererData.pickingPsoDescriptor, IID_PPV_ARGS(&s_RendererData.pPickingPipelineState)));
	}

	entity Renderer3D::GetHoveredEntity(const uint32_t x, const uint32_t y) noexcept
	{
			//WaitForGPU();
			D3D12_RANGE readBackBufferRange{ 0, s_RendererData.totalBytes };
			uint32_t* pReadBackBufferData{};
			DXCall(s_RendererData.m_pIdentifierReadbackTexture->GetInterface()->Map
			(
				0u,
				&readBackBufferRange,
				reinterpret_cast<void**>(&pReadBackBufferData)
			));

			auto desc = s_RendererData.m_pIdentifierRenderTexture->GetInterface()->GetDesc();
			D3D12_PLACED_SUBRESOURCE_FOOTPRINT footPrint{};

			DXCall_STD(D3D12Core::GetDevice()->GetCopyableFootprints(&desc, 0u, 1u, 0u, &footPrint, nullptr, nullptr, nullptr));
			const uint32_t index = (y * (footPrint.Footprint.RowPitch / 4)) + x;
			const uint32_t indexExpressedAsBytes = index * 4;

			bool outsideOfViewport = indexExpressedAsBytes > s_RendererData.totalBytes;
			if (outsideOfViewport)
				return NULL_ENTITY;

			s_RendererData.hoveredEntity = pReadBackBufferData[index];

			D3D12_RANGE emptyRange{ 0, 0 };
			DXCall_STD(s_RendererData.m_pIdentifierReadbackTexture->GetInterface()->Unmap
			(
				0,
				&emptyRange
			));
		return s_RendererData.hoveredEntity;
	}
}