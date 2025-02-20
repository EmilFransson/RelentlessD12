#include "UtilityRenderer.h"
#include "Core/Application.h"
#include "Graphics/D3D12Core.h"
#include "Graphics/Resources/Texture.h"
#include "Graphics/Shaders/ShaderLibrary.h"
#include "MasterRenderer.h"
#include "Graphics/D3D12Debug.h"


namespace Relentless
{
	UtilityRenderer::UtilityRenderer()
	{
		const DirectX::XMMATRIX textureCubeGenerationProjectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV2, 1.0f, 0.1f, 1000.0f);

		DirectX::XMMATRIX faceViewMatrices[6] = {
		DirectX::XMMatrixLookAtLH(DirectX::XMVectorZero(), DirectX::XMVectorSet(1.0f,  0.0f,  0.0f, 0.0f), DirectX::XMVectorSet(0.0f,  1.0f,  0.0f, 0.0f)), // +X
		DirectX::XMMatrixLookAtLH(DirectX::XMVectorZero(), DirectX::XMVectorSet(-1.0f,  0.0f,  0.0f, 0.0f), DirectX::XMVectorSet(0.0f,  1.0f,  0.0f, 0.0f)), // -X
		DirectX::XMMatrixLookAtLH(DirectX::XMVectorZero(), DirectX::XMVectorSet(0.0f,  1.0f,  0.0f, 0.0f), DirectX::XMVectorSet(0.0f,  0.0f, -1.0f, 0.0f)), // +Y
		DirectX::XMMatrixLookAtLH(DirectX::XMVectorZero(), DirectX::XMVectorSet(0.0f, -1.0f,  0.0f, 0.0f), DirectX::XMVectorSet(0.0f,  0.0f,  1.0f, 0.0f)), // -Y
		DirectX::XMMatrixLookAtLH(DirectX::XMVectorZero(), DirectX::XMVectorSet(0.0f,  0.0f,  1.0f, 0.0f), DirectX::XMVectorSet(0.0f,  1.0f,  0.0f, 0.0f)), // +Z
		DirectX::XMMatrixLookAtLH(DirectX::XMVectorZero(), DirectX::XMVectorSet(0.0f,  0.0f, -1.0f, 0.0f), DirectX::XMVectorSet(0.0f,  1.0f,  0.0f, 0.0f))  // -Z
		};

		for (uint32_t i{ 0u }; i < 6; ++i)
		{
			const DirectX::XMMATRIX viewProjectionMatrix = DirectX::XMMatrixMultiply(faceViewMatrices[i],textureCubeGenerationProjectionMatrix);
			DirectX::XMStoreFloat4x4(&m_TextureCubeCreationViewProjectionMatrices[i].VPMatrix, viewProjectionMatrix);
			
			m_pTextureCubeCreationVPCBs[i] = std::make_unique<ConstantBufferSet>("m_pTextureCubeCreationVPCBs " + std::to_string(i), sizeof(DirectX::XMFLOAT4X4));
			
			ConstantBuffer2& constantBuffer = m_pTextureCubeCreationVPCBs[i]->At(0);
			void* pData = nullptr;
			D3D12_RANGE readRange = D3D12_RANGE(0,0);
			DXCall(constantBuffer.GetInterface()->Map(0u, &readRange, &pData));
			RLS_ASSERT(pData, "[UtilityRenderer]: Pointer to buffer write target is invalid.");
		
			std::memcpy(pData, &m_TextureCubeCreationViewProjectionMatrices[i], sizeof(TextureCubeCreationViewProjectionMatrix));

			DXCall_STD(constantBuffer.GetInterface()->Unmap(0u, nullptr));
		}

		//Radiance Map Roughness Data
		{
			for (uint32_t i = 0; i < 5; ++i)
			{
				m_pRadianceMapRougnessCBSets[i] = std::make_unique<ConstantBufferSet>("m_pRadianceMapRougnessCBSets " + std::to_string(i), sizeof(RadianceRoughnessData));
			}
		}

