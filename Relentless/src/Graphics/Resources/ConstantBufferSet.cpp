#include "ConstantBufferSet.h"
#include "Graphics/D3D12Core.h"
#include "Graphics/GPUTaskManager.h"
#include "Graphics/MemoryManager.h"
#include "Core/Application.h"

namespace Relentless
{
	ConstantBuffer2::ConstantBuffer2(const std::string& name, uint32_t sizeInBytes) noexcept
		:IResource{name}, m_SizeInBytes{sizeInBytes}
	{
		RLS_ASSERT(sizeInBytes > 0, "[ConstantBuffer]: Invalid size provided to constant buffer '{0}'", name.c_str());

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

		m_CBVDescriptorHandle = Application::Get().GetMemorymanager().CreateDescriptorHandle(DescriptorHandleType::CBV);
		DXCall_STD(D3D12Core::GetDevice()->CreateConstantBufferView(&constantBufferDescriptor, m_CBVDescriptorHandle.CPUHandle));

		NAME_D12_OBJECT(m_pResource, ConvertStringToWstring(name).c_str());

		const D3D12_RANGE nullReadRange = D3D12_RANGE(0,0);
		DXCall(m_pResource->Map(0u, &nullReadRange, &m_pData));
		RLS_ASSERT(m_pData, "[ConstantBuffer]: Pointer to buffer starting memory location is invalid.");
	}

	const DescriptorHandle& ConstantBuffer2::GetCBVDescriptorHandle() const noexcept
	{
		return m_CBVDescriptorHandle;
	}

	size_t ConstantBuffer2::GetSizeInBytes() const noexcept
	{
		return m_SizeInBytes;
	}

	void ConstantBuffer2::UploadData(void* ptrToData, size_t sizeInBytes, size_t offset)
	{
		RLS_ASSERT(ptrToData, "[ConstantBuffer]: Data pointer is invalid.");
		RLS_ASSERT(sizeInBytes <= m_SizeInBytes, "[ConstantBuffer]: Byte size of data to upload exceeds buffer capacity.");

		std::memcpy(reinterpret_cast<uint8_t*>(m_pData) + offset, ptrToData, sizeInBytes);
	}

	ConstantBufferSet::ConstantBufferSet(const std::string& name, uint32_t sizeInBytes) noexcept
		: m_Name{name}, m_SizeInBytes{sizeInBytes}
	{
		RLS_ASSERT(sizeInBytes > 0, "[ConstantBufferSet]: Invalid size provided to constant buffer set '{0}'", name.c_str());

		m_ConstantBuffers.reserve(GPUTaskManager::FRAMES_IN_FLIGHT);
		for (uint32_t i = 0u; i < GPUTaskManager::FRAMES_IN_FLIGHT; ++i)
		{
			const std::string constantBufferName = name + " - ConstantBuffer[" + std::to_string(i) + "]";
			m_ConstantBuffers.emplace_back(constantBufferName, sizeInBytes);
		}
	}

	uint32_t ConstantBufferSet::GetCBVDescriptorIndex(uint32_t bufferIndex) const noexcept
	{
		RLS_ASSERT(bufferIndex < m_ConstantBuffers.size(), "[ConstantBufferSet]: Buffer index is out of range.");

		return m_ConstantBuffers[bufferIndex].GetCBVDescriptorHandle().Index;
	}

	const ConstantBuffer2& ConstantBufferSet::operator[](uint32_t index) const noexcept
	{
		RLS_ASSERT(index < GPUTaskManager::FRAMES_IN_FLIGHT, "[ConstantBufferSet]: index out of bounds");
		RLS_ASSERT(m_ConstantBuffers.size() > index, "[ConstantBufferSet]: index out of bounds");

		return m_ConstantBuffers[index];
	}

	ConstantBuffer2& ConstantBufferSet::operator[](uint32_t index) noexcept
	{
		RLS_ASSERT(index < GPUTaskManager::FRAMES_IN_FLIGHT, "[ConstantBufferSet]: index out of bounds");
		RLS_ASSERT(m_ConstantBuffers.size() > index, "[ConstantBufferSet]: index out of bounds");

		return m_ConstantBuffers[index];
	}

	ConstantBuffer2& ConstantBufferSet::At(uint32_t bufferIndex) noexcept
	{
		return m_ConstantBuffers[bufferIndex];
	}

	const ConstantBuffer2& ConstantBufferSet::At(uint32_t bufferIndex) const noexcept
	{
		return m_ConstantBuffers[bufferIndex];
	}

}