#include "Texture.h"
#include "../D3D12Core.h"
#include "..\MemoryManager.h"
#include "../../Core/Utility.h"

#include "../../../vendor/includes/DirectXTK/WICTextureLoader.h"
#include "../../../vendor/includes/DirectXTK/ResourceUploadBatch.h"

namespace Relentless
{
	Texture::Texture(const RenderTextureSpecification& textureSpecification, const std::string& name) noexcept
		 :m_Width{textureSpecification.Width},
	      m_Height{ textureSpecification.Height},
		  m_Format{ textureSpecification.Format },
		   m_ClearColor{ textureSpecification.ClearColor },
		  m_MSAACount{ textureSpecification.MultiSampleCount },
		  IResource(name),
		  m_SRVDescriptorHandle{}
	{}

	RenderTexture::RenderTexture(const RenderTextureSpecification& textureSpecification, const std::string& name) noexcept
		: Texture(textureSpecification, name),
		  m_RTVDescriptorHandle{}
	{
		RLS_ASSERT(textureSpecification.Width > 0 && textureSpecification.Height > 0, "Texture dimension is not valid.");
		
		D3D12_HEAP_PROPERTIES heapProperties{};
		heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProperties.CreationNodeMask = 0u;
		heapProperties.VisibleNodeMask = 0u;

		D3D12_RESOURCE_DESC resourceDescriptor{};
		resourceDescriptor.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		resourceDescriptor.Alignment = 0u;
		resourceDescriptor.Width = textureSpecification.Width;
		resourceDescriptor.Height = textureSpecification.Height;
		resourceDescriptor.DepthOrArraySize = 1u;
		resourceDescriptor.MipLevels = 1u;
		resourceDescriptor.Format = textureSpecification.Format;
		resourceDescriptor.SampleDesc = { textureSpecification.MultiSampleCount, 0u };
		resourceDescriptor.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resourceDescriptor.Flags = textureSpecification.Flags;
	
		m_CurrentState = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

		D3D12_CLEAR_VALUE clearValue{};
		clearValue.Format = textureSpecification.Format;
		clearValue.Color[0] = textureSpecification.ClearColor.x;
		clearValue.Color[1] = textureSpecification.ClearColor.y;
		clearValue.Color[2] = textureSpecification.ClearColor.z;
		clearValue.Color[3] = textureSpecification.ClearColor.w;

		DXCall(D3D12Core::GetDevice()->CreateCommittedResource
		(
			&heapProperties,
			D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES,
			&resourceDescriptor,
			m_CurrentState,
			textureSpecification.Flags  == D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE ? nullptr : &clearValue,
			IID_PPV_ARGS(&m_pResource)
		));

		if ((textureSpecification.Flags & D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) != 0)
		{
			D3D12_RENDER_TARGET_VIEW_DESC renderTargetViewDescriptor = {};
			renderTargetViewDescriptor.Format = textureSpecification.Format;
			renderTargetViewDescriptor.ViewDimension = textureSpecification.MultiSampleCount > 1u ? D3D12_RTV_DIMENSION_TEXTURE2DMS : D3D12_RTV_DIMENSION_TEXTURE2D;

			m_RTVDescriptorHandle = MemoryManager::Get().CreateDescriptorHandle(DescriptorHandleType::RTV);
			DXCall_STD(D3D12Core::GetDevice()->CreateRenderTargetView(m_pResource.Get(), &renderTargetViewDescriptor, m_RTVDescriptorHandle.CPUHandle));
		}
	
		if (textureSpecification.CreateSRV)
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDescriptor ={};
			shaderResourceViewDescriptor.Format = textureSpecification.Format;
			shaderResourceViewDescriptor.ViewDimension = textureSpecification.MultiSampleCount > 1u ? D3D12_SRV_DIMENSION_TEXTURE2DMS : D3D12_SRV_DIMENSION_TEXTURE2D, shaderResourceViewDescriptor.Texture2D.MostDetailedMip = 0u, shaderResourceViewDescriptor.Texture2D.MipLevels = static_cast<UINT>(- 1);
			shaderResourceViewDescriptor.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

			m_SRVDescriptorHandle = MemoryManager::Get().CreateDescriptorHandle(DescriptorHandleType::SRV);
			DXCall_STD(D3D12Core::GetDevice()->CreateShaderResourceView(m_pResource.Get(), &shaderResourceViewDescriptor, m_SRVDescriptorHandle.CPUHandle));
		}

