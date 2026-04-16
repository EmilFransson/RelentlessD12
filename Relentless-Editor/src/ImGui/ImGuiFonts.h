#pragma once
#include "ImGuiIncludes.h"

namespace Relentless::UI
{
	struct FontConfiguration
	{
		String FontName;
		StringView FilePath;
		float Size = 16.0f;
		const ImWchar* GlyphRanges = nullptr;
		bool MergeWithLast = false;
	};

	class Fonts
	{
	public:
		static void Add(const FontConfiguration& aConfig, bool aIsDefault = false);
		static void PushFont(const String& aFontName);
		static void PopFont();
		static ImFont* Get(const String& aFontName);
	};
}