		//Equirectangular To CubeMap Pass:
		{
			ColorAttachment colorAttachment;
			colorAttachment.IsSRGB = false;
			colorAttachment.Format = TextureFormat::RGBA32F;
			colorAttachment.OperatorOnLoad = OperatorOnLoad::LoadOnly;
			colorAttachment.Blend = false;
			colorAttachment.ShouldResize = false;
			colorAttachment.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
			colorAttachment.Transfer = false;

			FrameBufferSpecification frameBufferSpecification{};
			frameBufferSpecification.DebugName = "Equirectangular To CubeMap Framebuffer";
			frameBufferSpecification.ShouldResize = false;
			frameBufferSpecification.Width = 512u;
			frameBufferSpecification.Height = 512u;
			frameBufferSpecification.Attachments.ColorAttachments = {colorAttachment};

			PipelineSpecification pipelineSpecification{};
			pipelineSpecification.DebugName = "Equirectangular To CubeMap Pipeline";
			pipelineSpecification.pVertexShader = MasterRenderer::GetShaderLibrary().Get("EquirectangularToCubemapVertexShader");
			pipelineSpecification.pPixelShader = MasterRenderer::GetShaderLibrary().Get("EquirectangularToCubemapPixelShader");
			pipelineSpecification.pFrameBuffer = FrameBuffer::Create(frameBufferSpecification);
			pipelineSpecification.MSAAEligible = false;
			pipelineSpecification.DepthWrite = false;
			pipelineSpecification.BackfaceCulling = false;

			RenderPassSpecification renderpassDescriptor{};
			renderpassDescriptor.DebugName = "Equirectangular To CubeMap Pass";
			renderpassDescriptor.RenderPipeline = Pipeline::Create(pipelineSpecification);

			m_pEquirectangularToCubeMapPass = RenderPass::Create(renderpassDescriptor);
		}

		//Create Irradiance Map Pass:
		{
			ColorAttachment colorAttachment;
			colorAttachment.IsSRGB = false;
			colorAttachment.Format = TextureFormat::RGBA32F;
			colorAttachment.OperatorOnLoad = OperatorOnLoad::LoadOnly;
			colorAttachment.Blend = false;
			colorAttachment.ShouldResize = false;
			colorAttachment.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
			colorAttachment.Transfer = false;

			FrameBufferSpecification frameBufferSpecification{};
			frameBufferSpecification.DebugName = "CubeMap Irradiance Convolution Framebuffer";
			frameBufferSpecification.ShouldResize = false;
			frameBufferSpecification.Width = 32u;
			frameBufferSpecification.Height = 32u;
			frameBufferSpecification.Attachments.ColorAttachments = { colorAttachment };

			PipelineSpecification pipelineSpecification{};
			pipelineSpecification.DebugName = "CubeMap Irradiance Convolution Pipeline";
			pipelineSpecification.pVertexShader = MasterRenderer::GetShaderLibrary().Get("EquirectangularToCubemapVertexShader");
			pipelineSpecification.pPixelShader = MasterRenderer::GetShaderLibrary().Get("PixelShaderIrradianceConvolution");
			pipelineSpecification.pFrameBuffer = FrameBuffer::Create(frameBufferSpecification);
			pipelineSpecification.MSAAEligible = false;
			pipelineSpecification.DepthWrite = false;
			pipelineSpecification.BackfaceCulling = false;

			RenderPassSpecification renderpassDescriptor{};
			renderpassDescriptor.DebugName = "CubeMap Irradiance Convolution Pass";
			renderpassDescriptor.RenderPipeline = Pipeline::Create(pipelineSpecification);

			m_pTextureCubeIrradianceConvolutionPass = RenderPass::Create(renderpassDescriptor);
		}

