#pragma once

namespace Relentless
{
	enum class EMSAASampleCount : uint8 { None = 1u, x2 = 2u, x4 = 4u, x8 = 8u };

	struct RLS_API RenderQualitySettings
	{
		EMSAASampleCount MSAASampleCount = EMSAASampleCount::None;
	};
}