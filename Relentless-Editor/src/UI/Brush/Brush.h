#pragma once
#include <Relentless.h>

namespace Relentless
{
	struct ThumbnailBrush
	{
		Color LineColor				= Colors::Transparent;
		Color BackgroundColor		= Colors::Transparent;
		Color TintColor				= Colors::White;
		Ref<Texture> BackingTexture = nullptr;
	};
}