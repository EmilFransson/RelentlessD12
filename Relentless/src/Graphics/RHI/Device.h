#pragma once
#include "CommandQueue.h"
#include "DeviceResource.h"
#include "Fence.h"
#include "Graphics/MemoryManager.h"
#include "Graphics/Shaders/ShaderLibrary.h"
#include "RHI.h"

namespace Relentless
{
	struct GraphicsDeviceOptions
	{
		bool UseDebugDevice = false;
		bool UseGPUValidation = false;
		bool UseWarp = false;
		bool UseStablePowerState = false;
	};

	class GraphicsDevice : public DeviceObject
	{
	public:
		static constexpr uint8 NUM_BUFFERS = 2u;

		GraphicsDevice(const GraphicsDeviceOptions& options) noexcept;
		virtual ~GraphicsDevice() noexcept override;

		[[nodiscard]] CommandContext* AllocateCommandContext(D3D12_COMMAND_LIST_TYPE type) noexcept;
		[[nodiscard]] Ref<Buffer> CreateBuffer(const BufferDesc& desc, const char* pName, const void* pInitData = (const void*)nullptr) noexcept;
		[[nodiscard]] Ref<TextureEx> CreateTexture(const TextureDesc& desc, const char* pName, std::span<D3D12_SUBRESOURCE_DATA> initData) noexcept;
		[[nodiscard]] Ref<DepthStencilView> CreateDSV(TextureEx* pTexture, const TextureDSVDesc& textureDSVDesc) noexcept;
		[[nodiscard]] Ref<PipelineState> CreatePipelineState(const PipelineStateInitializer& pipelineStateInitializer) noexcept;
		[[nodiscard]] Ref<ShaderResourceView> CreateSRV(TextureEx* pTexture, const TextureSRVDesc& srvDesc) noexcept;
		[[nodiscard]] Ref<RenderTargetView> CreateRTV(TextureEx* pTexture, const TextureRTVDesc& textureRTVDesc) noexcept;

		void DeferReleaseObject(ID3D12Object* pResource) noexcept;
		void FreeCommandContext(CommandContext* pCommandContext) noexcept;
		[[nodiscard]] CommandQueue* GetCommandQueue(D3D12_COMMAND_LIST_TYPE type) const noexcept;
		[[nodiscard]] ID3D12Device5* GetDevice() const noexcept;
		[[nodiscard]] Fence* GetFrameFence() const noexcept;
		[[nodiscard]] DescriptorHeap* GetGlobalShaderBindableHeap() const noexcept;
		[[nodiscard]] ShaderLibrary* GetShaderLibrary() const noexcept;

		void IdleGPU() noexcept;
		[[nodiscard]] DescriptorHandle RegisterGlobalDescriptor(DescriptorHandleType descriptorHandleType) noexcept;
		void TickFrame() noexcept;
		void UnregisterGlobalDescriptor(const DescriptorHandle& descriptorHandle) noexcept;
	private:
		Ref<IDXGIFactory6> m_pFactory = nullptr;
		Ref<ID3D12Device5> m_pDevice = nullptr;

		uint64 m_FrameIndex = 0u;

		std::array<uint64, NUM_BUFFERS> m_FrameFenceValues{};
		Ref<Fence> m_pFrameFence = nullptr;

		static constexpr uint8_t NUM_COMMAND_LIST_TYPES = D3D12_COMMAND_LIST_TYPE_VIDEO_DECODE;
		std::array<Ref<CommandQueue>, NUM_COMMAND_LIST_TYPES> m_CommandQueues;
		std::array<std::vector<Ref<CommandContext>>, NUM_COMMAND_LIST_TYPES> m_CommandContextPool;
		std::array<std::queue<CommandContext*>, NUM_COMMAND_LIST_TYPES> m_FreeCommandContexts;

		class DeferredDeleteQueue : DeviceObject
		{
		private:
			struct FencedObject
			{
				ID3D12Object* pResource = nullptr;
				SyncPoint Sync;
			};
		public:
			DeferredDeleteQueue(GraphicsDevice* pParent) noexcept;
			~DeferredDeleteQueue() noexcept;

			void Clean() noexcept;
			void EnqueueResource(ID3D12Object* pResource, const SyncPoint& syncPoint) noexcept;
		private:
			std::queue<FencedObject> m_DeletionQueue;
			std::mutex m_QueueMutex;
		};

		DeferredDeleteQueue m_DeferredDeleteQueue;

		MemoryManager m_MemoryManager;
		std::unique_ptr<ShaderLibrary> m_pShaderLibrary = nullptr;

		Ref<ScratchAllocationManager> m_pScratchAllocationManager = nullptr;
		Ref<RingBufferAllocator> m_pRingBufferAllocator = nullptr;

		std::mutex m_CommandContextAllocationMutex;
	};
}