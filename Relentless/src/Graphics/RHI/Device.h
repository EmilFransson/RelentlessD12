#pragma once
#include "CommandQueue.h"
#include "DescriptorManager.h"
#include "DeviceResource.h"
#include "Fence.h"
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

		[[nodiscard]] CommandContext* AllocateCommandContext(D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT) noexcept;
		[[nodiscard]] Ref<BufferEx> CreateBuffer(const BufferDesc& desc, const char* pName, const void* pInitData = (const void*)nullptr) noexcept;
		[[nodiscard]] Ref<DepthStencilView> CreateDSV(TextureEx* pTexture, const TextureDSVDesc& textureDSVDesc) noexcept;            
		[[nodiscard]] Ref<TextureEx> CreateTexture(const TextureDesc& desc, const char* pName, Span<D3D12_SUBRESOURCE_DATA> initData = {}) noexcept;
		[[nodiscard]] Ref<TextureEx> CreateTextureForSwapchain(ID3D12ResourceX* pResource, uint32 index) noexcept;
		[[nodiscard]] Ref<PipelineState> CreatePipeline(const PipelineStateInitializer& pipelineStateInitializer) noexcept;
		[[nodiscard]] Ref<PipelineState> CreateComputePipeline(RootSignature* pRootSignature, const char* pShaderName, const char* pEntryPoint) noexcept;
		[[nodiscard]] Ref<RenderTargetView> CreateRTV(TextureEx* pTexture, const TextureRTVDesc& textureRTVDesc) noexcept;
		[[nodiscard]] Ref<ShaderResourceView> CreateSRV(BufferEx* pBuffer, const BufferSRVDesc& desc) noexcept;
		[[nodiscard]] Ref<ShaderResourceView> CreateSRV(TextureEx* pTexture, const TextureSRVDesc& srvDesc) noexcept;
		[[nodiscard]] Ref<UnorderedAccessView> CreateUAV(TextureEx* pTexture, const TextureUAVDesc& desc) noexcept;

		void DeferReleaseObject(ID3D12Object* pResource) noexcept;
		void FreeCommandContext(CommandContext* pCommandContext) noexcept;
		[[nodiscard]] CommandQueue* GetCommandQueue(D3D12_COMMAND_LIST_TYPE type) const noexcept;
		[[nodiscard]] ID3D12DeviceX* GetDevice() const noexcept;
		[[nodiscard]] IDXGIFactoryX* GetFactory() const noexcept;
		[[nodiscard]] Fence* GetFrameFence() const noexcept;
		[[nodiscard]] DescriptorHeapEx* GetGlobalShaderBindableHeap() const noexcept;
		[[nodiscard]] DescriptorHeapEx* GetGlobalSamplerHeap() const noexcept;
		[[nodiscard]] RootSignature* GetGlobalRootSignature() const noexcept;
		[[nodiscard]] DescriptorHeapEx* GetRenderTargetViewDescriptorHeap() const noexcept;
		[[nodiscard]] RingBufferAllocator* GetRingBuffer() const noexcept;
		[[nodiscard]] ShaderLibrary* GetShaderLibrary() const noexcept;

		void IdleGPU() noexcept;
		[[nodiscard]] DescriptorHandleEx RegisterGlobalDescriptor(DescriptorHandleTypeEx descriptorHandleType) noexcept;
		[[nodiscard]] std::vector<DescriptorHandleEx> RegisterGlobalDescriptorBlock(DescriptorHandleTypeEx descriptorHandleType, uint32 blockSize) noexcept;

		void TickFrame() noexcept;
		void UnregisterGlobalDescriptor(const DescriptorHandleEx& descriptorHandle) noexcept;
	private:
		Ref<IDXGIFactoryX> m_pFactory = nullptr;
		Ref<ID3D12DeviceX> m_pDevice = nullptr;
		Ref<IDXGIAdapter4> m_pAdapter = nullptr;

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

		std::unique_ptr<DescriptorManager> m_pDescriptorManager = nullptr;
		std::unique_ptr<ShaderLibrary> m_pShaderLibrary = nullptr;

		Ref<ScratchAllocationManager> m_pScratchAllocationManager = nullptr;
		Ref<RingBufferAllocator> m_pRingBufferAllocator = nullptr;

		std::mutex m_CommandContextAllocationMutex;

		Ref<RootSignature> m_pGlobalRootSignature = nullptr;
	};
}