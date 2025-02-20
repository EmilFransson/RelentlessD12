#include "Renderer.h"

#include "Core/Time.h"

#include "Graphics/RHI/CommandContext.h"
#include "Graphics/RHI/Device.h"
#include "Graphics/RHI/RingBufferAllocator.h"

namespace Relentless
{
	Renderer::Renderer(GraphicsDevice* pDevice) noexcept
		: m_pDevice{pDevice}
	{
		m_pForwardRenderer = std::make_unique<ForwardRenderer>(pDevice);
		m_pToyRenderer = std::make_unique<ToyRenderer>(pDevice);
	}

	void Renderer::Render(Scene* pScene, const ViewTransform& cameraViewTransform, const GraphicsOptions& graphicsOptions, Ref<TextureEx> pTarget) noexcept
	{
		m_pCurrentScene = pScene;

		const uint32 width = pTarget->GetWidth();
		const uint32 height = pTarget->GetHeight();

		m_MainView.Viewport = FloatRect(0, 0, width, height);
		m_MainView.pRenderer = this;
																													//Change to RGBA32_FLOAT for when tone map etc
		m_SceneTextures.pColorTarget = m_pDevice->CreateTexture(TextureDesc::Create2D(width, height, ResourceFormat::RGB10A2_UNORM), "Color Target");
		m_SceneTextures.pDepthTarget = m_pDevice->CreateTexture(TextureDesc::Create2D(width, height, ResourceFormat::D32_FLOAT), "Depth Target");

		{
			m_pDevice->GetRingBuffer()->Sync();
		}
		
		{
			CommandContext* pCommandContext = m_pDevice->AllocateCommandContext();
			UploadViewUniforms(*pCommandContext, m_MainView);
			pCommandContext->Execute();
		}
		
		{
			CommandContext* pCommandContext = m_pDevice->AllocateCommandContext();
			m_pForwardRenderer->Render(*pCommandContext, m_MainView, m_SceneTextures);
			pCommandContext->Execute();
		}

		{
			CommandContext* pCommandContext = m_pDevice->AllocateCommandContext(D3D12_COMMAND_LIST_TYPE_COMPUTE);
			m_pToyRenderer->Render(*pCommandContext, m_MainView, m_SceneTextures);
			pCommandContext->Execute();
		}

		CommandQueue* pDirectQueue = m_pDevice->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
		CommandQueue* pComputeQueue = m_pDevice->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE);
		pDirectQueue->InsertWait(pComputeQueue);

		{
			CommandContext* pCommandContext = m_pDevice->AllocateCommandContext();

			pCommandContext->InsertResourceBarrier(pTarget, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
			pCommandContext->InsertResourceBarrier(m_SceneTextures.pColorTarget, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_SOURCE);
		
			pCommandContext->CopyResource(m_SceneTextures.pColorTarget, pTarget);
		
			pCommandContext->InsertResourceBarrier(pTarget, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			pCommandContext->Execute();
		}

		//{
		//	CommandContext* pCommandContext = m_pDevice->AllocateCommandContext();
		//	pCommandContext->InsertResourceBarrier(pTarget, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		//	pCommandContext->Execute();
		//}

		m_Frame++;
	}

	void Renderer::BindViewData(CommandContext& commandContext, const RenderView& pRenderView) noexcept
	{
		ShaderInterop::ViewUniforms viewUniforms;
		pRenderView.pRenderer->GetViewUniforms(pRenderView, viewUniforms);
		commandContext.BindRootCBV(BindingSlot::PerView, &viewUniforms, sizeof(ShaderInterop::ViewUniforms));
	}

	void Renderer::GetViewUniforms(const RenderView& renderView, ShaderInterop::ViewUniforms& outViewUniform) noexcept
	{
		outViewUniform.WorldToView				= renderView.WorldToView;
		outViewUniform.ViewToWorld				= renderView.ViewToWorld;
		outViewUniform.ViewToClip				= renderView.ViewToClip;
		outViewUniform.ClipToView				= renderView.ClipToView;
		outViewUniform.WorldToClip				= renderView.WorldToClip;
		outViewUniform.ClipToWorld				= renderView.ClipToView * renderView.ViewToWorld;

		outViewUniform.ViewLocation				= renderView.Location;

		outViewUniform.ViewportDimensions		= Vector2(renderView.Viewport.GetWidth(), renderView.Viewport.GetHeight());
		outViewUniform.ViewportDimensionsInv	= Vector2(1.0f / renderView.Viewport.GetWidth(), 1.0f / renderView.Viewport.GetHeight());

		outViewUniform.FrameIndex				= m_Frame;
		outViewUniform.DeltaTime				= Time::GetDeltaTime();
		outViewUniform.ElapsedTime				= Time::GetElapsedTime();

		outViewUniform.NumInstances				= m_InstancesBuffer.Count;
		outViewUniform.LightCount				= m_LightsBuffer.Count;
	}

	void Renderer::UploadSceneData(CommandContext& commandContext) noexcept
	{
		
	}

	void Renderer::UploadViewUniforms(CommandContext& commandContext, RenderView& view) noexcept
	{
		ScratchAllocation alloc = commandContext.AllocateScratch(sizeof(ShaderInterop::ViewUniforms));
		ShaderInterop::ViewUniforms& parameters = alloc.As<ShaderInterop::ViewUniforms>();
		GetViewUniforms(view, parameters);
		
		if (!view.ViewCB)
			view.ViewCB = commandContext.GetParent()->CreateBuffer(BufferDesc{ .Size = sizeof(ShaderInterop::ViewUniforms), .ElementSize = sizeof(ShaderInterop::ViewUniforms) }, "ViewUniforms");
		
		commandContext.CopyBuffer(alloc.pBackingResource, view.ViewCB, alloc.Size, alloc.Offset, 0);
	}

}