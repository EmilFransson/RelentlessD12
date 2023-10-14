#include "UploadBuffer.h"
#include "../D3D12Core.h"
#include "../Renderer/MasterRenderer.h"
#include "../Renderer/RenderCommand.h"
namespace Relentless
{
	UploadBuffer::UploadBuffer(const uint64_t initialSizeInBytes, const std::string& name) noexcept
		: IResource{ name },
		  m_MappedPtr{ nullptr },
		  m_Capacity{ initialSizeInBytes },
		  m_CurrentSize{ 0u }
	{
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
			uploadHeapDescriptor.SizeInBytes = initialSizeInBytes;
			uploadHeapDescriptor.Properties = uploadHeapProperties;
			uploadHeapDescriptor.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
			uploadHeapDescriptor.Flags = D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES | D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES;
		}

		Microsoft::WRL::ComPtr<ID3D12Heap> pTempHeap{ nullptr };
		DXCall(D3D12Core::GetDevice()->CreateHeap(&uploadHeapDescriptor, IID_PPV_ARGS(&pTempHeap)));

		D3D12_RESOURCE_DESC uploadBufferDescriptor = {};
		{
			uploadBufferDescriptor.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			uploadBufferDescriptor.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
			uploadBufferDescriptor.Width = initialSizeInBytes;
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
			IID_PPV_ARGS(&m_pResource))
		);

		D3D12_RANGE nullRange = { 0,0 };
		DXCall(m_pResource->Map(0u, &nullRange, reinterpret_cast<void**>(&m_MappedPtr)));

		NAME_D12_OBJECT(m_pResource, ConvertStringToWstring(m_Name).c_str());
	}

	inline static std::mutex g_LoadMutex;

	void UploadBuffer::Upload() noexcept
	{
		//const std::lock_guard<std::mutex> lock(g_LoadMutex);

		uint64_t offset = 0u;
		for (uint32_t i{ 0u }; i < m_UploadQueue.size(); i++)
		{
			auto data = m_UploadQueue.front();
			RLS_ASSERT(data.pResource, "Destination resource to upload to is nullptr.");
			DXCall_STD(D3D12Core::GetCommandList()->CopyBufferRegion(data.pResource->GetInterface().Get(), 0u, m_pResource.Get(), offset, data.Size));

			D3D12_RESOURCE_BARRIER resourceTransitionBarrier{};
			resourceTransitionBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			resourceTransitionBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			resourceTransitionBarrier.Transition.pResource = data.pResource->GetInterface().Get();
			resourceTransitionBarrier.Transition.StateBefore = data.pResource->GetCurrentState();
			resourceTransitionBarrier.Transition.StateAfter = data.StateAfterCopy;
			resourceTransitionBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			DXCall_STD(D3D12Core::GetCommandList()->ResourceBarrier(1u, &resourceTransitionBarrier));
			data.pResource->SetCurrentState(data.StateAfterCopy);

			offset += data.Size;
			m_UploadQueue.pop();
			--i;
		}
		m_MappedPtr -= offset;
		m_CurrentSize = 0u;

		
		//MasterRenderer::ExecuteCommands();
		//MasterRenderer::WaitForGPU();
		//MasterRenderer::ResetFrameCommandUnits(MasterRenderer::GetCurrentFrameIndex());
	}

	void UploadBuffer::Copy(const uint64_t size, void* pSrc, IResource* pDst, const D3D12_RESOURCE_STATES stateAfterCopy) noexcept
	{
		const std::lock_guard<std::mutex> lock(g_LoadMutex);
		
		RLS_ASSERT(size > 0, "Size is not greater than 0.");
		RLS_ASSERT(pSrc, "Source adress is nullptr.");
		RLS_ASSERT(pDst, "Destination resource is nullptr.");
		RLS_ASSERT(!(m_CurrentSize + size > m_Capacity), "Upload buffer has reached its capacity."); // Being dynamic might be more logical

		std::memcpy(m_MappedPtr, pSrc, size);
		m_CurrentSize += size;
		m_MappedPtr += size;

		UploadData uploadData{};
		uploadData.Size = size;
		uploadData.pResource = pDst;
		uploadData.StateAfterCopy = stateAfterCopy;
		m_UploadQueue.push(uploadData);
	}
}