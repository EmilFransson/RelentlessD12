#include "DepthStencil.h"
#include "../D3D12Core.h"
#include "../MemoryManager.h"
#include "Core/Application.h"
#include "Core/Utility.h"

namespace Relentless
{
	DepthStencil::DepthStencil(const DepthStencilSpecification& depthStencilSpecification, const std::string& name) noexcept
		:IResource{name},
		 m_Specification{ depthStencilSpecification }
	{
		RLS_ASSERT
		(
			m_Specification.Width > 0u && m_Specification.Width <= D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION
			&& m_Specification.Height > 0u && m_Specification.Height <= D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION,
			"Depth Stencil texture dimensions are invalid."
		);
		RLS_ASSERT(m_Specification.Samples > 0u, "Depth Stencil multisample count is invalid."); // & LESS THAN MAX...

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
		resourceDescriptor.Width = m_Specification.Width;
		resourceDescriptor.Height = m_Specification.Height;
		resourceDescriptor.DepthOrArraySize = 1u;
		resourceDescriptor.MipLevels = 1u;
		resourceDescriptor.Format = m_Specification.Format;
		resourceDescriptor.SampleDesc = { m_Specification.Samples, 0 };
		resourceDescriptor.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resourceDescriptor.Flags = m_Specification.Flags;
		if (!m_Specification.CreateSRV)
			resourceDescriptor.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;

		D3D12_CLEAR_VALUE optimizedClearValue = {};
		optimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
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
		depthStencilViewDescriptor.Format = DXGI_FORMAT_D32_FLOAT;
		depthStencilViewDescriptor.ViewDimension = m_Specification.Samples > 1 ? D3D12_DSV_DIMENSION_TEXTURE2DMS : D3D12_DSV_DIMENSION_TEXTURE2D;

		MemoryManager& memorymanager = Application::Get().GetMemorymanager();

		m_DSVDescriptorHandle = memorymanager.CreateDescriptorHandle(DescriptorHandleType::DSV);
		DXCall_STD(D3D12Core::GetDevice()->CreateDepthStencilView(m_pResource.Get(), &depthStencilViewDescriptor, m_DSVDescriptorHandle.CPUHandle));

		if (m_Specification.CreateSRV)
		{
			// Create the shader resource view.
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = DXGI_FORMAT_R32_FLOAT;  // Interpret as R32_FLOAT for the SRV.
			srvDesc.ViewDimension = m_Specification.Samples > 1 ? D3D12_SRV_DIMENSION_TEXTURE2DMS : D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.MipLevels = 1;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			
			m_SRVDescriptorHandle = memorymanager.CreateDescriptorHandle(DescriptorHandleType::SRV);
			DXCall_STD(D3D12Core::GetDevice()->CreateShaderResourceView(m_pResource.Get(), &srvDesc, m_SRVDescriptorHandle.CPUHandle));
		}

		NAME_D12_OBJECT(m_pResource, ConvertStringToWstring(m_Name).c_str());
		RLS_CORE_INFO("Created Depth Stencil '{0}' of [width, height] = [{1},{2}]", m_Name, m_Specification.Width, m_Specification.Height);
	}

	std::shared_ptr<DepthStencil> DepthStencil::Create(const DepthStencilSpecification& depthStencilDescriptor, const std::string& name) noexcept
	{
		return std::make_shared<DepthStencil>(depthStencilDescriptor, name);
	}
}