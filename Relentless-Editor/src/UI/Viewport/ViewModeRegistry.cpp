#include "ViewModeRegistry.h"

namespace Relentless
{
	namespace
	{
		constexpr ViewModeInfo kViewModes[] =
		{
			{ ERenderViewMode::Lit,              "Lit",               ICON_FA_CIRCLE_HALF_STROKE,			"Renders the scene with normal lighting."},
			{ ERenderViewMode::Unlit,            "Unlit",             ICON_FA_CIRCLE,						"Renders the scene with no lights."},
			{ ERenderViewMode::GeometricNormals, "Geometric Normals", ICON_FA_ARROW_UP_FROM_BRACKET,		"Geometric Normals."},
			{ ERenderViewMode::ShadingNormals,   "Shading Normals",   ICON_FA_ARROW_UP_RIGHT_FROM_SQUARE ,	"Shading Normals."},
			{ ERenderViewMode::Metallic,         "Metallic",          ICON_FA_MERCURY,						"Metallic."},
			{ ERenderViewMode::Roughness,        "Roughness",         ICON_FA_MOUND,						"Roughness."},
			{ ERenderViewMode::AmbientOcclusion, "Ambient Occlusion", ICON_FA_CLOUD,						"Ambient Occlusion."},
			{ ERenderViewMode::Opacity,          "Opacity",           ICON_FA_DROPLET,						"Opacity."},
		};
	}

	Span<const ViewModeInfo> GetViewModeRegistry() noexcept
	{
		return kViewModes;
	}

	const ViewModeInfo& GetViewModeInfo(ERenderViewMode aMode) noexcept
	{
		return kViewModes[(uint32)aMode];
	}
}