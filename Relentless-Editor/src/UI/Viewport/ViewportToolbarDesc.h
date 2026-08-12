#pragma once

namespace Relentless
{
	struct ViewportToolbarDesc
	{
		float Height = 25.0f;
		float Margin = 3.0f;
		float GroupSpacing = 10.0f;
		bool Enabled = true;

		NO_DISCARD static ViewportToolbarDesc Disabled() noexcept
		{
			ViewportToolbarDesc desc;
			desc.Enabled = false;

			return desc;
		}

		NO_DISCARD bool IsEnabled() const noexcept { return Enabled; }
	};
}