		NAME_D12_OBJECT(m_pResource, ConvertStringToWstring(m_Name).c_str());
		RLS_CORE_INFO("Created Render Texture '{0}' of size [width, height]=[{1},{2}]", m_Name, textureSpecification.Width, textureSpecification.Height);
	}

	std::shared_ptr<RenderTexture> RenderTexture::Create(RenderTextureSpecification& textureSpecification, const std::string& name) noexcept
	{
		if (textureSpecification.isSRGB)
		{
			DirectX::XMVECTOR convertedColor = DirectX::XMColorSRGBToRGB(DirectX::XMLoadFloat4(&textureSpecification.ClearColor));
			textureSpecification.ClearColor.x = DirectX::XMVectorGetX(convertedColor);
			textureSpecification.ClearColor.y = DirectX::XMVectorGetY(convertedColor);
			textureSpecification.ClearColor.z = DirectX::XMVectorGetZ(convertedColor);
			textureSpecification.ClearColor.w = DirectX::XMVectorGetW(convertedColor);
		}

		return std::make_shared<RenderTexture>(textureSpecification, name);
	}

	Texture2D::Texture2D(const std::string& fileName) noexcept
	{
		DirectX::ResourceUploadBatch resourceUpload(D3D12Core::GetDevice().Get());

		std::filesystem::path fullPath = fileName;
		m_Name = fullPath.stem().string();

		resourceUpload.Begin();


		DXCall(DirectX::CreateWICTextureFromFile(
			D3D12Core::GetDevice().Get(),
			resourceUpload, 
			fullPath.c_str(),
			&m_pResource, 
			true)
		);

		// Upload the resources to the GPU.
		auto uploadResourcesFinished = resourceUpload.End(D3D12Core::GetCommandQueue().Get());

		// Wait for the upload thread to terminate
		uploadResourcesFinished.wait();

		auto textureDescriptor = m_pResource->GetDesc();

		D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDescriptor = {};
		shaderResourceViewDescriptor.Format = textureDescriptor.Format;
		shaderResourceViewDescriptor.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDescriptor.Texture2D.MipLevels = textureDescriptor.MipLevels;
		shaderResourceViewDescriptor.Texture2D.MostDetailedMip = 0;
		shaderResourceViewDescriptor.Texture2D.ResourceMinLODClamp = 0.0f;
		shaderResourceViewDescriptor.Texture2D.PlaneSlice = 0u;
		shaderResourceViewDescriptor.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		m_SRVDescriptorHandle = MemoryManager::Get().CreateDescriptorHandle(DescriptorHandleType::SRV);
		DXCall_STD(D3D12Core::GetDevice()->CreateShaderResourceView(m_pResource.Get(), &shaderResourceViewDescriptor, m_SRVDescriptorHandle.CPUHandle));
		
		NAME_D12_OBJECT(m_pResource, ConvertStringToWstring(m_Name).c_str());
		RLS_CORE_INFO("Created Texture2D '{0}' of size [width, height]=[{1},{2}]", m_Name, textureDescriptor.Width, textureDescriptor.Height);
	}

	std::shared_ptr<Texture2D> Texture2D::Create(const std::string& fileName) noexcept
	{
		return std::make_shared<Texture2D>(fileName);
	}

	std::shared_ptr<Texture2D> Texture2D::CreateFromPath(const std::filesystem::path& filePath) noexcept
	{
		return std::make_shared<Texture2D>(filePath.string());
	}
}