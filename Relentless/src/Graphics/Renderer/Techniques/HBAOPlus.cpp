#include "HBAOPlus.h"

#include "Graphics/Renderer/Renderer.h"
#include "Graphics/RHI/CommandContext.h"
#include "Graphics/RHI/Device.h"
#include "Graphics/RHI/ResourceViews.h"

#include "../../../vendor/includes/HBAO+/GFSDK_SSAO.h"

namespace Relentless
{
	HBAOPlus::HBAOPlus(GraphicsDevice* pDevice) noexcept
		: m_pDevice{ pDevice }
	{
		GFSDK_SSAO_CustomHeap CustomHeap;
		CustomHeap.new_ = ::operator new;
		CustomHeap.delete_ = ::operator delete;

		m_ShaderBindableHandles = m_pDevice->RegisterGlobalDescriptorBlock(DescriptorHandleType::CBV, 64);
		m_RTVHandles = m_pDevice->RegisterGlobalDescriptorBlock(DescriptorHandleType::RTV, 64);

		GFSDK_SSAO_DescriptorHeaps_D3D12 DescriptorHeaps{};
		DescriptorHeaps.CBV_SRV_UAV.pDescHeap = m_pDevice->GetGlobalShaderBindableHeap()->GetDescriptorHeapInterface();
		DescriptorHeaps.CBV_SRV_UAV.BaseIndex = m_ShaderBindableHandles[0].Index;
		DescriptorHeaps.RTV.pDescHeap = m_pDevice->GetRenderTargetViewDescriptorHeap()->GetDescriptorHeapInterface();
		DescriptorHeaps.RTV.BaseIndex = m_RTVHandles[0].Index;

		const GFSDK_SSAO_Status status = GFSDK_SSAO_CreateContext_D3D12
		(
			m_pDevice->GetDevice(),
			1u,
			DescriptorHeaps,
			&m_pSSAOContext,
			&CustomHeap
		);
		RLS_VERIFY(status == GFSDK_SSAO_OK, "[HBAOPlus::HBAOPlus]: Failed To Initialize HBAO+.");
	}

	void HBAOPlus::Render(CommandContext& commandContext, const RenderView& renderView, SceneTextures& sceneTextures) noexcept
	{
		#if defined(RLS_DEBUG)
		// HBAO+ internally waits on fence value 0 on first frame — suppress known 3rd party warning
		Microsoft::WRL::ComPtr<ID3D12InfoQueue> infoQueue;
		bool hasInfoQueue = SUCCEEDED(m_pDevice->GetDevice()->QueryInterface(IID_PPV_ARGS(&infoQueue)));
		if (hasInfoQueue && !m_FirstFrameDone)
		{
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, FALSE);
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, FALSE);
			infoQueue->SetMuteDebugOutput(true);
		}
		#endif

		GFSDK_SSAO_InputData_D3D12 inputData = {};
		inputData.DepthData.DepthTextureType = GFSDK_SSAO_HARDWARE_DEPTHS;
		inputData.DepthData.FullResDepthTextureSRV.pResource = sceneTextures.pDepthTarget->GetResource();
		inputData.DepthData.FullResDepthTextureSRV.GpuHandle = sceneTextures.pDepthTarget->GetSRV()->GetGPUHandle().ptr;
		inputData.DepthData.ProjectionMatrix.Data = GFSDK_SSAO_Float4x4((const GFSDK_SSAO_FLOAT*)&renderView.ViewToClip);
		inputData.DepthData.ProjectionMatrix.Layout = GFSDK_SSAO_ROW_MAJOR_ORDER;
		inputData.DepthData.MetersToViewSpaceUnits = 1.0f;
		inputData.NormalData.Enable = false;

		const GFSDK_SSAO_RenderMask renderMask = GFSDK_SSAO_RENDER_AO;

		GFSDK_SSAO_RenderTargetView_D3D12 rtv{};
		rtv.pResource = sceneTextures.pColorTarget->GetResource();
		rtv.CpuHandle = sceneTextures.pColorTarget->GetRTV()->GetCPUHandle().ptr;

		GFSDK_SSAO_Output_D3D12 output{};
		output.pRenderTargetView = &rtv;
		output.Blend.Mode = GFSDK_SSAO_MULTIPLY_RGB;

		GFSDK_SSAO_Parameters parameters{};
		parameters.Radius = 2.f;
		parameters.Bias = 0.1f;
		parameters.PowerExponent = 2.f;
		parameters.Blur.Enable = true;
		parameters.Blur.Sharpness = 16.f;
		parameters.Blur.Radius = GFSDK_SSAO_BLUR_RADIUS_4;
		parameters.DepthStorage = GFSDK_SSAO_FP32_VIEW_DEPTHS;
		parameters.EnableDualLayerAO = false;
		parameters.StepCount = GFSDK_SSAO_STEP_COUNT_8;

		const uint32 width = sceneTextures.pColorTarget->GetWidth();
		const uint32 height = sceneTextures.pColorTarget->GetHeight();

		commandContext.InsertResourceBarrier(sceneTextures.pDepthTarget, sceneTextures.pDepthTarget->GetResourceState(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		sceneTextures.pDepthTarget->SetResourceState(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

		commandContext.SetViewport(FloatRect(0, 0, (float)width, (float)height), 0.0f, 1.0f);

		const GFSDK_SSAO_Status status = m_pSSAOContext->RenderAO(m_pDevice->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT)->GetCommandQueue(), commandContext.GetCommandList(), inputData, parameters, output, renderMask);
		RLS_VERIFY(status == GFSDK_SSAO_OK, "Failed To Issue HBAOPlus Render Command.");

		#if defined(RLS_DEBUG)
		if (hasInfoQueue && !m_FirstFrameDone)
		{
			// Restore breaks first, then pop filter
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
			infoQueue->SetMuteDebugOutput(false);
			m_FirstFrameDone = true;
		}
		#endif
	}
}

