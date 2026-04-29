#include "AutoExposure.h"

#include "Core/Time.h"

#include "Graphics/Renderer/Renderer.h"
#include "Graphics/RHI/CommandContext.h"
#include "Graphics/RHI/Device.h"
#include "Graphics/RHI/PipelineState.h"

namespace Relentless
{
	AutoExposure::AutoExposure(GraphicsDevice* pDevice) noexcept
		: m_pDevice{ pDevice }
	{
	}

	void AutoExposure::Render(CommandContext& aCommandContext, MAYBE_UNUSED const RenderView& aRenderView, SceneTextures& aSceneTextures, SceneBuffers& aSceneBuffers, float aMinLogLuminance, float aMinEV100, float aMaxEV100, float aExposureCompensation) noexcept
	{
		m_pDevice->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE)->InsertWait(m_pDevice->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT));

		aCommandContext.InsertResourceBarrier(aSceneTextures.pAutoExposureDownscaleTarget, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		aCommandContext.InsertResourceBarrier(aSceneTextures.pColorTarget, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		aCommandContext.SetComputeRootSignature(m_pDevice->GetGlobalRootSignature());
		aCommandContext.SetPipelineState(m_pDevice->GetOrCreateComputePipeline(m_pDevice->GetGlobalRootSignature(), "DownsampleColorShader", "cs_main"));

		//Downsample:
		{
			struct
			{
				Vector2i	TargetDimensions;
				Vector2		TargetDimensionsInv;
				uint32		InputIndex;
				uint32		OutputIndex;
				Vector2		Padding;
			} parameters;

			parameters.TargetDimensions.x = aSceneTextures.pAutoExposureDownscaleTarget->GetWidth();
			parameters.TargetDimensions.y = aSceneTextures.pAutoExposureDownscaleTarget->GetHeight();
			parameters.TargetDimensionsInv = Vector2(1.0f / aSceneTextures.pAutoExposureDownscaleTarget->GetWidth(), 1.0f / aSceneTextures.pAutoExposureDownscaleTarget->GetHeight());
			parameters.InputIndex = aSceneTextures.pColorTarget->GetSRVIndex();
			parameters.OutputIndex = aSceneTextures.pAutoExposureDownscaleTarget->GetUAVIndex();
			aCommandContext.BindRootCBV(BindingSlot::PerInstance, &parameters, sizeof(parameters));

			aCommandContext.Dispatch(ComputeUtils::GetNumThreadGroups(parameters.TargetDimensions.x, 8, parameters.TargetDimensions.y, 8));
		}

		aCommandContext.InsertUAVBarrier();

		//Luminance Histogram:
		aCommandContext.InsertResourceBarrier(aSceneBuffers.LuminanceHistogramBuffer.pBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		aCommandContext.InsertResourceBarrier(aSceneTextures.pAutoExposureDownscaleTarget, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		{
			aCommandContext.ClearBufferUInt(aSceneBuffers.LuminanceHistogramBuffer.pBuffer);

			aCommandContext.SetComputeRootSignature(m_pDevice->GetGlobalRootSignature());
			aCommandContext.SetPipelineState(m_pDevice->GetOrCreateComputePipeline(m_pDevice->GetGlobalRootSignature(), "LuminanceHistogramShader", "cs_main"));

			struct
			{
				uint32 Width;
				uint32 Height;
				float MinLogLuminance;
				float OneOverLogLuminanceRange;
				uint32 HDRTextureIndex;
				uint32 LuminanceHistogramIndex;
			} parameters;

			parameters.Width = aSceneTextures.pAutoExposureDownscaleTarget->GetWidth();
			parameters.Height = aSceneTextures.pAutoExposureDownscaleTarget->GetHeight();
			parameters.MinLogLuminance = aMinLogLuminance;
			parameters.OneOverLogLuminanceRange = 1.0f / (20.0f - aMinLogLuminance);
			parameters.HDRTextureIndex = aSceneTextures.pAutoExposureDownscaleTarget->GetSRVIndex();
			parameters.LuminanceHistogramIndex = aSceneBuffers.LuminanceHistogramBuffer.pBuffer->GetUAVIndex();
			aCommandContext.BindRootCBV(BindingSlot::PerInstance, &parameters, sizeof(parameters));

			aCommandContext.Dispatch(ComputeUtils::GetNumThreadGroups(parameters.Width, 16, parameters.Height, 16));
		}
	
		aCommandContext.InsertUAVBarrier();

		{
			aCommandContext.InsertResourceBarrier(aSceneBuffers.AverageLuminanceBuffer.pBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			aCommandContext.InsertResourceBarrier(aSceneBuffers.LuminanceHistogramBuffer.pBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

			aCommandContext.SetComputeRootSignature(m_pDevice->GetGlobalRootSignature());
			aCommandContext.SetPipelineState(m_pDevice->GetOrCreateComputePipeline(m_pDevice->GetGlobalRootSignature(), "AverageLuminanceShader", "cs_main"));

			struct
			{
				uint32	PixelCount;
				float	MinLogLuminance;
				float	LogLuminanceRange;
				float	TimeDelta;
				float	SpeedUp;
				float	SpeedDown;
				float	ExposureCompensation;
				float	MinEV100;
				float	MaxEV100;
				float   LowPercent;
				float   HighPercent;
				uint32	LuminanceHistogramIndex;
				uint32 	LuminanceOutputIndex;
				float	Padding0;
				float	Padding1;
				float	Padding2;
			} parameters;

			parameters.PixelCount = aSceneTextures.pAutoExposureDownscaleTarget->GetWidth() * aSceneTextures.pAutoExposureDownscaleTarget->GetHeight();
			parameters.MinLogLuminance = aMinLogLuminance;
			parameters.LogLuminanceRange = 20.0f - aMinLogLuminance;
			parameters.TimeDelta = Time::GetDeltaTime();
			parameters.SpeedUp = 6.0f;
			parameters.SpeedDown = 5.0f;
			parameters.ExposureCompensation = aExposureCompensation;
			parameters.MinEV100 = aMinEV100;
			parameters.MaxEV100 = aMaxEV100;
			parameters.LowPercent = 0.1f;
			parameters.HighPercent = 0.90f;
			parameters.LuminanceHistogramIndex = aSceneBuffers.LuminanceHistogramBuffer.pBuffer->GetSRVIndex();
			parameters.LuminanceOutputIndex = aSceneBuffers.AverageLuminanceBuffer.pBuffer->GetUAVIndex();

			aCommandContext.BindRootCBV(BindingSlot::PerInstance, &parameters, sizeof(parameters));

			aCommandContext.Dispatch(1);
		}

		aCommandContext.InsertResourceBarrier(aSceneBuffers.AverageLuminanceBuffer.pBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	}

}