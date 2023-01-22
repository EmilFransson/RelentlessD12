#include "ConstantBuffer.h"
#include "../D3D12Core.h"
namespace Relentless
{
	ConstantBuffer::ConstantBuffer(size_t sizeInBytes) noexcept
		: m_SizeInBytes{sizeInBytes}
	{
		D3D12_HEAP_PROPERTIES bufferHeapProperties = {};
		bufferHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
		bufferHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		bufferHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		bufferHeapProperties.CreationNodeMask = 0u;
		bufferHeapProperties.VisibleNodeMask = 0u;

		sizeInBytes = (sizeInBytes + 255) & ~255;

		D3D12_RESOURCE_DESC bufferDescriptor = {};
		bufferDescriptor.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		bufferDescriptor.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		bufferDescriptor.Width = sizeInBytes;
		bufferDescriptor.Height = 1u;
		bufferDescriptor.DepthOrArraySize = 1u;
		bufferDescriptor.MipLevels = 1u;
		bufferDescriptor.Format = DXGI_FORMAT_UNKNOWN;
		bufferDescriptor.Flags = D3D12_RESOURCE_FLAG_NONE;
		bufferDescriptor.SampleDesc.Count = 1u;
		bufferDescriptor.SampleDesc.Quality = 0u;
		bufferDescriptor.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		DXCall(D3D12Core::GetDevice()->CreateCommittedResource(&bufferHeapProperties, D3D12_HEAP_FLAG_NONE,
			&bufferDescriptor, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_pResource)));

		D3D12_CONSTANT_BUFFER_VIEW_DESC constantBufferDescriptor = {};
		constantBufferDescriptor.BufferLocation = m_pResource->GetGPUVirtualAddress();
		constantBufferDescriptor.SizeInBytes = static_cast<UINT>(sizeInBytes);

		m_NonVisibleHandle = MemoryManager::Get().CreateDescriptorHandle(DescriptorHandleType::CBV_NV);
		DXCall_STD(D3D12Core::GetDevice()->CreateConstantBufferView(&constantBufferDescriptor, m_NonVisibleHandle.CPUHandle));
	
		for (uint32_t i{ 0u }; i < D3D12Core::GetNrOfBufferedFrames(); ++i)
		{
			m_VisibleHandles[i] = MemoryManager::Get().CreateDescriptorHandle(DescriptorHandleType::CBV);
		}
	}
}