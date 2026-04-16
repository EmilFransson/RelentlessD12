#pragma once
#include <Relentless.h>

namespace Relentless
{
	struct ThumbnailBrush
	{
		Color TypeColor				= Colors::Black;
		Ref<Texture> BackingTexture = nullptr;
	};
}