#pragma once
#include "Core/DLLExport.h"
#include "CommandQueue.h"

#include "DescriptorManager.h"
#include "DeviceResource.h"

#include "Fence.h"

#include "Graphics/Shaders/ShaderLibrary.h"

#include "RHI.h"

#include <directxtex/DirectXTex.h>

namespace Relentless
{
	struct GraphicsDeviceOptions
	{
		bool UseDebugDevice = false;
		bool UseGPUValidation = false;
		bool UseWarp = false;
		bool UseStablePowerState = false;
	};

	class RLS_API GraphicsDevice : public DeviceObject
	{
	public:
		static constexpr uint8 NUM_BUFFERS = 2u;

		GraphicsDevice(const GraphicsDeviceOptions& options) noexcept;
		virtual ~GraphicsDevice() noexcept override;

		NO_DISCARD CommandContext* AllocateCommandContext(D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT) noexcept;
		NO_DISCARD Ref<Buffer> CreateBuffer(const BufferDesc& desc, const char* pName, const void* pInitData = (const void*)nullptr) noexcept;
		NO_DISCARD Ref<DepthStencilView> CreateDSV(Texture* pTexture, const TextureDSVDesc& textureDSVDesc) noexcept;            
		NO_DISCARD Ref<Texture> CreateTexture(const TextureDesc& desc, const char* pName, Span<D3D12_SUBRESOURCE_DATA> initData = {}) noexcept;
		NO_DISCARD Ref<Texture> CreateTexture(const TextureDesc& desc, const char* pName, const DirectX::ScratchImage& aImage) noexcept;
		NO_DISCARD Ref<Texture> CreateTextureForSwapchain(ID3D12ResourceX* pResource, uint32 index) noexcept;
		NO_DISCARD Ref<PipelineState> CreatePipeline(const PipelineStateInitializer& pipelineStateInitializer) noexcept;
		NO_DISCARD Ref<PipelineState> CreateComputePipeline(RootSignature* pRootSignature, const char* pShaderName, const char* pEntryPoint) noexcept;
		NO_DISCARD Ref<RenderTargetView> CreateRTV(Texture* pTexture, const TextureRTVDesc& textureRTVDesc) noexcept;
		NO_DISCARD Ref<ShaderResourceView> CreateSRV(Buffer* pBuffer, const BufferSRVDesc& desc) noexcept;
		NO_DISCARD Ref<ShaderResourceView> CreateSRV(Texture* pTexture, const TextureSRVDesc& srvDesc) noexcept;
		NO_DISCARD Ref<UnorderedAccessView> CreateUAV(Texture* pTexture, const TextureUAVDesc& desc) noexcept;
		NO_DISCARD Ref<UnorderedAccessView> CreateUAV(Buffer* pBuffer, const BufferUAVDesc& desc) noexcept;

		void DeferReleaseObject(ID3D12Object* pResource) noexcept;
		void FreeCommandContext(CommandContext* pCommandContext) noexcept;
		NO_DISCARD CommandQueue* GetCommandQueue(D3D12_COMMAND_LIST_TYPE type) const noexcept;
		NO_DISCARD ID3D12DeviceX* GetDevice() const noexcept;
		NO_DISCARD IDXGIFactoryX* GetFactory() const noexcept;
		NO_DISCARD Fence* GetFrameFence() const noexcept;
		NO_DISCARD DescriptorHeap* GetGlobalShaderBindableHeap() const noexcept;
		NO_DISCARD DescriptorHeap* GetGlobalSamplerHeap() const noexcept;
		NO_DISCARD RootSignature* GetGlobalRootSignature() const noexcept;
		NO_DISCARD DescriptorHeap* GetRenderTargetViewDescriptorHeap() const noexcept;
		NO_DISCARD RingBufferAllocator* GetRingBuffer() const noexcept;
		NO_DISCARD ShaderLibrary* GetShaderLibrary() const noexcept;

		void IdleGPU() noexcept;
		NO_DISCARD DescriptorHandle RegisterGlobalDescriptor(DescriptorHandleType descriptorHandleType) noexcept;
		NO_DISCARD std::vector<DescriptorHandle> RegisterGlobalDescriptorBlock(DescriptorHandleType descriptorHandleType, uint32 blockSize) noexcept;

		void TickFrame() noexcept;
		void UnregisterGlobalDescriptor(const DescriptorHandle& descriptorHandle) noexcept;
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