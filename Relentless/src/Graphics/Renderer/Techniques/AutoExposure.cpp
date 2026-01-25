#include "AutoExposure.h"

#include "Graphics/Renderer/Renderer.h"
#include "Graphics/RHI/CommandContext.h"

#include "Core/Time.h"

namespace Relentless
{
	AutoExposure::AutoExposure(GraphicsDevice* pDevice) noexcept
		: m_pDevice{ pDevice }
	{
		m_pDownsampleColorPSO = m_pDevice->CreateComputePipeline(m_pDevice->GetGlobalRootSignature(), "DownsampleColor", "cs_main");
		m_pLuminanceHistogramPSO = m_pDevice->CreateComputePipeline(m_pDevice->GetGlobalRootSignature(), "LuminanceHistogram", "cs_main");
		m_pAverageLuminancePSO = m_pDevice->CreateComputePipeline(m_pDevice->GetGlobalRootSignature(), "AverageLuminance", "cs_main");
	}

	void AutoExposure::Render(CommandContext& commandContext, MAYBE_UNUSED const RenderView& renderView, SceneTextures& sceneTextures, float aMinLogLuminance, float aMinEV100, float aMaxEV100, float aExposureCompensation) noexcept
	{
		m_pDevice->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE)->InsertWait(m_pDevice->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT));

		Ref<Texture> pColor = sceneTextures.pColorTarget;
		TextureDesc sourceDesc = pColor->GetDesc();
		if (!m_pDownscaleTarget || m_pDownscaleTarget->GetWidth() != Math::DivideAndRoundUp(sourceDesc.Width, 4) || m_pDownscaleTarget->GetHeight() != Math::DivideAndRoundUp(sourceDesc.Height, 4) ||
			m_pDownscaleTarget->GetFormat() != sourceDesc.Format)
		{
			const uint32 width = Math::DivideAndRoundUp(sourceDesc.Width, 4);
			const uint32 height = Math::DivideAndRoundUp(sourceDesc.Height, 4);
			const ResourceFormat colorFormat = sourceDesc.Format;
			m_pDownscaleTarget = m_pDevice->CreateTexture(TextureDesc::Create2D(width, height, colorFormat, 1u, TextureFlag::UnorderedAccess | TextureFlag::ShaderResource), "Downscaled HDR Target");
		}

		commandContext.InsertResourceBarrier(m_pDownscaleTarget.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandContext.InsertResourceBarrier(pColor, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		commandContext.SetComputeRootSignature(m_pDevice->GetGlobalRootSignature());
		commandContext.SetPipelineState(m_pDownsampleColorPSO);

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

			parameters.TargetDimensions.x = m_pDownscaleTarget->GetWidth();
			parameters.TargetDimensions.y = m_pDownscaleTarget->GetHeight();
			parameters.TargetDimensionsInv = Vector2(1.0f / m_pDownscaleTarget->GetWidth(), 1.0f / m_pDownscaleTarget->GetHeight());
			parameters.InputIndex = pColor->GetSRVIndex();
			parameters.OutputIndex = m_pDownscaleTarget->GetUAVIndex();
			commandContext.BindRootCBV(BindingSlot::PerInstance, &parameters, sizeof(parameters));

			commandContext.Dispatch(ComputeUtils::GetNumThreadGroups(parameters.TargetDimensions.x, 8, parameters.TargetDimensions.y, 8));
		}

		commandContext.InsertUAVBarrier();

		if (!m_pLuminanceHistogram)
		{
			m_pLuminanceHistogram = m_pDevice->CreateBuffer(BufferDesc::CreateTyped(256, ResourceFormat::R32_UINT, BufferFlag::UnorderedAccess), "Luminance Histogram");
		}

		//Luminance Histogram:
		commandContext.InsertResourceBarrier(m_pLuminanceHistogram.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandContext.InsertResourceBarrier(m_pDownscaleTarget.Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		{
			commandContext.ClearBufferUInt(m_pLuminanceHistogram);

			commandContext.SetComputeRootSignature(m_pDevice->GetGlobalRootSignature());
			commandContext.SetPipelineState(m_pLuminanceHistogramPSO);

			struct
			{
				uint32 Width;
				uint32 Height;
				float MinLogLuminance;
				float OneOverLogLuminanceRange;
				uint32 HDRTextureIndex;
				uint32 LuminanceHistogramIndex;
			} parameters;

			parameters.Width = m_pDownscaleTarget->GetWidth();
			parameters.Height = m_pDownscaleTarget->GetHeight();
			parameters.MinLogLuminance = aMinLogLuminance;
			parameters.OneOverLogLuminanceRange = 1.0f / (20.0f - aMinLogLuminance);
			parameters.HDRTextureIndex = m_pDownscaleTarget->GetSRVIndex();
			parameters.LuminanceHistogramIndex = m_pLuminanceHistogram->GetUAVIndex();
			commandContext.BindRootCBV(BindingSlot::PerInstance, &parameters, sizeof(parameters));

			commandContext.Dispatch(ComputeUtils::GetNumThreadGroups(m_pDownscaleTarget->GetWidth(), 16, m_pDownscaleTarget->GetHeight(), 16));
		}
	
		commandContext.InsertUAVBarrier();
	
		if (!m_pAverageLuminance)
		{
			m_pAverageLuminance = m_pDevice->CreateBuffer(BufferDesc::CreateStructured(3u, sizeof(float), BufferFlag::UnorderedAccess), "Average Luminance");
		}

		{
			commandContext.InsertResourceBarrier(m_pAverageLuminance.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			commandContext.InsertResourceBarrier(m_pLuminanceHistogram.Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

			commandContext.SetComputeRootSignature(m_pDevice->GetGlobalRootSignature());
			commandContext.SetPipelineState(m_pAverageLuminancePSO);

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

			parameters.PixelCount = m_pDownscaleTarget->GetWidth() * m_pDownscaleTarget->GetHeight();
			parameters.MinLogLuminance = aMinLogLuminance;
			parameters.LogLuminanceRange = 20.0f - aMinLogLuminance;
			parameters.TimeDelta = Time::GetDeltaTime();
			parameters.SpeedUp = 3.0f;
			parameters.SpeedDown = 1.0f;
			parameters.ExposureCompensation = aExposureCompensation;
			parameters.MinEV100 = aMinEV100;
			parameters.MaxEV100 = aMaxEV100;
			parameters.LowPercent = 0.05f;
			parameters.HighPercent = 0.95f;
			parameters.LuminanceHistogramIndex = m_pLuminanceHistogram->GetSRVIndex();
			parameters.LuminanceOutputIndex = m_pAverageLuminance->GetUAVIndex();

			commandContext.BindRootCBV(BindingSlot::PerInstance, &parameters, sizeof(parameters));

			commandContext.Dispatch(1);
		}

		commandContext.InsertResourceBarrier(m_pAverageLuminance.Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	}

}