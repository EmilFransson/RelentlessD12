#include "ImGuiFonts.h"

namespace Relentless::UI
{
	static std::unordered_map<String, ImFont*> s_Fonts;

	void Fonts::Add(const FontConfiguration& aConfig, bool aIsDefault)
	{
		if (s_Fonts.find(aConfig.FontName) != s_Fonts.end())
		{
			RLS_CORE_WARN("[Fonts::Add]: Tried to add font with name '{0}' but that name is already taken!", aConfig.FontName);
			return;
		}

		const String fullPath = String(EDITOR_RESOURCE_DIRECTORY) + String(aConfig.FilePath);

		ImFontConfig imguiFontConfig;
		imguiFontConfig.MergeMode = aConfig.MergeWithLast;
		auto& io = ImGui::GetIO();
		ImFont* font = io.Fonts->AddFontFromFileTTF(fullPath.c_str(), aConfig.Size, &imguiFontConfig, aConfig.GlyphRanges == nullptr ? io.Fonts->GetGlyphRangesDefault() : aConfig.GlyphRanges);
		RLS_VERIFY(font, "[Fonts::Add]: Failed to load font file!");
		s_Fonts[aConfig.FontName] = font;

		if (aIsDefault)
			io.FontDefault = font;
	}

	ImFont* Fonts::Get(const String& aFontName)
	{
		RLS_VERIFY(s_Fonts.find(aFontName) != s_Fonts.end(), "[Fonts::Get]: Failed to find font with that name!");
		return s_Fonts.at(aFontName);
	}

	void Fonts::PushFont(const String& aFontName)
	{
		const auto& io = ImGui::GetIO();

		if (s_Fonts.find(aFontName) == s_Fonts.end())
		{
			ImGui::PushFont(io.FontDefault);
			return;
		}

		ImGui::PushFont(s_Fonts.at(aFontName));
	}

	void Fonts::PopFont()
	{
		ImGui::PopFont();
	}

	Vector2 CalculateTextSize(const char* aText) noexcept
	{
		const ImVec2 size = ImGui::CalcTextSize(aText);
		return Vector2(size.x, size.y);
	}
}