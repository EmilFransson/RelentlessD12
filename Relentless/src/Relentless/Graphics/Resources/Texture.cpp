#include "Texture.h"
#include "../D3D12Core.h"
#include "..\MemoryManager.h"
#include "../../Utility.h"
namespace Relentless
{
	Texture::Texture(const TextureSpecification& textureSpecification, const std::string& name) noexcept
		 :IResource(name),
		  m_TextureSpecification{ textureSpecification },
		  m_SRVDescriptorHandle{}
	{}

	Texture::Texture(const std::string& name) noexcept
		: IResource(name),
		  m_TextureSpecification{},
		  m_SRVDescriptorHandle{}
	{}

	Texture::Texture() noexcept
		: m_TextureSpecification{},
		  m_SRVDescriptorHandle{}
	{}

	RenderTexture::RenderTexture(const uint32_t width, const uint32_t height, const std::string& name) noexcept
		: Texture(name),
		  m_RTVDescriptorHandle{}
	{
		m_TextureSpecification.Width = width;
		m_TextureSpecification.Height = height;
		m_TextureSpecification.MultiSampleCount = 1u;

		RLS_ASSERT(m_TextureSpecification.Width > 0 && m_TextureSpecification.Height > 0, "Texture dimension is not valid.");

		D3D12_HEAP_PROPERTIES heapProperties{};
		heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProperties.CreationNodeMask = 0u;
		heapProperties.VisibleNodeMask = 0u;

		D3D12_RESOURCE_DESC resourceDescriptor{};
		resourceDescriptor.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		resourceDescriptor.Alignment = 0u;
		resourceDescriptor.Width = m_TextureSpecification.Width;
		resourceDescriptor.Height = m_TextureSpecification.Height;
		resourceDescriptor.DepthOrArraySize = 1u;
		resourceDescriptor.MipLevels = 1u;
		resourceDescriptor.Format = m_TextureSpecification.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		resourceDescriptor.SampleDesc = { 1u, 0u };
		resourceDescriptor.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resourceDescriptor.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

		m_CurrentState = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

		D3D12_CLEAR_VALUE clearValue{};
		clearValue.Format = m_TextureSpecification.Format;
		DirectX::XMVECTOR convertedColor = DirectX::XMColorSRGBToRGB(DirectX::Colors::Black);
		clearValue.Color[0] = DirectX::XMVectorGetX(convertedColor);
		clearValue.Color[1] = DirectX::XMVectorGetY(convertedColor);
		clearValue.Color[2] = DirectX::XMVectorGetZ(convertedColor);
		clearValue.Color[3] = DirectX::XMVectorGetW(convertedColor);

		DXCall(D3D12Core::GetDevice()->CreateCommittedResource
		(
			&heapProperties,
			D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES,
			&resourceDescriptor,
			m_CurrentState,
			&clearValue,
			IID_PPV_ARGS(&m_pResource)
		));

		D3D12_RENDER_TARGET_VIEW_DESC renderTargetViewDescriptor = {};
		renderTargetViewDescriptor.Format = m_TextureSpecification.Format;
		renderTargetViewDescriptor.ViewDimension = m_TextureSpecification.MultiSampleCount > 1u ? D3D12_RTV_DIMENSION_TEXTURE2DMS : D3D12_RTV_DIMENSION_TEXTURE2D;

		m_RTVDescriptorHandle = MemoryManager::Get().CreateDescriptorHandle(DescriptorHandleType::RTV);
		DXCall_STD(D3D12Core::GetDevice()->CreateRenderTargetView(m_pResource.Get(), &renderTargetViewDescriptor, m_RTVDescriptorHandle.CPUHandle));
		
		D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDescriptor = {};
		shaderResourceViewDescriptor.Format = m_TextureSpecification.Format;
		shaderResourceViewDescriptor.ViewDimension = m_TextureSpecification.MultiSampleCount > 1u ? D3D12_SRV_DIMENSION_TEXTURE2DMS : D3D12_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDescriptor.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		m_SRVDescriptorHandle = MemoryManager::Get().CreateDescriptorHandle(DescriptorHandleType::SRV_NV);
		DXCall_STD(D3D12Core::GetDevice()->CreateShaderResourceView(m_pResource.Get(), &shaderResourceViewDescriptor, m_SRVDescriptorHandle.CPUHandle));
	
		NAME_D12_OBJECT(m_pResource, ConvertStringToWstring(m_Name).c_str());
		RLS_CORE_INFO("Created Render Texture '{0}' of size [width, height]=[{1},{2}]", m_Name, m_TextureSpecification.Width, m_TextureSpecification.Height);
	}

