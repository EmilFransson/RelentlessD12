#include "RenderCommand.h"
#include "../D3D12Core.h"
namespace Relentless
{
	void RenderCommand::TransitionResource(const Microsoft::WRL::ComPtr<ID3D12Resource> pResource, const D3D12_RESOURCE_STATES previousState, const D3D12_RESOURCE_STATES newState) noexcept
	{
		D3D12_RESOURCE_BARRIER resourceTransitionBarrier{};
		resourceTransitionBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		resourceTransitionBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		resourceTransitionBarrier.Transition.pResource = pResource.Get();
		resourceTransitionBarrier.Transition.StateBefore = previousState;
		resourceTransitionBarrier.Transition.StateAfter = newState;
		resourceTransitionBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		DXCall_STD(D3D12Core::GetCommandList()->ResourceBarrier(1u, &resourceTransitionBarrier));
	}

	void RenderCommand::ClearRenderTarget(const D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptorHandle, const DirectX::XMVECTORF32& clearColor) noexcept
	{
		DXCall_STD(D3D12Core::GetCommandList()->ClearRenderTargetView(cpuDescriptorHandle, clearColor, 0u, nullptr));
	}
}