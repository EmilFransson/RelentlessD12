#pragma once
#include <Relentless.h>

namespace Relentless
{
	struct ThumbnailInfo : public RefCounted<ThumbnailInfo>
	{
		String DisplayName;
		String Label;
		Color TypeColor = Colors::White;
		Color TintColor = Colors::White;
		uint32 Width = 0u;
		uint32 Height = 0u;
	};

	struct ThumbnailRenderData : public RefCounted<ThumbnailRenderData >
	{
		uint64 RenderHandle = std::numeric_limits<uint64>::max();
	};
}