#include "RenderCommand.h"
#include "../D3D12Core.h"
#include "../Resources/Texture.h"
#include "../Resources/DepthStencil.h"
#include "../../Window.h"
namespace Relentless
{
	void RenderCommand::ResetFrameCommandUnits(const uint32_t frameIndex) noexcept
	{
		auto pCommandAllocator{ D3D12Core::GetCommandAllocator(frameIndex) };
		auto pCommandList{ D3D12Core::GetCommandList() };
		DXCall(pCommandAllocator->Reset());
		DXCall(pCommandList->Reset(pCommandAllocator.Get(), nullptr));
	}

	void RenderCommand::TransitionResource(const std::shared_ptr<IResource>& pResource, const D3D12_RESOURCE_STATES newState) noexcept
	{
		TransitionResource(pResource->GetInterface(), pResource->GetCurrentState(), newState);
		pResource->SetCurrentState(newState);
	}

	void RenderCommand::TransitionResource(const Microsoft::WRL::ComPtr<ID3D12Resource>& pResource, const D3D12_RESOURCE_STATES currentState, const D3D12_RESOURCE_STATES newState) noexcept
	{
		RLS_ASSERT(currentState != newState, "Current state and new state is identical.");
		RLS_ASSERT(pResource, "Resource pointer is invalid.");

		D3D12_RESOURCE_BARRIER resourceTransitionBarrier{};
		resourceTransitionBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		resourceTransitionBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		resourceTransitionBarrier.Transition.pResource = pResource.Get();
		resourceTransitionBarrier.Transition.StateBefore = currentState;
		resourceTransitionBarrier.Transition.StateAfter = newState;
		resourceTransitionBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		DXCall_STD(D3D12Core::GetCommandList()->ResourceBarrier(1u, &resourceTransitionBarrier));
	}

	void RenderCommand::ResolveMSAA(const std::shared_ptr<Texture>& pSource, const std::shared_ptr<Texture>& pDestination) noexcept
	{
		RLS_ASSERT(pSource && pDestination, "Texture pointers are invalid.");
		RLS_ASSERT(pSource->GetFormat() == pDestination->GetFormat(), "Format types are not identical.");

		DXCall_STD(D3D12Core::GetCommandList()->ResolveSubresource
		(
			pDestination->GetInterface().Get(),
			0,
			pSource->GetInterface().Get(),
			0,
			pDestination->GetFormat())
		);
	}

	void RenderCommand::ClearRenderTarget(const D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptorHandle, const DirectX::XMVECTORF32& clearColor) noexcept
	{
		DirectX::XMVECTOR convertedColor = DirectX::XMColorSRGBToRGB(clearColor);
		DirectX::XMVECTORF32 finalClearColor;
		finalClearColor.f[0] = DirectX::XMVectorGetX(convertedColor);
		finalClearColor.f[1] = DirectX::XMVectorGetY(convertedColor);
		finalClearColor.f[2] = DirectX::XMVectorGetZ(convertedColor);
		finalClearColor.f[3] = DirectX::XMVectorGetW(convertedColor);

		DXCall_STD(D3D12Core::GetCommandList()->ClearRenderTargetView(cpuDescriptorHandle, finalClearColor, 0u, nullptr));
	}

	void RenderCommand::ClearDepthStencil(const std::shared_ptr<DepthStencil>& pDepthStencil) noexcept
	{
		DXCall_STD(D3D12Core::GetCommandList()->ClearDepthStencilView(pDepthStencil->GetDSVDescriptorHandle().CPUHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0u, 0u, nullptr));
	}

	void RenderCommand::SetRenderTarget(const std::shared_ptr<RenderTexture>& pRenderTarget, const std::shared_ptr<DepthStencil>& pDepthStencil) noexcept 
	{
		RLS_ASSERT(pRenderTarget, "Render target is invalid.");

		DXCall_STD(D3D12Core::GetCommandList()->OMSetRenderTargets
		(
			1u, 
			&pRenderTarget->GetRTVDescriptorHandle().CPUHandle, 
			false, 
			&pDepthStencil->GetDSVDescriptorHandle().CPUHandle
		));
	}
	void RenderCommand::SetRenderTarget(const BackBuffer& backBuffer) noexcept 
	{
		DXCall_STD(D3D12Core::GetCommandList()->OMSetRenderTargets(1u, &backBuffer.Handle.CPUHandle, false, nullptr));
	}

	void RenderCommand::SetViewport(const D3D12_VIEWPORT& viewport) noexcept
	{
		DXCall_STD(D3D12Core::GetCommandList()->RSSetViewports(1u, &viewport));
	}

	void RenderCommand::SetScissorRect(const RECT& scissorRect) noexcept
	{
		DXCall_STD(D3D12Core::GetCommandList()->RSSetScissorRects(1u, &scissorRect));
	}

	void RenderCommand::SetTopology(const D3D12_PRIMITIVE_TOPOLOGY topology) noexcept
	{
		DXCall_STD(D3D12Core::GetCommandList()->IASetPrimitiveTopology(topology));
	}

	void RenderCommand::SetPipelineState(const Microsoft::WRL::ComPtr<ID3D12PipelineState>& pPipelineState) noexcept
	{
		DXCall_STD(D3D12Core::GetCommandList()->SetPipelineState(pPipelineState.Get()));
	}

	void RenderCommand::SetRootSignature(const Microsoft::WRL::ComPtr<ID3D12RootSignature>& pRootSignature) noexcept
	{
		DXCall_STD(D3D12Core::GetCommandList()->SetGraphicsRootSignature(pRootSignature.Get()));
	}
}