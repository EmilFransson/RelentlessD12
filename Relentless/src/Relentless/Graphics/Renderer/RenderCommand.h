#pragma once
namespace Relentless
{
	class IResource;
	class Texture;
	class RenderTexture;
	class DepthStencil;
	struct BackBuffer;
	class RenderCommand
	{
	public:
		static void ResetFrameCommandUnits(const uint32_t frameIndex) noexcept;
		static void TransitionResource(const std::shared_ptr<IResource>& pResource, const D3D12_RESOURCE_STATES newState) noexcept;
		static void TransitionResource(const Microsoft::WRL::ComPtr<ID3D12Resource>& pResource, const D3D12_RESOURCE_STATES currentState, const D3D12_RESOURCE_STATES newState) noexcept;
		static void ResolveMSAA(const std::shared_ptr<Texture>& pSource, const std::shared_ptr<Texture>& pDestination) noexcept;
		static void ClearRenderTarget(const D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptorHandle, const DirectX::XMVECTORF32& clearColor) noexcept;
		static void ClearDepthStencil(const std::shared_ptr<DepthStencil>& pDepthStencil) noexcept;
		static void SetRenderTarget(const std::shared_ptr<RenderTexture>& pRenderTarget, const std::shared_ptr<DepthStencil>& pDepthStencilView = nullptr) noexcept;
		static void SetRenderTarget(const BackBuffer& backBuffer) noexcept;
		static void SetViewport(const D3D12_VIEWPORT& viewport) noexcept;
		static void SetScissorRect(const RECT& scissorRect) noexcept;
		static void SetTopology(const D3D12_PRIMITIVE_TOPOLOGY topology) noexcept;
		static void SetPipelineState(const Microsoft::WRL::ComPtr<ID3D12PipelineState>& pPipelineState) noexcept;
		static void SetRootSignature(const Microsoft::WRL::ComPtr<ID3D12RootSignature>& pRootSignature) noexcept;
	private:
		RenderCommand() noexcept = delete;
		~RenderCommand() noexcept = default;
	};
}