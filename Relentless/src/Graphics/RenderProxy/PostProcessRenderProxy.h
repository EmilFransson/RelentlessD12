#pragma once

namespace Relentless
{
	struct ExposureRenderProxySettings
	{
		float Compensation		= 1.0f;
		float MinEV100			= -10.0f;
		float MaxEV100			= 20.0f;
		float SpeedUp			= 6.0f;
		float SpeedDown			= 5.0f;
		float LowPercent		= 10.0f;
		float HighPercent		= 90.0f;
		float HistogramMinEV100 = -10.0f;
		float HistogramMaxEV100 = 16.0f;
	};

	struct AmbientOcclusionProxySettings
	{
		float Radius			= 2.0f;
		float Bias				= 0.1f;
		float PowerExponent		= 2.0f;
		float BlurSharpness		= 16.0f;
		uint8 BlurRadius		= 4u;
		uint8 DepthPrecision	= 32u;
		uint8 StepCount			= 8u;
		bool BlurEnabled		= true;
		bool IsEnabled			= true;
	};

	struct PostProcessRenderProxy
	{
		ExposureRenderProxySettings ExposureRenderProxySettings{};
		AmbientOcclusionProxySettings AmbientOcclusionProxySettings{};

		uint32 ID = std::numeric_limits<uint32>::max();
	};
}