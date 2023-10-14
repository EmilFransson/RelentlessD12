#include "IndexBuffer.h"
#include "../../Mesh/Vertex.h"
#include "../MemoryManager.h"
#include "../D3D12Core.h"
namespace Relentless
{
	IndexBuffer::IndexBuffer(const Specification* specification) noexcept
		: IResource{ specification->Name },
		  m_Specification{ *specification }
	{
		D3D12_HEAP_PROPERTIES heapProperties = {};
		heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProperties.CreationNodeMask = 0u;
		heapProperties.VisibleNodeMask = 0u;

		D3D12_HEAP_DESC heapDescriptor = {};
		heapDescriptor.SizeInBytes = specification->TotalSizeInBytes;
		heapDescriptor.Properties = heapProperties;
		heapDescriptor.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		heapDescriptor.Flags = D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES | D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES;

		Microsoft::WRL::ComPtr<ID3D12Heap> pIBHeap{ nullptr };
		DXCall(D3D12Core::GetDevice()->CreateHeap(&heapDescriptor, IID_PPV_ARGS(&pIBHeap)));

		D3D12_RESOURCE_DESC resourceDescriptor = {};
		resourceDescriptor.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resourceDescriptor.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		resourceDescriptor.Width = specification->TotalSizeInBytes;
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

		auto& pUploadBuffer = MemoryManager::Get().GetUploadBuffer();
		pUploadBuffer->Copy(resourceDescriptor.Width, specification->pBuffer, this, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		NAME_D12_OBJECT(m_pResource, ConvertStringToWstring(m_Name).c_str());
	}
}