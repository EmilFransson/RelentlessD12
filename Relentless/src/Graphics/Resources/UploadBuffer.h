#pragma once
#include "IResource.h"
namespace Relentless
{
	class UploadBuffer : public IResource
	{
	public:

		struct UploadData
		{
			uint64_t Size{0u};
			IResource* pResource{nullptr};
			D3D12_RESOURCE_STATES StateAfterCopy;
		};

		UploadBuffer(const uint64_t initialSizeInBytes, const std::string& name = "?") noexcept;
		virtual ~UploadBuffer() noexcept override final = default;
		void Copy(const uint64_t size, void* pSrc, IResource* pDst, const D3D12_RESOURCE_STATES stateAfterCopy) noexcept;
		void Upload() noexcept;
		[[nodiscard]] constexpr uint64_t GetSize() const noexcept { return m_CurrentSize; };
	private:
		std::queue<UploadData> m_UploadQueue;
		unsigned char* m_MappedPtr;
		uint64_t m_Capacity;
		uint64_t m_CurrentSize;
	};
}