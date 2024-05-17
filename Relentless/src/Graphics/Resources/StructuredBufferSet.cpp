#include "StructuredBufferSet.h"
#include "Graphics/D3D12Core.h"
#include "Graphics/GPUTaskManager.h"
#include "Graphics/MemoryManager.h"

namespace Relentless
{
	StructuredBuffer2::StructuredBuffer2(const std::string& name, uint32_t nrOfElements, uint32_t byteStride) noexcept
		: IResource{name}, m_NrOfElements{nrOfElements}, m_ByteStride{byteStride}
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

		m_SRVDescriptorHandle = MemoryManager::Get().CreateDescriptorHandle(DescriptorHandleType::SRV);
		DXCall_STD(D3D12Core::GetDevice()->CreateShaderResourceView(m_pResource.Get(), &shaderResourceViewDescriptor, m_SRVDescriptorHandle.CPUHandle));
		
		NAME_D12_OBJECT(m_pResource, ConvertStringToWstring(name).c_str());
	}
	
	const DescriptorHandle& StructuredBuffer2::GetSRVDescriptorHandle() const noexcept
	{
		return m_SRVDescriptorHandle;
	}

	size_t StructuredBuffer2::GetSizeInBytes() const noexcept
	{
		return m_SizeInBytes;
	}

	uint32_t StructuredBuffer2::GetByteStride() const noexcept
	{
		return m_ByteStride;
	}

	StructuredBufferSet::StructuredBufferSet(const std::string& name, uint32_t nrOfElements, uint32_t byteStride) noexcept
		:m_Name{name}
	{
		m_StructuredBuffers.reserve(GPUTaskManager::FRAMES_IN_FLIGHT);
		for (uint32_t i{ 0u }; i < GPUTaskManager::FRAMES_IN_FLIGHT; ++i)
		{
			const std::string structuredBufferName = name + " - Structured Buffer[" + std::to_string(i) + "]";
			m_StructuredBuffers.emplace_back(structuredBufferName, nrOfElements, byteStride);
		}
	}

	uint32_t StructuredBufferSet::GetSRVDescriptorIndex(uint32_t bufferIndex) const noexcept
	{
		return m_StructuredBuffers[bufferIndex].GetSRVDescriptorHandle().Index;
	}

	StructuredBuffer2& StructuredBufferSet::operator[](const uint32_t index) noexcept
	{
		RLS_ASSERT(index < GPUTaskManager::FRAMES_IN_FLIGHT, "[StructuredBufferSet]: index out of bounds");
		RLS_ASSERT(m_StructuredBuffers.size() > index, "[StructuredBufferSet]: index out of bounds");

		return m_StructuredBuffers[index];
	}

	const StructuredBuffer2& StructuredBufferSet::operator[](const uint32_t index) const noexcept
	{
		RLS_ASSERT(index < GPUTaskManager::FRAMES_IN_FLIGHT, "[StructuredBufferSet]: index out of bounds");
		RLS_ASSERT(m_StructuredBuffers.size() > index, "[StructuredBufferSet]: index out of bounds");
	
		return m_StructuredBuffers[index];
	}

	StructuredBuffer2& StructuredBufferSet::At(uint32_t bufferIndex) noexcept
	{
		return m_StructuredBuffers[bufferIndex];
	}

	const StructuredBuffer2& StructuredBufferSet::At(uint32_t bufferIndex) const noexcept
	{
		return m_StructuredBuffers[bufferIndex];
	}

}