		//Create Radiance Map Pass:
		{
			ColorAttachment colorAttachment;
			colorAttachment.IsSRGB = false;
			colorAttachment.Format = TextureFormat::RGBA32F;
			colorAttachment.OperatorOnLoad = OperatorOnLoad::LoadOnly;
			colorAttachment.Blend = false;
			colorAttachment.ShouldResize = false;
			colorAttachment.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
			colorAttachment.Transfer = false;

			FrameBufferSpecification frameBufferSpecification{};
			frameBufferSpecification.DebugName = "CubeMap Radiance Convolution Framebuffer";
			frameBufferSpecification.ShouldResize = false;
			frameBufferSpecification.Width = 128u;
			frameBufferSpecification.Height = 128u;
			frameBufferSpecification.Attachments.ColorAttachments = { colorAttachment };

			PipelineSpecification pipelineSpecification{};
			pipelineSpecification.DebugName = "CubeMap Radiance Convolution Pipeline";
			pipelineSpecification.pVertexShader = MasterRenderer::GetShaderLibrary().Get("EquirectangularToCubemapVertexShader");
			pipelineSpecification.pPixelShader = MasterRenderer::GetShaderLibrary().Get("PixelShaderRadianceConvolution");
			pipelineSpecification.pFrameBuffer = FrameBuffer::Create(frameBufferSpecification);
			pipelineSpecification.MSAAEligible = false;
			pipelineSpecification.DepthWrite = false;
			pipelineSpecification.BackfaceCulling = false;

			RenderPassSpecification renderpassDescriptor{};
			renderpassDescriptor.DebugName = "CubeMap Radiance Convolution Pass";
			renderpassDescriptor.RenderPipeline = Pipeline::Create(pipelineSpecification);

			m_pTextureCubeRadianceConvolutionPass = RenderPass::Create(renderpassDescriptor);
		}
	}

	void UtilityRenderer::ConvertEquirectangularToCubeMap(const std::shared_ptr<Texture2D> pEquirectangularTexture, EquirectangularToCubeMapConversionCompleteCallback&& callback)
	{
		TextureCubeSpecification textureCubeSpecification;
		textureCubeSpecification.Width = 512u;
		textureCubeSpecification.Height = 512u;
		textureCubeSpecification.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		textureCubeSpecification.Name = pEquirectangularTexture->GetName() + " - TextureCube";
		textureCubeSpecification.MipCount = 1;
		textureCubeSpecification.SampleCount = 1;
		textureCubeSpecification.IsSRGB = false;

		D3D12_RESOURCE_DESC textureCubeDescriptor{};
		textureCubeDescriptor.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		textureCubeDescriptor.Alignment = 0;
		textureCubeDescriptor.Width = textureCubeSpecification.Width;
		textureCubeDescriptor.Height = textureCubeSpecification.Height;
		textureCubeDescriptor.DepthOrArraySize = 6;
		textureCubeDescriptor.MipLevels = textureCubeSpecification.MipCount;
		textureCubeDescriptor.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		textureCubeDescriptor.SampleDesc.Count = textureCubeSpecification.SampleCount;
		textureCubeDescriptor.SampleDesc.Quality = 0;
		textureCubeDescriptor.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		textureCubeDescriptor.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

		D3D12_HEAP_PROPERTIES heapProperties = {};
		heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProperties.CreationNodeMask = 1u;
		heapProperties.VisibleNodeMask = 1u;

		D3D12Core::GetDevice()->CreateCommittedResource(
			&heapProperties, 
			D3D12_HEAP_FLAG_NONE, 
			&textureCubeDescriptor, 
			D3D12_RESOURCE_STATE_RENDER_TARGET, nullptr, IID_PPV_ARGS(&textureCubeSpecification.pResource));

		//MemoryManager& memoryManager = Application::Get().GetMemorymanager();

		D3D12_RENDER_TARGET_VIEW_DESC rtvDescriptor{};
		rtvDescriptor.Format = textureCubeDescriptor.Format;
		rtvDescriptor.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
		rtvDescriptor.Texture2DArray.MipSlice = 0;
		rtvDescriptor.Texture2DArray.ArraySize = 1;

		for (uint32_t i = 0u; i < 6u; ++i)
		{
			rtvDescriptor.Texture2DArray.FirstArraySlice = i;
			//textureCubeSpecification.DescriptorHandleRTV[i] = memoryManager.CreateDescriptorHandle(DescriptorHandleType::RTV);
			DXCall_STD(D3D12Core::GetDevice()->CreateRenderTargetView(textureCubeSpecification.pResource.Get(), &rtvDescriptor, textureCubeSpecification.DescriptorHandleRTV[i].CPUHandle));
		}

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDescriptor{};
		srvDescriptor.Format = textureCubeDescriptor.Format;
		srvDescriptor.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		srvDescriptor.TextureCube.MostDetailedMip = 0;
		srvDescriptor.TextureCube.MipLevels = 1;
		srvDescriptor.TextureCube.ResourceMinLODClamp = 0.0f;
		srvDescriptor.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		//textureCubeSpecification.DescriptorHandleSRV = memoryManager.CreateDescriptorHandle(DescriptorHandleType::SRV);
		DXCall_STD(D3D12Core::GetDevice()->CreateShaderResourceView(textureCubeSpecification.pResource.Get(), &srvDescriptor, textureCubeSpecification.DescriptorHandleSRV.CPUHandle));
		
		auto& pPipeline = m_pEquirectangularToCubeMapPass->GetPipeline();

		//GPUTaskManager& gpuTaskManager = Application::Get().GetGPUTaskManager();
		//Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> pCommandList = gpuTaskManager.RequestCommandList(CommandType::Direct);

		//DXCall_STD(pCommandList->SetDescriptorHeaps(1u, Application::Get().GetMemorymanager().GetShaderBindableDescriptorHeap()->GetDescriptorHeapInterface().GetAddressOf()));
		//DXCall_STD(pCommandList->SetGraphicsRootSignature(pPipeline->GetRootSig().Get()));
		//DXCall_STD(pCommandList->SetPipelineState(pPipeline->GetInterface2().Get()));
		//DXCall_STD(pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST));

		D3D12_VIEWPORT viewport;
		viewport.Width = textureCubeSpecification.Width;
		viewport.Height = textureCubeSpecification.Height;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;
		//DXCall_STD(pCommandList->RSSetViewports(1u, &viewport));
		
		D3D12_RECT scissorRect;
		scissorRect.left = 0u;
		scissorRect.top = 0u;
		scissorRect.right = static_cast<LONG>(textureCubeSpecification.Width);
		scissorRect.bottom = static_cast<LONG>(textureCubeSpecification.Height);
		//DXCall_STD(pCommandList->RSSetScissorRects(1u, &scissorRect));

		m_EquirectangularToCubeMapPassData.TextureIndex = pEquirectangularTexture->GetSRVDescriptorHandle().Index;
		for (uint32_t i = 0u; i < 6; ++i)
		{
			m_EquirectangularToCubeMapPassData.ViewProjectionIndex = m_pTextureCubeCreationVPCBs[i]->GetCBVDescriptorIndex(0);
			//m_pEquirectangularToCubeMapPass->Upload("createTextureCubePassData", &m_EquirectangularToCubeMapPassData, pCommandList);
			//DXCall_STD(pCommandList->OMSetRenderTargets(1u, &textureCubeSpecification.DescriptorHandleRTV[i].CPUHandle, false, nullptr));
			//DXCall_STD(pCommandList->DrawInstanced(36, 1, 0u, 0u));
		}

		//gpuTaskManager.ScheduleCommandList(pCommandList, [Callback = std::move(callback), textureCubeSpecification]()
		//	{
		//		Callback(std::make_shared<TextureCube>(textureCubeSpecification));
		//	});
	}

	void UtilityRenderer::CreateIrradianceMap(const std::shared_ptr<TextureCube> pEnvironmentTextureCube, CreateIrradianceMapCompleteCallback&& callback)
	{
		TextureCubeSpecification textureCubeSpecification;
		textureCubeSpecification.Width = 32u;
		textureCubeSpecification.Height = 32u;
		textureCubeSpecification.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		textureCubeSpecification.Name = pEnvironmentTextureCube->GetName() + " - Irradiance Map";
		textureCubeSpecification.MipCount = 1;
		textureCubeSpecification.SampleCount = 1;
		textureCubeSpecification.IsSRGB = false;

		D3D12_RESOURCE_DESC textureCubeDescriptor{};
		textureCubeDescriptor.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		textureCubeDescriptor.Alignment = 0;
		textureCubeDescriptor.Width = textureCubeSpecification.Width;
		textureCubeDescriptor.Height = textureCubeSpecification.Height;
		textureCubeDescriptor.DepthOrArraySize = 6;
		textureCubeDescriptor.MipLevels = textureCubeSpecification.MipCount;
		textureCubeDescriptor.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		textureCubeDescriptor.SampleDesc.Count = textureCubeSpecification.SampleCount;
		textureCubeDescriptor.SampleDesc.Quality = 0;
		textureCubeDescriptor.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		textureCubeDescriptor.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

		D3D12_HEAP_PROPERTIES heapProperties = {};
		heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProperties.CreationNodeMask = 1u;
		heapProperties.VisibleNodeMask = 1u;

		D3D12Core::GetDevice()->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&textureCubeDescriptor,
			D3D12_RESOURCE_STATE_RENDER_TARGET, nullptr, IID_PPV_ARGS(&textureCubeSpecification.pResource));

		//MemoryManager& memoryManager = Application::Get().GetMemorymanager();

		D3D12_RENDER_TARGET_VIEW_DESC rtvDescriptor{};
		rtvDescriptor.Format = textureCubeDescriptor.Format;
		rtvDescriptor.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
		rtvDescriptor.Texture2DArray.MipSlice = 0;
		rtvDescriptor.Texture2DArray.ArraySize = 1;

		for (uint32_t i = 0u; i < 6u; ++i)
		{
			rtvDescriptor.Texture2DArray.FirstArraySlice = i;
			//textureCubeSpecification.DescriptorHandleRTV[i] = memoryManager.CreateDescriptorHandle(DescriptorHandleType::RTV);
			DXCall_STD(D3D12Core::GetDevice()->CreateRenderTargetView(textureCubeSpecification.pResource.Get(), &rtvDescriptor, textureCubeSpecification.DescriptorHandleRTV[i].CPUHandle));
		}

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDescriptor{};
		srvDescriptor.Format = textureCubeDescriptor.Format;
		srvDescriptor.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		srvDescriptor.TextureCube.MostDetailedMip = 0;
		srvDescriptor.TextureCube.MipLevels = 1;
		srvDescriptor.TextureCube.ResourceMinLODClamp = 0.0f;
		srvDescriptor.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		//textureCubeSpecification.DescriptorHandleSRV = memoryManager.CreateDescriptorHandle(DescriptorHandleType::SRV);
		DXCall_STD(D3D12Core::GetDevice()->CreateShaderResourceView(textureCubeSpecification.pResource.Get(), &srvDescriptor, textureCubeSpecification.DescriptorHandleSRV.CPUHandle));

		auto& pPipeline = m_pTextureCubeIrradianceConvolutionPass->GetPipeline();

		//GPUTaskManager& gpuTaskManager = Application::Get().GetGPUTaskManager();
		//Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> pCommandList = gpuTaskManager.RequestCommandList(CommandType::Direct);

		//DXCall_STD(pCommandList->SetDescriptorHeaps(1u, Application::Get().GetMemorymanager().GetShaderBindableDescriptorHeap()->GetDescriptorHeapInterface().GetAddressOf()));
		//DXCall_STD(pCommandList->SetGraphicsRootSignature(pPipeline->GetRootSig().Get()));
		//DXCall_STD(pCommandList->SetPipelineState(pPipeline->GetInterface2().Get()));
		//DXCall_STD(pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST));

		D3D12_VIEWPORT viewport;
		viewport.Width = textureCubeSpecification.Width;
		viewport.Height = textureCubeSpecification.Height;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;
		//DXCall_STD(pCommandList->RSSetViewports(1u, &viewport));

		D3D12_RECT scissorRect;
		scissorRect.left = 0u;
		scissorRect.top = 0u;
		scissorRect.right = static_cast<LONG>(textureCubeSpecification.Width);
		scissorRect.bottom = static_cast<LONG>(textureCubeSpecification.Height);
		//DXCall_STD(pCommandList->RSSetScissorRects(1u, &scissorRect));

		m_EquirectangularToCubeMapPassData.TextureIndex = pEnvironmentTextureCube->GetSRVDescriptorHandle().Index;
		for (uint32_t i = 0u; i < 6; ++i)
		{
			m_EquirectangularToCubeMapPassData.ViewProjectionIndex = m_pTextureCubeCreationVPCBs[i]->GetCBVDescriptorIndex(0);
			//m_pTextureCubeIrradianceConvolutionPass->Upload("createTextureCubePassData", &m_EquirectangularToCubeMapPassData, pCommandList);
			//DXCall_STD(pCommandList->OMSetRenderTargets(1u, &textureCubeSpecification.DescriptorHandleRTV[i].CPUHandle, false, nullptr));
			//DXCall_STD(pCommandList->DrawInstanced(36, 1, 0u, 0u));
		}

		//gpuTaskManager.ScheduleCommandList(pCommandList, [Callback = std::move(callback), textureCubeSpecification]()
		//	{
		//		Callback(std::make_shared<TextureCube>(textureCubeSpecification));
		//	});
	}

	void UtilityRenderer::CreateRadianceMap(const std::shared_ptr<TextureCube> pEnvironmentTextureCube, CreateRadianceMapCompleteCallback&& callback)
	{
		TextureCubeSpecification textureCubeSpecification;
		textureCubeSpecification.Width = 128u;
		textureCubeSpecification.Height = 128u;
		textureCubeSpecification.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		textureCubeSpecification.Name = pEnvironmentTextureCube->GetName() + " - Radiance Map";
		textureCubeSpecification.MipCount = 5;
		textureCubeSpecification.SampleCount = 1;
		textureCubeSpecification.IsSRGB = false;

		D3D12_RESOURCE_DESC textureCubeDescriptor{};
		textureCubeDescriptor.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		textureCubeDescriptor.Alignment = 0;
		textureCubeDescriptor.Width = textureCubeSpecification.Width;
		textureCubeDescriptor.Height = textureCubeSpecification.Height;
		textureCubeDescriptor.DepthOrArraySize = 6;
		textureCubeDescriptor.MipLevels = textureCubeSpecification.MipCount;
		textureCubeDescriptor.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		textureCubeDescriptor.SampleDesc.Count = textureCubeSpecification.SampleCount;
		textureCubeDescriptor.SampleDesc.Quality = 0;
		textureCubeDescriptor.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		textureCubeDescriptor.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

		D3D12_HEAP_PROPERTIES heapProperties = {};
		heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProperties.CreationNodeMask = 1u;
		heapProperties.VisibleNodeMask = 1u;

		D3D12Core::GetDevice()->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&textureCubeDescriptor,
			D3D12_RESOURCE_STATE_RENDER_TARGET, nullptr, IID_PPV_ARGS(&textureCubeSpecification.pResource));

		//MemoryManager& memoryManager = Application::Get().GetMemorymanager();

		D3D12_RENDER_TARGET_VIEW_DESC rtvDescriptor{};
		rtvDescriptor.Format = textureCubeDescriptor.Format;
		rtvDescriptor.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
		rtvDescriptor.Texture2DArray.ArraySize = 1;

		for (uint32_t mip = 0u; mip < textureCubeDescriptor.MipLevels; ++mip)
		{
			for (uint32_t i = 0u; i < 6u; ++i)
			{
				rtvDescriptor.Texture2DArray.MipSlice = mip;
				rtvDescriptor.Texture2DArray.FirstArraySlice = i;
				//textureCubeSpecification.DescriptorHandleRTVs.push_back(memoryManager.CreateDescriptorHandle(DescriptorHandleType::RTV));
				//textureCubeSpecification.DescriptorHandleRTV[i] = memoryManager.CreateDescriptorHandle(DescriptorHandleType::RTV);
				DXCall_STD(D3D12Core::GetDevice()->CreateRenderTargetView(textureCubeSpecification.pResource.Get(), &rtvDescriptor, textureCubeSpecification.DescriptorHandleRTVs[(mip * 6) + i].CPUHandle));
			}
		}

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDescriptor{};
		srvDescriptor.Format = textureCubeDescriptor.Format;
		srvDescriptor.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		srvDescriptor.TextureCube.MostDetailedMip = 0;
		srvDescriptor.TextureCube.MipLevels = textureCubeSpecification.MipCount;
		srvDescriptor.TextureCube.ResourceMinLODClamp = 0.0f;
		srvDescriptor.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		//textureCubeSpecification.DescriptorHandleSRV = memoryManager.CreateDescriptorHandle(DescriptorHandleType::SRV);
		DXCall_STD(D3D12Core::GetDevice()->CreateShaderResourceView(textureCubeSpecification.pResource.Get(), &srvDescriptor, textureCubeSpecification.DescriptorHandleSRV.CPUHandle));

		auto& pPipeline = m_pTextureCubeRadianceConvolutionPass->GetPipeline();

		//GPUTaskManager& gpuTaskManager = Application::Get().GetGPUTaskManager();
		//Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> pCommandList = gpuTaskManager.RequestCommandList(CommandType::Direct);

		//DXCall_STD(pCommandList->SetDescriptorHeaps(1u, Application::Get().GetMemorymanager().GetShaderBindableDescriptorHeap()->GetDescriptorHeapInterface().GetAddressOf()));
		//DXCall_STD(pCommandList->SetGraphicsRootSignature(pPipeline->GetRootSig().Get()));
		//DXCall_STD(pCommandList->SetPipelineState(pPipeline->GetInterface2().Get()));
		//DXCall_STD(pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST));

		for (uint32_t mip = 0u; mip < textureCubeSpecification.MipCount; ++mip)
		{
			const uint32_t mipWidth = 128u * std::pow(0.5f, mip);
			const uint32_t mipHeight = 128u * std::pow(0.5f, mip);

			D3D12_VIEWPORT viewport;
			viewport.Width = mipWidth;
			viewport.Height = mipHeight;
			viewport.MinDepth = 0.0f;
			viewport.MaxDepth = 1.0f;
			viewport.TopLeftX = 0.0f;
			viewport.TopLeftY = 0.0f;
			//DXCall_STD(pCommandList->RSSetViewports(1u, &viewport));

			D3D12_RECT scissorRect;
			scissorRect.left = 0u;
			scissorRect.top = 0u;
			scissorRect.right = static_cast<LONG>(mipWidth);
			scissorRect.bottom = static_cast<LONG>(mipHeight);
			//DXCall_STD(pCommandList->RSSetScissorRects(1u, &scissorRect));

			m_RadianceRoughnessData.Roughness = (float)mip / (float)(textureCubeSpecification.MipCount - 1);
			void* pData = nullptr;
			D3D12_RANGE readRange = D3D12_RANGE(0, 0);
			DXCall(m_pRadianceMapRougnessCBSets[mip]->At(0).GetInterface()->Map(0u, &readRange, &pData));
			RLS_ASSERT(pData, "Buffer write pointer is null");
			std::memcpy(pData, static_cast<const void*>(&m_RadianceRoughnessData), sizeof(RadianceRoughnessData));
			DXCall_STD(m_pRadianceMapRougnessCBSets[mip]->At(0).GetInterface()->Unmap(0, nullptr));

			m_EquirectangularToCubeMapPassData.TextureIndex = pEnvironmentTextureCube->GetSRVDescriptorHandle().Index;
			m_RadianceRoughnessIndexData.RoughnessIndex = m_pRadianceMapRougnessCBSets[mip]->GetCBVDescriptorIndex(0);
			
			//m_pTextureCubeRadianceConvolutionPass->Upload("mipRoughnessPassData", &m_RadianceRoughnessIndexData, pCommandList);
			for (uint32_t i = 0u; i < 6; ++i)
			{
				m_EquirectangularToCubeMapPassData.ViewProjectionIndex = m_pTextureCubeCreationVPCBs[i]->GetCBVDescriptorIndex(0);
				//m_pTextureCubeRadianceConvolutionPass->Upload("createTextureCubePassData", &m_EquirectangularToCubeMapPassData, pCommandList);

				//DXCall_STD(pCommandList->OMSetRenderTargets(1u, &textureCubeSpecification.DescriptorHandleRTV[i].CPUHandle, false, nullptr));
				//DXCall_STD(pCommandList->OMSetRenderTargets(1u, &textureCubeSpecification.DescriptorHandleRTVs[(mip * 6) + i].CPUHandle, false, nullptr));
				//DXCall_STD(pCommandList->DrawInstanced(36, 1, 0u, 0u));
			}
		}

		//gpuTaskManager.ScheduleCommandList(pCommandList, [Callback = std::move(callback), textureCubeSpecification]()
		//	{
		//		Callback(std::make_shared<TextureCube>(textureCubeSpecification));
		//	});
	}
}