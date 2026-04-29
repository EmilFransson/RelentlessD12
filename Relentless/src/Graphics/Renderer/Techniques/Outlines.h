#pragma once
#include "Graphics/Renderer/RenderTypes.h"
#include "Graphics/RHI/RHI.h"

namespace Relentless
{
	static constexpr uint32 kMaxBlurRadius = 32;
	static constexpr uint32 kWeightCount = kMaxBlurRadius * 2 + 1;

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
		Outlines(GraphicsDevice* aGraphicsDevice) noexcept;
		void Render(CommandContext& aCommandContext, const RenderView& aRenderView, SceneTextures& aSceneTextures) noexcept;
	private:
		GraphicsDevice* m_pDevice = nullptr;

		GaussianBlurCB m_CBData;
	};
}