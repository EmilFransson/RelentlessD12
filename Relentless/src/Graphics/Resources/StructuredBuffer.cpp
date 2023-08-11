#include "StructuredBuffer.h"
#include "..\D3D12Core.h"
#include "..\MemoryManager.h"

namespace Relentless
{
	StructuredBuffer::StructuredBuffer(uint32_t nrOfElements, size_t byteStride) noexcept
		: m_NrOfElements{ 0u }
	{
		D3D12_HEAP_PROPERTIES bufferHeapProperties = {};
		bufferHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
		bufferHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		bufferHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		bufferHeapProperties.CreationNodeMask = 0u;
		bufferHeapProperties.VisibleNodeMask = 0u;

		D3D12_RESOURCE_DESC bufferDescriptor = {};
		bufferDescriptor.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		bufferDescriptor.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		bufferDescriptor.Width = nrOfElements * byteStride;
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

		D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDescriptor{};
		shaderResourceViewDescriptor.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		shaderResourceViewDescriptor.Format = DXGI_FORMAT_UNKNOWN;
		shaderResourceViewDescriptor.Buffer.FirstElement = 0u;
		shaderResourceViewDescriptor.Buffer.NumElements = m_Capacity = nrOfElements;
		shaderResourceViewDescriptor.Buffer.StructureByteStride = m_ByteStride = static_cast<uint32_t>(byteStride);
		shaderResourceViewDescriptor.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		shaderResourceViewDescriptor.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		m_NonVisibleHandle = MemoryManager::Get().CreateDescriptorHandle(DescriptorHandleType::SRV_NV);
		DXCall_STD(D3D12Core::GetDevice()->CreateShaderResourceView(m_pResource.Get(), &shaderResourceViewDescriptor, m_NonVisibleHandle.CPUHandle));

		for (uint32_t i{ 0u }; i < D3D12Core::GetNrOfBufferedFrames(); ++i)
		{
			m_VisibleHandles[i] = MemoryManager::Get().CreateDescriptorHandle(DescriptorHandleType::SRV);
		}
	}

	StructuredBuffer::~StructuredBuffer() noexcept
	{
		MemoryManager::Get().DestroyDescriptorHandle(m_NonVisibleHandle);
		for (uint32_t i{ 0u }; i < D3D12Core::GetNrOfBufferedFrames(); ++i)
		{
			MemoryManager::Get().DestroyDescriptorHandle(m_VisibleHandles[i]);
		}
	}

	uint32_t StructuredBuffer::GetFreeIndex() const noexcept
	{
		RLS_ASSERT(m_NrOfElements != m_Capacity, "Structured buffer is at capacity.");
		return m_NrOfElements++;
	}
}
