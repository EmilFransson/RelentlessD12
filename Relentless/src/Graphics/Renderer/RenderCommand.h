#pragma once
namespace Relentless
{
	class IResource;
	class DescriptorHeap;
	class Texture;
	class RenderTexture;
	class ReadBackBuffer;
	class DepthStencil;
	struct BackBuffer;
	class RenderCommand
	{
	public:
		static void ResetFrameCommandUnits(const uint32_t frameIndex) noexcept;
		static void TransitionResource(const std::shared_ptr<IResource>& pResource, const D3D12_RESOURCE_STATES newState) noexcept;
		static void TransitionResource(IResource& resource, const D3D12_RESOURCE_STATES newState) noexcept;
		static void TransitionResource(const Microsoft::WRL::ComPtr<ID3D12Resource>& pResource, const D3D12_RESOURCE_STATES currentState, const D3D12_RESOURCE_STATES newState) noexcept;
		static void FlushResourceTransitionBatch(std::vector<D3D12_RESOURCE_BARRIER>& batchedResourceStateTransitions) noexcept;
		static void ResolveMSAA(const std::shared_ptr<RenderTexture>& pSource, const std::shared_ptr<RenderTexture>& pDestination) noexcept;
		static void ClearRenderTarget(const D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptorHandle, const DirectX::XMFLOAT4& clearColor) noexcept;
		static void ClearDepthStencil(const std::shared_ptr<DepthStencil>& pDepthStencil) noexcept;
		static void SetRenderTarget(const std::shared_ptr<RenderTexture>& pRenderTarget, const std::shared_ptr<DepthStencil>& pDepthStencilView = nullptr) noexcept;
		static void SetRenderTarget(const BackBuffer& backBuffer) noexcept;
		static void SetViewport(const D3D12_VIEWPORT& viewport) noexcept;
		static void SetScissorRect(const RECT& scissorRect) noexcept;
		static void SetTopology(const D3D12_PRIMITIVE_TOPOLOGY topology) noexcept;
		static void SetPipelineState(const Microsoft::WRL::ComPtr<ID3D12PipelineState>& pPipelineState) noexcept;
		static void SetRootSignature(const Microsoft::WRL::ComPtr<ID3D12RootSignature>& pRootSignature) noexcept;
		static void SetDescriptorHeap(const std::unique_ptr<DescriptorHeap>& pDescriptorHeap) noexcept;
		static void DrawInstanced(const uint32_t vertexCount, const uint32_t instanceCount = 1u) noexcept;
		static void CopyTextureToTexture(const std::shared_ptr<Texture>& pSrcTexture, const std::shared_ptr<Texture>& pDstTexture) noexcept;
		static void CopyTexelToBuffer(const std::shared_ptr<Texture>& pSrcTexture, const std::shared_ptr<ReadBackBuffer>& pDstTexture, uint32_t x, uint32_t y, uint32_t texelSize) noexcept;
	private:
		RenderCommand() noexcept = delete;
		~RenderCommand() noexcept = default;
	};
}