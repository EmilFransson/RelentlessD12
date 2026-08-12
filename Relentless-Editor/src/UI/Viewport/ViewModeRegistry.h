#pragma once
#include "Graphics/Renderer/RenderViewModes.h"

namespace Relentless
{
	struct ViewModeInfo
	{
		ERenderViewMode Mode = ERenderViewMode::Lit;
		StringView      DisplayName = {};
		StringView      Icon = {};
		StringView		Description = {};
	};

	NO_DISCARD Span<const ViewModeInfo> GetViewModeRegistry() noexcept;
	NO_DISCARD const ViewModeInfo& GetViewModeInfo(ERenderViewMode aMode) noexcept;
}
