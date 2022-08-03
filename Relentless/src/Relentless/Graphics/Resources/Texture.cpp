#include "Texture.h"
#include "../D3D12Core.h"
#include "..\MemoryManager.h"
namespace Relentless
{
	RenderTexture::RenderTexture(const uint32_t width, const uint32_t height) noexcept
		: m_Width{ width },
		  m_Height{ height }
	{
		RLS_ASSERT(m_Width > 0 && m_Height > 0, "Texture dimension is not valid.");
		
		D3D12_HEAP_PROPERTIES heapProperties{};
		heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProperties.CreationNodeMask = 0u;
		heapProperties.VisibleNodeMask = 0u;

		D3D12_RESOURCE_DESC resourceDescriptor{};
		resourceDescriptor.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		resourceDescriptor.Alignment = 0u;
		resourceDescriptor.Width = m_Width;
		resourceDescriptor.Height = m_Height;
		resourceDescriptor.DepthOrArraySize = 1u;
		resourceDescriptor.MipLevels = 1u;
		resourceDescriptor.Format = m_Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		resourceDescriptor.SampleDesc = { 1u, 0u };
		resourceDescriptor.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resourceDescriptor.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

		m_CurrentState = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON;

		D3D12_CLEAR_VALUE clearValue{};
		clearValue.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		clearValue.Color[0] = DirectX::Colors::Brown.f[0];
		clearValue.Color[1] = DirectX::Colors::Brown.f[1];
		clearValue.Color[2] = DirectX::Colors::Brown.f[2];
		clearValue.Color[3] = DirectX::Colors::Brown.f[3];

		DXCall(D3D12Core::GetDevice()->CreateCommittedResource
		(
			&heapProperties,
			D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES,
			&resourceDescriptor,
			m_CurrentState,
			&clearValue,
			IID_PPV_ARGS(&m_pResource)
		));

		m_RTVDescriptorHandle = MemoryManager::Get().CreateDescriptorHandle(DescriptorHandleType::RTV);
		DXCall_STD(D3D12Core::GetDevice()->CreateRenderTargetView(m_pResource.Get(), nullptr, m_RTVDescriptorHandle.CPUHandle));
		m_SRVDescriptorHandle = MemoryManager::Get().CreateDescriptorHandle(DescriptorHandleType::SRV_NV);
		DXCall_STD(D3D12Core::GetDevice()->CreateShaderResourceView(m_pResource.Get(), nullptr, m_SRVDescriptorHandle.CPUHandle));
	}

	std::shared_ptr<RenderTexture> RenderTexture::Create(const uint32_t width, const uint32_t height) noexcept
	{
		return std::make_shared<RenderTexture>(width, height);
	}

	RenderTextureMSAA::RenderTextureMSAA(const uint32_t width, const uint32_t height, const uint8_t multiSampleCount) noexcept
		: m_Width{ width },
		  m_Height{ height },
		  m_MultiSampleCount{ multiSampleCount }
	{
		RLS_ASSERT(m_Width > 0 && m_Height > 0, "Texture dimension is not valid.");
		RLS_ASSERT(m_MultiSampleCount > 1, "Texture multi sample count does not exceed 1. Did you mean to use a normal Render Texture?");

		D3D12_HEAP_PROPERTIES heapProperties{};
		heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProperties.CreationNodeMask = 0u;
		heapProperties.VisibleNodeMask = 0u;

		D3D12_RESOURCE_DESC resourceDescriptor{};
		resourceDescriptor.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		resourceDescriptor.Alignment = 0u;
		resourceDescriptor.Width = m_Width;
		resourceDescriptor.Height = m_Height;
		resourceDescriptor.DepthOrArraySize = 1u;
		resourceDescriptor.MipLevels = 1u;
		resourceDescriptor.Format = m_Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
		resourceDescriptor.SampleDesc = { m_MultiSampleCount, 0u };
		resourceDescriptor.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resourceDescriptor.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

		m_CurrentState = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_RESOLVE_SOURCE;

		D3D12_CLEAR_VALUE clearValue{};
		clearValue.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		clearValue.Color[0] = DirectX::Colors::Brown.f[0];
		clearValue.Color[1] = DirectX::Colors::Brown.f[1];
		clearValue.Color[2] = DirectX::Colors::Brown.f[2];
		clearValue.Color[3] = DirectX::Colors::Brown.f[3];

		DXCall(D3D12Core::GetDevice()->CreateCommittedResource
		(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&resourceDescriptor,
			m_CurrentState,
			&clearValue,
			IID_PPV_ARGS(&m_pResource)
		));

		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;

		m_RTVDescriptorHandle = MemoryManager::Get().CreateDescriptorHandle(DescriptorHandleType::RTV);
		DXCall_STD(D3D12Core::GetDevice()->CreateRenderTargetView(m_pResource.Get(), &rtvDesc, m_RTVDescriptorHandle.CPUHandle));
		
		m_SRVDescriptorHandle = MemoryManager::Get().CreateDescriptorHandle(DescriptorHandleType::SRV_NV);
		DXCall_STD(D3D12Core::GetDevice()->CreateShaderResourceView(m_pResource.Get(), nullptr, m_SRVDescriptorHandle.CPUHandle));

		NAME_D12_OBJECT(m_pResource, L"Main MSAA Texture");
	}

	std::shared_ptr<RenderTextureMSAA> RenderTextureMSAA::Create(const uint32_t width, const uint32_t height, const uint8_t multiSampleCount) noexcept
	{
		return std::make_shared<RenderTextureMSAA>(width, height, multiSampleCount);
	}
}