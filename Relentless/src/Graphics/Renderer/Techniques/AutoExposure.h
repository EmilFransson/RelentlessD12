#pragma once
#include "Graphics/Renderer/RenderTypes.h"
#include "Graphics/RHI/Device.h"
#include "Graphics/RHI/PipelineState.h"
#include "Graphics/RHI/RHI.h"
#include "Graphics/RHI/Buffer.h"
#include "Graphics/RHI/Texture.h"

namespace Relentless
{
	class AutoExposure
	{
	public:
		AutoExposure(GraphicsDevice* pDevice) noexcept;
		void Render(CommandContext& commandContext, const RenderView& renderView, SceneTextures& sceneTextures, float aMinLogLuminance, float aMinEV100, float aMaxEV100, float aExposureCompensation) noexcept;
		NO_DISCARD Ref<Buffer> GetAverageLuminanceBuffer() const noexcept { return m_pAverageLuminance; }
	private:
		GraphicsDevice* m_pDevice = nullptr;

		Ref<PipelineState> m_pDownsampleColorPSO = nullptr;
		Ref<PipelineState> m_pLuminanceHistogramPSO = nullptr;
		Ref<PipelineState> m_pAverageLuminancePSO = nullptr;
		Ref<Buffer> m_pAverageLuminance = nullptr;
		Ref<Buffer> m_pLuminanceHistogram = nullptr;
		Ref<Texture> m_pDownscaleTarget = nullptr;
	};
}