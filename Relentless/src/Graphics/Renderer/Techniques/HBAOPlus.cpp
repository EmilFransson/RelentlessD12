#include "HBAOPlus.h"

#include "Graphics/Renderer/Renderer.h"
#include "Graphics/RHI/CommandContext.h"
#include "Graphics/RHI/Device.h"
#include "Graphics/RHI/ResourceViews.h"

#include "Subsystem/CoreTypes/PostProcessRenderSubsystem.h"

#include "../../../vendor/includes/HBAO+/GFSDK_SSAO.h"

namespace Relentless
{
	HBAOPlus::HBAOPlus(GraphicsDevice* aGraphicsDevice) noexcept
		: m_pDevice{ aGraphicsDevice }
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

	HBAOPlus::~HBAOPlus() noexcept
	{
		m_pSSAOContext->Release();
	}

	void HBAOPlus::Render(CommandContext& aCommandContext, const RenderView& aRenderView, SceneTextures& aSceneTextures) noexcept
	{
		PostProcessRenderSubsystem* pPostProcessRenderSubsystem = aRenderView.pRenderScene->GetSubsystem<PostProcessRenderSubsystem>();
		const PostProcessRenderProxy& renderProxy = pPostProcessRenderSubsystem->GetRenderProxy();
		const AmbientOcclusionProxySettings& ambientOcclusionSettings = renderProxy.AmbientOcclusionProxySettings;

		if (!ambientOcclusionSettings.IsEnabled)
			return;

		RLS_ASSERT(ambientOcclusionSettings.BlurRadius == 2u || ambientOcclusionSettings.BlurRadius == 4u, "[HBAOPlus::Render]: Invalid Blur Radius Encountered.");
		RLS_ASSERT(ambientOcclusionSettings.DepthPrecision == 16u || ambientOcclusionSettings.DepthPrecision == 32u, "[HBAOPlus::Render]: Invalid Depth Precision Encountered.");
		RLS_ASSERT(ambientOcclusionSettings.StepCount == 4u || ambientOcclusionSettings.StepCount == 8u, "[HBAOPlus::Render]: Invalid Step Count Encountered.");

		// HBAO+ internally waits on fence value 0 on first frame — suppress known 3rd party warning
		Microsoft::WRL::ComPtr<ID3D12InfoQueue> infoQueue;
		bool hasInfoQueue = SUCCEEDED(m_pDevice->GetDevice()->QueryInterface(IID_PPV_ARGS(&infoQueue)));
		if (hasInfoQueue && !m_FirstFrameDone)
		{
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, FALSE);
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, FALSE);
			infoQueue->SetMuteDebugOutput(true);
		}
		//#endif

		GFSDK_SSAO_InputData_D3D12 inputData = {};
		inputData.DepthData.DepthTextureType = GFSDK_SSAO_HARDWARE_DEPTHS;
		inputData.DepthData.FullResDepthTextureSRV.pResource = aSceneTextures.pDepthTarget->GetResource();
		inputData.DepthData.FullResDepthTextureSRV.GpuHandle = aSceneTextures.pDepthTarget->GetSRV()->GetGPUHandle().ptr;
		inputData.DepthData.ProjectionMatrix.Data = GFSDK_SSAO_Float4x4((const GFSDK_SSAO_FLOAT*)&aRenderView.ViewToClip);
		inputData.DepthData.ProjectionMatrix.Layout = GFSDK_SSAO_ROW_MAJOR_ORDER;
		inputData.DepthData.MetersToViewSpaceUnits = 1.0f;
		inputData.NormalData.Enable = false;

		const GFSDK_SSAO_RenderMask renderMask = GFSDK_SSAO_RENDER_AO;

		GFSDK_SSAO_RenderTargetView_D3D12 rtv{};
		rtv.pResource = aSceneTextures.pHDRColorTarget->GetResource();
		rtv.CpuHandle = aSceneTextures.pHDRColorTarget->GetRTV()->GetCPUHandle().ptr;

		GFSDK_SSAO_Output_D3D12 output{};
		output.pRenderTargetView = &rtv;
		output.Blend.Mode = GFSDK_SSAO_MULTIPLY_RGB;

		GFSDK_SSAO_Parameters parameters{};
		parameters.Radius = ambientOcclusionSettings.Radius;
		parameters.Bias = ambientOcclusionSettings.Bias;
		parameters.PowerExponent = ambientOcclusionSettings.PowerExponent;
		parameters.Blur.Enable = ambientOcclusionSettings.BlurEnabled;
		parameters.Blur.Sharpness = ambientOcclusionSettings.BlurSharpness;
		parameters.Blur.Radius = ambientOcclusionSettings.BlurRadius == 4u ? GFSDK_SSAO_BLUR_RADIUS_4 : GFSDK_SSAO_BLUR_RADIUS_2;
		parameters.DepthStorage = ambientOcclusionSettings.DepthPrecision == 32u ? GFSDK_SSAO_FP32_VIEW_DEPTHS : GFSDK_SSAO_FP16_VIEW_DEPTHS ;
		parameters.EnableDualLayerAO = false;
		parameters.StepCount = ambientOcclusionSettings.StepCount == 8u ? GFSDK_SSAO_STEP_COUNT_8 : GFSDK_SSAO_STEP_COUNT_4;

		const uint32 width = aSceneTextures.pHDRColorTarget->GetWidth();
		const uint32 height = aSceneTextures.pHDRColorTarget->GetHeight();

		aCommandContext.InsertResourceBarrier(aSceneTextures.pHDRColorTarget, D3D12_RESOURCE_STATE_RENDER_TARGET);
		aCommandContext.InsertResourceBarrier(aSceneTextures.pDepthTarget, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		aCommandContext.FlushResourceBarriers();

		aCommandContext.SetViewport(FloatRect(0.0f, 0.0f, (float)width, (float)height), 0.0f, 1.0f);

		const GFSDK_SSAO_Status status = m_pSSAOContext->RenderAO(m_pDevice->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT)->GetCommandQueue(), aCommandContext.GetCommandList(), inputData, parameters, output, renderMask);
		RLS_VERIFY(status == GFSDK_SSAO_OK, "Failed To Issue HBAOPlus Render Command.");

		if (hasInfoQueue && !m_FirstFrameDone)
		{
			// Restore breaks first, then pop filter
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
			infoQueue->SetMuteDebugOutput(false);
			m_FirstFrameDone = true;
		}
	}
}

