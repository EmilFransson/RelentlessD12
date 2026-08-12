#pragma once

namespace Relentless
{
	struct ViewportSidePanelDesc
	{
		float Width = 250.0f;
		bool Enabled = true;
		bool StartVisible = false;
		bool CanToggle = true;

		NO_DISCARD static ViewportSidePanelDesc Disabled() noexcept
		{
			ViewportSidePanelDesc desc;
			desc.Enabled = false;

			return desc;
		}

		NO_DISCARD bool IsEnabled() const noexcept { return Enabled; }
	};
}