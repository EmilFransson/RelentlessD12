#include "MemoryManager.h"
#include "D3D12Core.h"
#include "../Window.h"
namespace Relentless
{
	MemoryManager MemoryManager::s_Instance;

	MemoryManager& MemoryManager::Get() noexcept
	{
		return s_Instance;
	}

	void MemoryManager::Initialize() noexcept
	{
		m_pRTVDescriptorHeap = std::move(std::make_unique<DescriptorHeap>(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 100'000, false));
		m_pDSVDescriptorHeap = std::move(std::make_unique<DescriptorHeap>(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 10, false));
		m_pShaderBindablesDescriptorHeapNV = std::move(std::make_unique<DescriptorHeap>(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 100'000, false));
		m_pDeferredFreeLists = std::move(std::unique_ptr<std::vector<DescriptorHandle>[]>(RLS_NEW std::vector<DescriptorHandle>[D3D12Core::GetNrOfBufferedFrames()]));
		m_pDeferredFreeListsResources = std::move(std::unique_ptr<std::vector<std::shared_ptr<IResource>>[]>(RLS_NEW std::vector<std::shared_ptr<IResource>>[D3D12Core::GetNrOfBufferedFrames()]));
	
		//Upload buffer:
		D3D12_HEAP_PROPERTIES uploadHeapProperties = {};
		{
			uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
			uploadHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			uploadHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			uploadHeapProperties.CreationNodeMask = 0u;
			uploadHeapProperties.VisibleNodeMask = 0u;
		}

		D3D12_HEAP_DESC uploadHeapDescriptor = {};
		{
			uploadHeapDescriptor.SizeInBytes = 100'000;
			uploadHeapDescriptor.Properties = uploadHeapProperties;
			uploadHeapDescriptor.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
			uploadHeapDescriptor.Flags = D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES | D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES;
		}

		Microsoft::WRL::ComPtr<ID3D12Heap> pTempHeap {nullptr};

		DXCall(D3D12Core::GetDevice()->CreateHeap(&uploadHeapDescriptor, IID_PPV_ARGS(&pTempHeap)));
		//HR(m_pUploadHeap->SetName(L"Main Upload Heap"));

		D3D12_RESOURCE_DESC uploadBufferDescriptor = {};
		{
			uploadBufferDescriptor.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			uploadBufferDescriptor.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
			uploadBufferDescriptor.Width = 100'000;
			uploadBufferDescriptor.Height = 1u;
			uploadBufferDescriptor.DepthOrArraySize = 1u;
			uploadBufferDescriptor.MipLevels = 1u;
			uploadBufferDescriptor.Format = DXGI_FORMAT_UNKNOWN;
			uploadBufferDescriptor.SampleDesc.Count = 1u;
			uploadBufferDescriptor.SampleDesc.Quality = 0u;
			uploadBufferDescriptor.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			uploadBufferDescriptor.Flags = D3D12_RESOURCE_FLAG_NONE;
		}

		DXCall(D3D12Core::GetDevice()->CreatePlacedResource
		(
			pTempHeap.Get(),
			0u,
			&uploadBufferDescriptor,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_pTempUploadBuffer))
		);
		DXCall(m_pTempUploadBuffer->SetName(L"Main Upload Buffer"));
	
	}

	const DescriptorHandle MemoryManager::CreateDescriptorHandle(DescriptorHandleType descriptorHandleType) noexcept
	{
		switch (descriptorHandleType)
		{
		case DescriptorHandleType::RTV:
		{
			DescriptorHandle handle = m_pRTVDescriptorHeap->AllocateDescriptor();
			handle.Type = descriptorHandleType;
			return handle;
		}
		case DescriptorHandleType::DSV:
		{
			DescriptorHandle handle = m_pDSVDescriptorHeap->AllocateDescriptor();
			handle.Type = descriptorHandleType;
			return handle;
		}
		case DescriptorHandleType::SRV_NV:
		case DescriptorHandleType::CBV_NV:
		case DescriptorHandleType::UAV_NV:
		{
			DescriptorHandle handle = m_pShaderBindablesDescriptorHeapNV->AllocateDescriptor();
			handle.Type = descriptorHandleType;
			return handle;
		}
		default:
			RLS_ASSERT(false, "Unknown descriptor handle type.");
			break;
		}
		return DescriptorHandle();
	}

	void MemoryManager::DestroyDescriptorHandle(const DescriptorHandle& descriptorHandle) noexcept
	{
		const uint32_t index = Window::GetCurrentBackbufferIndex() % D3D12Core::GetNrOfBufferedFrames();
		m_pDeferredFreeLists[index].emplace_back(descriptorHandle);
	}

	void MemoryManager::DestroyResource(std::shared_ptr<IResource> pResource) noexcept
	{
		const uint32_t index = Window::GetCurrentBackbufferIndex() % D3D12Core::GetNrOfBufferedFrames();
		m_pDeferredFreeListsResources[index].emplace_back(std::move(pResource));
	}

	void MemoryManager::PerformDeferredDeletion() noexcept
	{
		const uint32_t index = Window::GetCurrentBackbufferIndex() % D3D12Core::GetNrOfBufferedFrames();
		if (!m_pDeferredFreeLists[index].empty())
		{
			for (uint32_t i{ 0u }; i < m_pDeferredFreeLists[index].size(); ++i)
			{
				switch (m_pDeferredFreeLists[index][i].Type)
				{
				case DescriptorHandleType::RTV:
					m_pRTVDescriptorHeap->FreeDescriptor(m_pDeferredFreeLists[index][i]);
					break;
				case DescriptorHandleType::DSV:
					m_pDSVDescriptorHeap->FreeDescriptor(m_pDeferredFreeLists[index][i]);
					break;
				case DescriptorHandleType::CBV_NV:
				case DescriptorHandleType::SRV_NV:
				case DescriptorHandleType::UAV_NV:
					m_pShaderBindablesDescriptorHeapNV->FreeDescriptor(m_pDeferredFreeLists[index][i]);
					break;
				}
			}
			m_pDeferredFreeLists[index].clear();
		}
#if defined (RLS_DEBUG)
		for (uint32_t i{ 0u }; i < m_pDeferredFreeListsResources[index].size(); i++)
			RLS_CORE_WARN("Destroyed resource '{0}'", m_pDeferredFreeListsResources[index][i]->GetName());
#endif
		if (!m_pDeferredFreeListsResources[index].empty())
			m_pDeferredFreeListsResources[index].clear();
	}
}