#pragma once
#include "DeviceResource.h"
#include "CommandQueue.h"

namespace Relentless
{
	class SyncPoint;
	class CommandContext;

	struct RingBufferAllocation
	{
		CommandContext* pContext = nullptr;
		Ref<Buffer> pBackingResource = nullptr;
		D3D12_GPU_VIRTUAL_ADDRESS GpuHandle{ 0 };
		uint32 Offset = 0;
		uint32 Size = 0;
		void* pMappedMemory = nullptr;
	};

	class RingBufferAllocator : public DeviceObject
	{
	public:
		RingBufferAllocator(GraphicsDevice* pDevice, uint32 size) noexcept;
		virtual ~RingBufferAllocator() noexcept override;

		bool Allocate(uint32 size, RingBufferAllocation& allocation) noexcept;
		void Free(RingBufferAllocation& allocation) noexcept;
		void Sync() noexcept;

	private:
		struct RetiredAllocation
		{
			SyncPoint Sync;
			uint32 Offset = 0u;
			uint32 Size = 0u;
		};
		std::queue<RetiredAllocation> m_RetiredAllocations;

		CommandQueue* m_pQueue = nullptr;
		std::mutex m_Lock;
		uint32 m_Size = 0u;
		uint32 m_ConsumeOffset = 0u;
		uint32 m_ProduceOffset = 0u;

		SyncPoint m_LastSync;
		Ref<Buffer> m_pBuffer = nullptr;
	};
}
