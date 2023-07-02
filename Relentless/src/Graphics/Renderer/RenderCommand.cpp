#include "RenderCommand.h"
#include "../D3D12Core.h"
#include "../Resources/Texture.h"
#include "../Resources/Buffer.h"
#include "../Resources/DepthStencil.h"
#include "../../Core/Window.h"
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

	void RenderCommand::ResolveMSAA(const std::shared_ptr<RenderTexture>& pSource, const std::shared_ptr<RenderTexture>& pDestination) noexcept
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

	void RenderCommand::ClearRenderTarget(const D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptorHandle, const DirectX::XMFLOAT4& clearColor) noexcept
	{
		DirectX::XMVECTORF32 finalClearColor;
		finalClearColor.f[0] = clearColor.x;
		finalClearColor.f[1] = clearColor.y;
		finalClearColor.f[2] = clearColor.z;
		finalClearColor.f[3] = clearColor.w;

		DXCall_STD(D3D12Core::GetCommandList()->ClearRenderTargetView(cpuDescriptorHandle, finalClearColor, 0u, nullptr));
	}

	void RenderCommand::ClearDepthStencil(const std::shared_ptr<DepthStencil>& pDepthStencil) noexcept
	{
		DXCall_STD(D3D12Core::GetCommandList()->ClearDepthStencilView(pDepthStencil->GetDSVDescriptorHandle().CPUHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0u, 0u, nullptr));
	}

	void RenderCommand::SetRenderTarget(const std::shared_ptr<RenderTexture>& pRenderTarget, const std::shared_ptr<DepthStencil>& pDepthStencil) noexcept 
	{
		RLS_ASSERT(pRenderTarget, "Render target is invalid.");

		if (pDepthStencil)
		{
			DXCall_STD(D3D12Core::GetCommandList()->OMSetRenderTargets
			(
				1u,
				&pRenderTarget->GetRTVDescriptorHandle().CPUHandle,
				false,
				&pDepthStencil->GetDSVDescriptorHandle().CPUHandle
			));
		}
		else
		{
			DXCall_STD(D3D12Core::GetCommandList()->OMSetRenderTargets
			(
				1u,
				&pRenderTarget->GetRTVDescriptorHandle().CPUHandle,
				false,
				nullptr
			));
		}
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

	void RenderCommand::SetDescriptorHeap(const std::unique_ptr<DescriptorHeap>& pDescriptorHeap) noexcept
	{
		DXCall_STD(D3D12Core::GetCommandList()->SetDescriptorHeaps(1u, pDescriptorHeap->GetDescriptorHeapInterface().GetAddressOf()));
	}

	void RenderCommand::DrawInstanced(const uint32_t vertexCount, const uint32_t instanceCount) noexcept
	{
		DXCall_STD(D3D12Core::GetCommandList()->DrawInstanced(vertexCount, instanceCount, 0u, 0u));
	}

	//Debatable whether this should possibly transition as well...
	void RenderCommand::CopyTextureToTexture(const std::shared_ptr<Texture>& pSrcTexture, const std::shared_ptr<Texture>& pDstTexture) noexcept
	{
		RLS_ASSERT(pSrcTexture && pDstTexture, "Texture(s) invalid.");

		if (pSrcTexture->GetCurrentState() != D3D12_RESOURCE_STATE_COPY_SOURCE)
		{
			TransitionResource(pSrcTexture, D3D12_RESOURCE_STATE_COPY_SOURCE);
		}
		if (pDstTexture->GetCurrentState() != D3D12_RESOURCE_STATE_COPY_DEST)
		{
			TransitionResource(pDstTexture, D3D12_RESOURCE_STATE_COPY_DEST);
		}

		D3D12_PLACED_SUBRESOURCE_FOOTPRINT footPrint = {};
		auto desc = pSrcTexture->GetInterface()->GetDesc();

		DXCall_STD(D3D12Core::GetDevice()->GetCopyableFootprints(&desc, 0u, 1u, 0u, &footPrint, nullptr, nullptr, nullptr));

		D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
		srcLocation.pResource = pSrcTexture->GetInterface().Get();
		srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		srcLocation.SubresourceIndex = 0u;

		D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
		dstLocation.pResource = pDstTexture->GetInterface().Get();
		dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		dstLocation.PlacedFootprint = footPrint;

		DXCall_STD(D3D12Core::GetCommandList()->CopyTextureRegion(&dstLocation, 0u, 0u, 0u, &srcLocation, nullptr));
	}

	void RenderCommand::CopyTexelToBuffer(const std::shared_ptr<Texture>& pSrcTexture, const std::shared_ptr<ReadBackBuffer>& pDstBuffer, uint32_t x, uint32_t y, uint32_t texelSize) noexcept
	{
		RLS_ASSERT(pSrcTexture && pDstBuffer, "One or more resource is invalid.");
		RLS_ASSERT(!(texelSize > pDstBuffer->GetSize()), "Memory size to copy to buffer exceeds buffer capacity.");

		if (pSrcTexture->GetCurrentState() != D3D12_RESOURCE_STATE_COPY_SOURCE)
		{
			TransitionResource(pSrcTexture, D3D12_RESOURCE_STATE_COPY_SOURCE);
		}
		if (pDstBuffer->GetCurrentState() != D3D12_RESOURCE_STATE_COPY_DEST)
		{
			TransitionResource(pDstBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
		}

		D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
		srcLocation.pResource = pSrcTexture->GetInterface().Get();
		srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		srcLocation.SubresourceIndex = 0u;

		D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
		dstLocation.pResource = pDstBuffer->GetInterface().Get();
		dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		dstLocation.PlacedFootprint.Offset = 0;
		dstLocation.PlacedFootprint.Footprint.Format = pSrcTexture->GetFormat();
		dstLocation.PlacedFootprint.Footprint.Width = 1;
		dstLocation.PlacedFootprint.Footprint.Height = 1;
		dstLocation.PlacedFootprint.Footprint.Depth = 1;
		dstLocation.PlacedFootprint.Footprint.RowPitch = texelSize;

		D3D12_BOX areaToCopy{};
		areaToCopy.left = x;
		areaToCopy.right = x + 1;
		areaToCopy.top = y;
		areaToCopy.bottom = y + 1;
		areaToCopy.front = 0;
		areaToCopy.back = 1;

		DXCall_STD(D3D12Core::GetCommandList()->CopyTextureRegion(&dstLocation, 0u, 0u, 0u, &srcLocation, &areaToCopy));
	}
}