#pragma once

#include "Graphics/Renderer/RenderTypes.h"
#include "Graphics/RHI/RHI.h"

namespace Relentless
{
	static constexpr uint32 kMaxBlurRadius = 32;
	static constexpr uint32 kWeightCount = kMaxBlurRadius * 2 + 1;

	// This struct ensures each float is aligned to 16 bytes
	struct alignas(16) GaussianBlurCB
	{
		struct alignas(16) PaddedFloat
		{
			float value = 0.0f;
			float padding[3] = { 0.0f, 0.0f, 0.0f }; // pad to 16 bytes
		};

		PaddedFloat Weights[kWeightCount];
	};

	class Outlines
	{
	public:
		Outlines(GraphicsDevice* pDevice) noexcept;
		void Render(CommandContext& commandContext, const RenderView& renderView, SceneTextures& sceneTextures, Ref<TextureEx> pEntityIDTexture) noexcept;

		[[nodiscard]] Ref<TextureEx> GetSelectedEntityIDOutput() const noexcept;
		[[nodiscard]] Ref<TextureEx> GetBlurredOutput() const noexcept;
	private:
		GraphicsDevice* m_pDevice = nullptr;

		Ref<PipelineState> m_pSolidPSO = nullptr;
		Ref<PipelineState> m_pGaussianBlurPSO = nullptr;

		Ref<TextureEx> m_pSolidOutput = nullptr;
		Ref<TextureEx> m_pIntermediateBlur = nullptr;
		Ref<TextureEx> m_pBlurredOutput = nullptr;

		GaussianBlurCB m_CBData;
	};
}