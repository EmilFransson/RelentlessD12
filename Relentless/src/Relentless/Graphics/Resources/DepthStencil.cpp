#include "DepthStencil.h"
#include "../D3D12Core.h"
#include "../MemoryManager.h"
#include "../../Utility.h"
namespace Relentless
{
	DepthStencil::DepthStencil(const uint32_t width, const uint32_t height, const uint8_t multiSampleCount, const std::string& name) noexcept
		:IResource{name},
		 m_Width{width}, 
		 m_Height{height}, 
		 m_Format{DXGI_FORMAT_D32_FLOAT},
		 m_MultiSampleCount{multiSampleCount}
	{
		RLS_ASSERT
		(
			m_Width > 0u && m_Width <= D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION 
			&& m_Height > 0u && m_Height <= D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION, 
			"Depth Stencil texture dimensions are invalid."
		);
		RLS_ASSERT(multiSampleCount > 0u, "Depth Stencil multisample count is invalid."); // & LESS THAN MAX...

		m_CurrentState = D3D12_RESOURCE_STATE_DEPTH_WRITE;

		D3D12_HEAP_PROPERTIES heapProperties = {};
		heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProperties.CreationNodeMask = 0u;
		heapProperties.VisibleNodeMask = 0u;

		D3D12_RESOURCE_DESC resourceDescriptor = {};
		resourceDescriptor.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		resourceDescriptor.Alignment = 0u; //Use 4mb for MSAA textures and 64kb for everything else.
		resourceDescriptor.Width = m_Width;
		resourceDescriptor.Height = m_Height;
		resourceDescriptor.DepthOrArraySize = 1u;
		resourceDescriptor.MipLevels = 1u;
		resourceDescriptor.Format = m_Format;
		resourceDescriptor.SampleDesc = { m_MultiSampleCount, 0 };
		resourceDescriptor.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resourceDescriptor.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		D3D12_CLEAR_VALUE optimizedClearValue = {};
		optimizedClearValue.Format = m_Format;
		optimizedClearValue.DepthStencil.Depth = 1.0f;
		optimizedClearValue.DepthStencil.Stencil = 0u;

		DXCall(D3D12Core::GetDevice()->CreateCommittedResource
		(
			&heapProperties, 
			D3D12_HEAP_FLAG_NONE,
			&resourceDescriptor,
			m_CurrentState,
			&optimizedClearValue,
			IID_PPV_ARGS(&m_pResource)
		));

		D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilViewDescriptor = {};
		depthStencilViewDescriptor.Format = m_Format;
		depthStencilViewDescriptor.ViewDimension = m_MultiSampleCount > 1 ? D3D12_DSV_DIMENSION_TEXTURE2DMS : D3D12_DSV_DIMENSION_TEXTURE2D;

		m_DSVDescriptorHandle = MemoryManager::Get().CreateDescriptorHandle(DescriptorHandleType::DSV);
		DXCall_STD(D3D12Core::GetDevice()->CreateDepthStencilView(m_pResource.Get(), &depthStencilViewDescriptor, m_DSVDescriptorHandle.CPUHandle));

		NAME_D12_OBJECT(m_pResource, ConvertStringToWstring(m_Name).c_str());
		RLS_CORE_INFO("Created Depth Stencil '{0}' of [width, height] = [{1},{2}]", m_Name, m_Width, m_Height);
	}

	std::shared_ptr<DepthStencil> DepthStencil::Create(const uint32_t width, const uint32_t height, const uint8_t multiSampleCount, const std::string& name) noexcept
	{
		return std::make_shared<DepthStencil>(width, height, multiSampleCount, name);
	}
}