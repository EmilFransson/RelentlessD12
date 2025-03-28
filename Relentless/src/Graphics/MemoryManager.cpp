#include "MemoryManager.h"
#include "D3D12Core.h"
#include "../Core/Window.h"
#include "Assets/AssetManager.h"
namespace Relentless
{
	void MemoryManager::Initialize() noexcept
	{
		m_pRTVDescriptorHeap = std::move(std::make_unique<DescriptorHeap>(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 100'000, false));
		m_pDSVDescriptorHeap = std::move(std::make_unique<DescriptorHeap>(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 100, false));
		m_pShaderBindablesDescriptorHeapNV = std::move(std::make_unique<DescriptorHeap>(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 100'000, false));
		m_pShaderBindablesDescriptorHeap = std::move(std::make_unique<DescriptorHeap>(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 100'000, true));
		m_pDeferredFreeLists = std::move(std::unique_ptr<std::vector<DescriptorHandle>[]>(RLS_NEW std::vector<DescriptorHandle>[GPUTaskManager::FRAMES_IN_FLIGHT]));
		m_pDeferredFreeListsResources = std::move(std::unique_ptr<std::vector<std::shared_ptr<IResource>>[]>(RLS_NEW std::vector<std::shared_ptr<IResource>>[GPUTaskManager::FRAMES_IN_FLIGHT]));
		m_pUploadBuffer = std::move(std::make_unique<UploadBuffer>(700'000'000, "Main Upload Buffer"));
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
		{
			DescriptorHandle handle = m_pShaderBindablesDescriptorHeapNV->AllocateDescriptor();
			handle.Type = descriptorHandleType;
			return handle;
		}
		case DescriptorHandleType::CBV_NV:
		{
			DescriptorHandle handle = m_pShaderBindablesDescriptorHeapNV->AllocateDescriptor();
			handle.Type = descriptorHandleType;
			return handle;
		}
		case DescriptorHandleType::UAV_NV:
		{
			DescriptorHandle handle = m_pShaderBindablesDescriptorHeapNV->AllocateDescriptor();
			handle.Type = descriptorHandleType;
			return handle;
		}
		case DescriptorHandleType::SRV:
		{
			DescriptorHandle handle = m_pShaderBindablesDescriptorHeap->AllocateDescriptor();
			handle.Type = descriptorHandleType;
			return handle;
		}
		case DescriptorHandleType::CBV:
		{
			DescriptorHandle handle = m_pShaderBindablesDescriptorHeap->AllocateDescriptor();
			handle.Type = descriptorHandleType;
			return handle;
		}
		case DescriptorHandleType::UAV:
		{
			DescriptorHandle handle = m_pShaderBindablesDescriptorHeap->AllocateDescriptor();
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
		//const uint32_t frameIndex = Application::Get().GetGPUTaskManager().GetCurrentFrameIndex();

		//m_pDeferredFreeLists[frameIndex].emplace_back(descriptorHandle);
	}

	void MemoryManager::DestroyResource(std::shared_ptr<IResource> pResource) noexcept
	{
		//const uint32_t frameIndex = Application::Get().GetGPUTaskManager().GetCurrentFrameIndex();
		
		//m_pDeferredFreeListsResources[frameIndex].emplace_back(std::move(pResource));
	}

	void MemoryManager::PerformDeferredDeletion() noexcept
	{
		PROFILE_FUNC;

		//const uint32_t frameIndex = Application::Get().GetGPUTaskManager().GetCurrentFrameIndex();
		
		//if (!m_pDeferredFreeLists[frameIndex].empty())
		//{
		//	for (uint32_t i{ 0u }; i < m_pDeferredFreeLists[frameIndex].size(); ++i)
		//	{
		//		switch (m_pDeferredFreeLists[frameIndex][i].Type)
		//		{
		//		case DescriptorHandleType::RTV:
		//			m_pRTVDescriptorHeap->FreeDescriptor(m_pDeferredFreeLists[frameIndex][i]);
		//			break;
		//		case DescriptorHandleType::DSV:
		//			m_pDSVDescriptorHeap->FreeDescriptor(m_pDeferredFreeLists[frameIndex][i]);
		//			break;
		//		case DescriptorHandleType::CBV:
		//			m_pShaderBindablesDescriptorHeap->FreeDescriptor(m_pDeferredFreeLists[frameIndex][i]);
		//			break;
		//		case DescriptorHandleType::SRV:
		//			m_pShaderBindablesDescriptorHeap->FreeDescriptor(m_pDeferredFreeLists[frameIndex][i]);
		//			break;
		//		case DescriptorHandleType::UAV:
		//			m_pShaderBindablesDescriptorHeap->FreeDescriptor(m_pDeferredFreeLists[frameIndex][i]);
		//			break;
		//		case DescriptorHandleType::CBV_NV:
		//		case DescriptorHandleType::SRV_NV:
		//		case DescriptorHandleType::UAV_NV:
		//			m_pShaderBindablesDescriptorHeapNV->FreeDescriptor(m_pDeferredFreeLists[frameIndex][i]);
		//			break;
		//		}
		//	}
		//	m_pDeferredFreeLists[frameIndex].clear();
		//}
#if defined (RLS_DEBUG)
		//for (uint32_t i{ 0u }; i < m_pDeferredFreeListsResources[frameIndex].size(); i++)
		//	RLS_CORE_WARN("Destroyed resource '{0}'", m_pDeferredFreeListsResources[frameIndex][i]->GetName());
#endif
		//if (!m_pDeferredFreeListsResources[frameIndex].empty())
		//	m_pDeferredFreeListsResources[frameIndex].clear();
	}

	void MemoryManager::SetDirtyMaterial(const AssetHandle& handle) noexcept
	{
		if (m_DirtyMaterials.contains(handle.Uuid))
		{
			m_DirtyMaterials[handle.Uuid].second = GPUTaskManager::FRAMES_IN_FLIGHT;
		}
		else
		{
			m_DirtyMaterials[handle.Uuid] = {handle, GPUTaskManager::FRAMES_IN_FLIGHT};
		}
	}

	void MemoryManager::UpdateDirtyMaterials()
	{
		if (m_DirtyMaterials.empty())
			return;

		//ResourceManager& resourceManager = Application::Get().GetResourceManager();
		//const uint32_t frameIndex = Application::Get().GetGPUTaskManager().GetCurrentFrameIndex();
		for (auto& [UUID, handleUpdatesPair] : m_DirtyMaterials)
		{
			//std::shared_ptr<Material> material = AssetManager::Get<Material>(handleUpdatesPair.first);
			//
			//const ResourceHandle constantBufferHandle = material->m_ConstantBufferHandle;
			//resourceManager.UploadConstantBufferData(constantBufferHandle, &(*material), 112u, frameIndex);

			handleUpdatesPair.second--;
		}

		for (auto it = m_DirtyMaterials.begin(); it != m_DirtyMaterials.end();)
		{
			if (it->second.second <= 0) 
			{
				it = m_DirtyMaterials.erase(it); 
			}
			else 
			{
				++it; 
			}
		}
	}
}