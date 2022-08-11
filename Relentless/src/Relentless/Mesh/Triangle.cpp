#include "Triangle.h"
#include "../Graphics/D3D12Core.h"
#include "../Graphics/MemoryManager.h"
namespace Relentless
{
	Triangle::Triangle() noexcept
		: m_NrOfVertices{ 3u },
		  m_NrOfIndices{ 3u }
	{
		SimpleVertex vertices[3];
		vertices[0].Position = DirectX::XMFLOAT3(-0.5f, -0.5f, 0.0f);
		vertices[1].Position = DirectX::XMFLOAT3(0.0f, 0.5f, 0.0f);
		vertices[2].Position = DirectX::XMFLOAT3(0.5f, -0.5f, 0.0f);
		unsigned int indices[3] = { 0, 1, 2 };

		Microsoft::WRL::ComPtr<ID3D12Heap> pIBHeap{ nullptr };

		D3D12_HEAP_PROPERTIES heapProperties = {};
		heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProperties.CreationNodeMask = 0u;
		heapProperties.VisibleNodeMask = 0u;

		D3D12_HEAP_DESC heapDescriptor = {};
		heapDescriptor.SizeInBytes = sizeof(SimpleVertex) * 3u;
		heapDescriptor.Properties = heapProperties;
		heapDescriptor.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		heapDescriptor.Flags = D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES | D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES;

		Microsoft::WRL::ComPtr<ID3D12Heap> pVBHeap{ nullptr };
		DXCall(D3D12Core::GetDevice()->CreateHeap(&heapDescriptor, IID_PPV_ARGS(&pVBHeap)));

		D3D12_RESOURCE_DESC resourceDescriptor = {};
		resourceDescriptor.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resourceDescriptor.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		resourceDescriptor.Width = sizeof(SimpleVertex) * 3u;
		resourceDescriptor.Height = 1u;
		resourceDescriptor.DepthOrArraySize = 1u;
		resourceDescriptor.MipLevels = 1u;
		resourceDescriptor.Format = DXGI_FORMAT_UNKNOWN;
		resourceDescriptor.SampleDesc = { 1u, 0u };
		resourceDescriptor.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resourceDescriptor.Flags = D3D12_RESOURCE_FLAG_NONE;
		DXCall(D3D12Core::GetDevice()->CreatePlacedResource
		(
			pVBHeap.Get(),
			0u,
			&resourceDescriptor,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&m_pVertexBuffer)
		));
		NAME_D12_OBJECT(m_pVertexBuffer, L"Triangle Vertex Buffer");

		auto pUploadBuffer = MemoryManager::Get().GetUploadBuffer();



		D3D12_RANGE nullRange = { 0,0 };
		unsigned char* mappedPtr = nullptr;

		DXCall(pUploadBuffer->Map(0u, &nullRange, reinterpret_cast<void**>(&mappedPtr)));
		std::memcpy(mappedPtr, reinterpret_cast<unsigned char*>(vertices), resourceDescriptor.Width);
		DXCall_STD(D3D12Core::GetCommandList()->CopyBufferRegion(m_pVertexBuffer.Get(), 0u, pUploadBuffer.Get(), 0u, resourceDescriptor.Width));
		DXCall_STD(pUploadBuffer->Unmap(0u, nullptr));
		mappedPtr = nullptr;

		heapDescriptor.SizeInBytes = sizeof(unsigned int) * ARRAYSIZE(indices);
		DXCall(D3D12Core::GetDevice()->CreateHeap(&heapDescriptor, IID_PPV_ARGS(&pIBHeap)));

		resourceDescriptor.Width = sizeof(unsigned int) * ARRAYSIZE(indices);
		DXCall(D3D12Core::GetDevice()->CreatePlacedResource
		(
			pIBHeap.Get(),
			0u,
			&resourceDescriptor,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&m_pIndexBuffer)
		));
		NAME_D12_OBJECT(m_pIndexBuffer, L"Triangle Index Buffer");

		DXCall(pUploadBuffer->Map(0u, &nullRange, reinterpret_cast<void**>(&mappedPtr)));
		std::memcpy(static_cast<unsigned char*>(mappedPtr) + sizeof(vertices), reinterpret_cast<unsigned char*>(indices), resourceDescriptor.Width);
		DXCall_STD(D3D12Core::GetCommandList()->CopyBufferRegion(m_pIndexBuffer.Get(), 0u, pUploadBuffer.Get(), sizeof(vertices), resourceDescriptor.Width));
		DXCall_STD(pUploadBuffer->Unmap(0u, nullptr));
		mappedPtr = nullptr;

		D3D12_RESOURCE_BARRIER resourceTransitionBarrier{};
		resourceTransitionBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		resourceTransitionBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		resourceTransitionBarrier.Transition.pResource = m_pVertexBuffer.Get();
		resourceTransitionBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		resourceTransitionBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
		resourceTransitionBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		DXCall_STD(D3D12Core::GetCommandList()->ResourceBarrier(1u, &resourceTransitionBarrier));

		resourceTransitionBarrier.Transition.pResource = m_pIndexBuffer.Get();
		resourceTransitionBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		DXCall_STD(D3D12Core::GetCommandList()->ResourceBarrier(1u, &resourceTransitionBarrier));
	}
}