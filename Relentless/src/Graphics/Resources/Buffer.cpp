#include "Buffer.h"
#include "../D3D12Core.h"
#include "Graphics/D3D12Debug.h"

namespace Relentless
{
	Buffer::Buffer(const uint32_t size, const std::string& name) noexcept
		: IResource{name},
		  m_SizeInBytes{size}
	{
	}

	ReadBackBuffer::ReadBackBuffer(const uint32_t sizeInBytes, const std::string& name) noexcept
		: Buffer{sizeInBytes, name}
	{
		D3D12_HEAP_PROPERTIES heapProperties{};
		heapProperties.Type = D3D12_HEAP_TYPE_READBACK;
		heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProperties.CreationNodeMask = 0u;
		heapProperties.VisibleNodeMask = 0u;

		D3D12_RESOURCE_DESC resourceDescriptor{};
		resourceDescriptor.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resourceDescriptor.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		resourceDescriptor.Width = sizeInBytes;
		resourceDescriptor.Height = 1;
		resourceDescriptor.DepthOrArraySize = 1u;
		resourceDescriptor.MipLevels = 1u;
		resourceDescriptor.Format = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
		resourceDescriptor.SampleDesc = { 1, 0u };
		resourceDescriptor.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resourceDescriptor.Flags = D3D12_RESOURCE_FLAG_NONE;

		m_CurrentState = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_DEST;

		DXCall(D3D12Core::GetDevice()->CreateCommittedResource
		(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&resourceDescriptor,
			m_CurrentState,
			nullptr,
			IID_PPV_ARGS(&m_pResource)
		));

		NAME_D12_OBJECT(m_pResource, std::wstring(name.begin(), name.end()).c_str());
		RLS_CORE_INFO("Created Readback Buffer '{0}' of size {1}", m_Name, sizeInBytes);
	}

	std::shared_ptr<ReadBackBuffer> ReadBackBuffer::Create(const uint32_t sizeInBytes, const std::string& name) noexcept
	{
		RLS_ASSERT(sizeInBytes > 0u, "Size of readback buffer must be greater than 0.");
		return std::make_shared<ReadBackBuffer>(sizeInBytes, name);
	}
}