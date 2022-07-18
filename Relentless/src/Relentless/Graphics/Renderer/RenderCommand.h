#pragma once
namespace Relentless
{
	class RenderCommand
	{
	public:
		static void TransitionResource(const Microsoft::WRL::ComPtr<ID3D12Resource> pResource, const D3D12_RESOURCE_STATES previousState, const D3D12_RESOURCE_STATES newState) noexcept;
		static void ClearRenderTarget(const D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptorHandle, const DirectX::XMVECTORF32& clearColor) noexcept;
	private:
		RenderCommand() noexcept = default;
		~RenderCommand() noexcept = default;
	};
}