#include "IndexBuffer.h"
#include "../../Mesh/Vertex.h"
#include "../D3D12Core.h"
#include "Core/Application.h"
#include "Graphics/D3D12Debug.h"

namespace Relentless
{
	IndexBuffer::IndexBuffer(const std::string& name, uint32_t sizeInBytes, uint32_t indexCount) noexcept
		:IResource{ name },
		m_SizeInBytes{ sizeInBytes },
		m_IndexCount{ indexCount }
	{
		D3D12_HEAP_PROPERTIES heapProperties = {};
		heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProperties.CreationNodeMask = 0u;
		heapProperties.VisibleNodeMask = 0u;

		D3D12_HEAP_DESC heapDescriptor = {};
		heapDescriptor.SizeInBytes = sizeInBytes;
		heapDescriptor.Properties = heapProperties;
		heapDescriptor.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		heapDescriptor.Flags = D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES | D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES;

		Microsoft::WRL::ComPtr<ID3D12Heap> pIBHeap{ nullptr };
		DXCall(D3D12Core::GetDevice()->CreateHeap(&heapDescriptor, IID_PPV_ARGS(&pIBHeap)));

		D3D12_RESOURCE_DESC resourceDescriptor = {};
		resourceDescriptor.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resourceDescriptor.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		resourceDescriptor.Width = sizeInBytes;
		resourceDescriptor.Height = 1u;
		resourceDescriptor.DepthOrArraySize = 1u;
		resourceDescriptor.MipLevels = 1u;
		resourceDescriptor.Format = DXGI_FORMAT_UNKNOWN;
		resourceDescriptor.SampleDesc = { 1u, 0u };
		resourceDescriptor.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resourceDescriptor.Flags = D3D12_RESOURCE_FLAG_NONE;

		m_CurrentState = D3D12_RESOURCE_STATE_COPY_DEST;
		DXCall(D3D12Core::GetDevice()->CreatePlacedResource
		(
			pIBHeap.Get(),
			0u,
			&resourceDescriptor,
			m_CurrentState,
			nullptr,
			IID_PPV_ARGS(&m_pResource)
		));

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements = indexCount;
		srvDesc.Buffer.StructureByteStride = sizeInBytes / indexCount;
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		//m_SRVDescriptorHandle = Application::Get().GetMemorymanager().CreateDescriptorHandle(DescriptorHandleType::SRV);
		DXCall_STD(D3D12Core::GetDevice()->CreateShaderResourceView(m_pResource.Get(), &srvDesc, m_SRVDescriptorHandle.CPUHandle));

		NAME_D12_OBJECT(m_pResource, ConvertStringToWstring(name).c_str());
	}

	void* IndexBuffer::Map(uint32_t offset, uint32_t sizeInBytes) noexcept
	{
		RLS_ASSERT(sizeInBytes <= m_SizeInBytes, "[IndexBuffer]: Size to map exceeds buffer size.");
		void* pData = nullptr;

		D3D12_RANGE nullReadRange = { 0, 0 };
		DXCall(m_pResource->Map(0u, &nullReadRange, &pData));
		RLS_ASSERT(pData, "[IndexBuffer]: Unable to map index buffer pointer.");

		return static_cast<void*>(static_cast<char*>(pData) + offset);
	}

	void IndexBuffer::Unmap() noexcept
	{
		DXCall_STD(m_pResource->Unmap(0u, nullptr));
	}

}