	RenderTexture::RenderTexture(const TextureSpecification& textureSpecification, const std::string& name) noexcept
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
		resourceDescriptor.Width = m_TextureSpecification.Width;
		resourceDescriptor.Height = m_TextureSpecification.Height;
		resourceDescriptor.DepthOrArraySize = 1u;
		resourceDescriptor.MipLevels = 1u;
		resourceDescriptor.Format = m_TextureSpecification.Format;
		resourceDescriptor.SampleDesc = { m_TextureSpecification.MultiSampleCount, 0u };
		resourceDescriptor.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resourceDescriptor.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

		m_CurrentState = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

		D3D12_CLEAR_VALUE clearValue{};
		clearValue.Format = m_TextureSpecification.Format;
		DirectX::XMVECTOR convertedColor = DirectX::XMColorSRGBToRGB(DirectX::XMLoadFloat4(&m_TextureSpecification.ClearColor));
		clearValue.Color[0] = DirectX::XMVectorGetX(convertedColor);
		clearValue.Color[1] = DirectX::XMVectorGetY(convertedColor);
		clearValue.Color[2] = DirectX::XMVectorGetZ(convertedColor);
		clearValue.Color[3] = DirectX::XMVectorGetW(convertedColor);

		DXCall(D3D12Core::GetDevice()->CreateCommittedResource
		(
			&heapProperties,
			D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES,
			&resourceDescriptor,
			m_CurrentState,
			&clearValue,
			IID_PPV_ARGS(&m_pResource)
		));

		if (m_TextureSpecification.CreateDescriptorHandles)
		{
			D3D12_RENDER_TARGET_VIEW_DESC renderTargetViewDescriptor = {};
			renderTargetViewDescriptor.Format = m_TextureSpecification.Format;
			renderTargetViewDescriptor.ViewDimension = m_TextureSpecification.MultiSampleCount > 1u ? D3D12_RTV_DIMENSION_TEXTURE2DMS : D3D12_RTV_DIMENSION_TEXTURE2D;

			m_RTVDescriptorHandle = MemoryManager::Get().CreateDescriptorHandle(DescriptorHandleType::RTV);
			DXCall_STD(D3D12Core::GetDevice()->CreateRenderTargetView(m_pResource.Get(), &renderTargetViewDescriptor, m_RTVDescriptorHandle.CPUHandle));
		
			D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDescriptor = {};
			shaderResourceViewDescriptor.Format = m_TextureSpecification.Format;
			shaderResourceViewDescriptor.ViewDimension = m_TextureSpecification.MultiSampleCount > 1u ? D3D12_SRV_DIMENSION_TEXTURE2DMS : D3D12_SRV_DIMENSION_TEXTURE2D;
			shaderResourceViewDescriptor.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

			m_SRVDescriptorHandle = MemoryManager::Get().CreateDescriptorHandle(DescriptorHandleType::SRV_NV);
			DXCall_STD(D3D12Core::GetDevice()->CreateShaderResourceView(m_pResource.Get(), &shaderResourceViewDescriptor, m_SRVDescriptorHandle.CPUHandle));
		}

		NAME_D12_OBJECT(m_pResource, ConvertStringToWstring(m_Name).c_str());
		RLS_CORE_INFO("Created Render Texture '{0}' of size [width, height]=[{1},{2}]", m_Name, m_TextureSpecification.Width, m_TextureSpecification.Height);
	}

	std::shared_ptr<RenderTexture> RenderTexture::Create(const uint32_t width, const uint32_t height, const std::string& name) noexcept
	{
		return std::make_shared<RenderTexture>(width, height, name);
	}

	std::shared_ptr<RenderTexture> RenderTexture::Create(const TextureSpecification& textureSpecification, const std::string& name) noexcept
	{
		return std::make_shared<RenderTexture>(textureSpecification, name);
	}
}