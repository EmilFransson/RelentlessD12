#include "Renderer3D.h"
#include "../Resources/Texture.h"
#include "../Resources/DepthStencil.h"
#include "../D3D12Core.h"
#include "RenderCommand.h"
#include "../../Window.h"
#include "../../ImGui/ImguiLayer.h"
#include "../MemoryManager.h"
#include "../../Mesh/Triangle.h"
#include "Camera/PerspectiveCamera.h"
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
		std::shared_ptr<DepthStencil> m_pMSAADepthStencil{ nullptr };
		D3D12_VIEWPORT viewPort{};
		RECT scissorRect{};
		Microsoft::WRL::ComPtr<ID3D12RootSignature> pRootSignature{ nullptr };
		Microsoft::WRL::ComPtr<ID3D12PipelineState> pPipelineState{ nullptr };

		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDescriptor{};
	};
	static Renderer3dData s_RendererData = {};
	
	void Renderer3D::Initialize() noexcept
	{
		s_RendererData.pFenceValues = std::move(std::make_unique<uint64_t[]>(D3D12Core::GetNrOfBufferedFrames()));

		DXCall(D3D12Core::GetDevice()->CreateFence(0u, D3D12_FENCE_FLAGS::D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&s_RendererData.pFence)));
		s_RendererData.pFenceValues[s_RendererData.currentFrameIndex]++;
		s_RendererData.fenceEvent = ::CreateEvent(nullptr, false, false, nullptr);
		RLS_ASSERT(s_RendererData.fenceEvent, "Fence event creation failed.");

		TextureSpecification textureSpecification = {};
		textureSpecification.Width = 800u;
		textureSpecification.Height = 600u;
		textureSpecification.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		textureSpecification.MultiSampleCount = 8u;
		textureSpecification.ClearColor = DirectX::XMFLOAT4(DirectX::Colors::Brown);
		textureSpecification.CreateDescriptorHandles = true;
		s_RendererData.m_pMSAARenderTexture = std::move(RenderTexture::Create(textureSpecification, "Main MSAA RenderTexture"));

		s_RendererData.m_pMSAADepthStencil = std::move(DepthStencil::Create(800u, 600u, 8u, "Main MSAA DepthStencil"));

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

		CreateTestRootSignature();
		CreateTestPipelineState();
	}

	void Renderer3D::Begin(const std::shared_ptr<PerspectiveCamera>& pSceneCamera) noexcept
	{
		RenderCommand::ResetFrameCommandUnits(s_RendererData.currentFrameIndex);

		RenderCommand::SetViewport(s_RendererData.viewPort);
		RenderCommand::SetScissorRect(s_RendererData.scissorRect);

		RenderCommand::TransitionResource(s_RendererData.m_pMSAARenderTexture, D3D12_RESOURCE_STATE_RENDER_TARGET);

		RenderCommand::ClearRenderTarget(s_RendererData.m_pMSAARenderTexture->GetRTVDescriptorHandle().CPUHandle, DirectX::Colors::Brown);
		RenderCommand::ClearDepthStencil(s_RendererData.m_pMSAADepthStencil);
		RenderCommand::SetRenderTarget(s_RendererData.m_pMSAARenderTexture, s_RendererData.m_pMSAADepthStencil);
	
		RenderCommand::SetRootSignature(s_RendererData.pRootSignature);
		RenderCommand::SetPipelineState(s_RendererData.pPipelineState);
		RenderCommand::SetTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		static VP vpMatrixCBuffer;
		auto vpMatrix = DirectX::XMLoadFloat4x4(&(pSceneCamera->GetViewProjectionMatrix()));
		vpMatrix = DirectX::XMMatrixTranspose(vpMatrix);
		DirectX::XMStoreFloat4x4(&vpMatrixCBuffer.VPMatrix, vpMatrix);
		DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRoot32BitConstants(2u, 4 * 4, &vpMatrixCBuffer, 0u));
	}

	void Renderer3D::Submit(const std::shared_ptr<Triangle>& pTriangle) noexcept
	{
		DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRootShaderResourceView(0u, pTriangle->GetVertexBuffer()->GetGPUVirtualAddress()));
		DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRootShaderResourceView(1u, pTriangle->GetIndexBuffer()->GetGPUVirtualAddress()));
		
		static World worldMatrixCBuffer;
		auto worldMatrix = DirectX::XMLoadFloat4x4(&(pTriangle->GetWorldMatrix()));
		worldMatrix = DirectX::XMMatrixTranspose(worldMatrix);
		DirectX::XMStoreFloat4x4(&worldMatrixCBuffer.WorldMatrix, worldMatrix);
		DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRoot32BitConstants(3u, 4 * 4, &worldMatrixCBuffer, 0u));
		DXCall_STD(D3D12Core::GetCommandList()->DrawInstanced(pTriangle->GetNrOfIndices(), 1u, 0u, 0u));
	}

	void Renderer3D::End() noexcept
	{
		RenderCommand::TransitionResource(s_RendererData.m_pMSAARenderTexture,D3D12_RESOURCE_STATE_RESOLVE_SOURCE);
		RenderCommand::TransitionResource(ImguiLayer::GetUITexture(), D3D12_RESOURCE_STATE_RESOLVE_DEST);
		 
		RenderCommand::ResolveMSAA(s_RendererData.m_pMSAARenderTexture, ImguiLayer::GetUITexture());

		RenderCommand::TransitionResource(ImguiLayer::GetUITexture(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

		BackBuffer backBuffer = Window::GetCurrentBackBuffer();
		RenderCommand::TransitionResource(backBuffer.pBackBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		RenderCommand::SetRenderTarget(backBuffer);
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
		s_RendererData.currentFrameIndex = Window::Get().GetCurrentBackbufferIndex();

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
		TextureSpecification textureSpecification = {};
		textureSpecification.Width = width;
		textureSpecification.Height = height;
		textureSpecification.Format = s_RendererData.m_pMSAARenderTexture->GetFormat();
		textureSpecification.MultiSampleCount = s_RendererData.m_pMSAARenderTexture->GetMultiSampleCount();
		textureSpecification.ClearColor = DirectX::XMFLOAT4(DirectX::Colors::Brown);
		textureSpecification.CreateDescriptorHandles = true;
		
		MemoryManager::Get().DestroyDescriptorHandle(s_RendererData.m_pMSAARenderTexture->GetSRVDescriptorHandle());
		MemoryManager::Get().DestroyDescriptorHandle(s_RendererData.m_pMSAARenderTexture->GetRTVDescriptorHandle());
		MemoryManager::Get().DestroyResource(std::move(s_RendererData.m_pMSAARenderTexture));
		s_RendererData.m_pMSAARenderTexture = std::move(RenderTexture::Create(textureSpecification, "Main MSAA RenderTexture"));

		MemoryManager::Get().DestroyDescriptorHandle(s_RendererData.m_pMSAADepthStencil->GetDSVDescriptorHandle());
		MemoryManager::Get().DestroyResource(std::move(s_RendererData.m_pMSAADepthStencil));
		s_RendererData.m_pMSAADepthStencil = std::move(DepthStencil::Create(width, height, s_RendererData.m_pMSAARenderTexture->GetMultiSampleCount(), "Main MSAA DepthStencil"));

		s_RendererData.viewPort.Width = static_cast<float>(width);
		s_RendererData.viewPort.Height = static_cast<float>(height);

		s_RendererData.scissorRect.right = static_cast<LONG>(s_RendererData.viewPort.Width);
		s_RendererData.scissorRect.bottom = static_cast<LONG>(s_RendererData.viewPort.Height);
	}

	void Renderer3D::CreateTestRootSignature() noexcept
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
		worldMatrixRootParameterVS.Constants.ShaderRegister = 1u;
		worldMatrixRootParameterVS.Constants.RegisterSpace = 0u;
		worldMatrixRootParameterVS.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
		rootParameters.push_back(worldMatrixRootParameterVS);

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
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS;

		Microsoft::WRL::ComPtr<ID3DBlob> pRootSignatureBlob{ nullptr };
		SERIALIZE_ROOT_SIGNATURE(rootSignatureDescriptor, pRootSignatureBlob);
		DXCall(D3D12Core::GetDevice()->CreateRootSignature
		(
			0u,
			pRootSignatureBlob->GetBufferPointer(),
			pRootSignatureBlob->GetBufferSize(),
			IID_PPV_ARGS(&s_RendererData.pRootSignature)
		));
	}
	
	void Renderer3D::CreateTestPipelineState() noexcept
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

		Microsoft::WRL::ComPtr<ID3DBlob> pvShaderBlob{ nullptr };
		COMPILE_SHADER_FROM_FILE("VertexShader.hlsl", "vs_main", "vs_5_1", pvShaderBlob);
		
		Microsoft::WRL::ComPtr<ID3DBlob> ppShaderBlob{ nullptr };
		COMPILE_SHADER_FROM_FILE("PixelShader.hlsl", "ps_main", "ps_5_1", ppShaderBlob);

		//We now create the Graphics Pipe line state, the PSO:
		std::array<DXGI_FORMAT, 1> rtvFormats = {DXGI_FORMAT_R8G8B8A8_UNORM_SRGB};
		s_RendererData.psoDescriptor.pRootSignature = s_RendererData.pRootSignature.Get();
		s_RendererData.psoDescriptor.VS.pShaderBytecode = pvShaderBlob->GetBufferPointer();
		s_RendererData.psoDescriptor.VS.BytecodeLength = pvShaderBlob->GetBufferSize();
		s_RendererData.psoDescriptor.PS.pShaderBytecode = ppShaderBlob->GetBufferPointer();
		s_RendererData.psoDescriptor.PS.BytecodeLength = ppShaderBlob->GetBufferSize();

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
	}
}