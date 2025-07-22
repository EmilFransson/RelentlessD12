#pragma once
#include "ImGui/ImguiLayer.h"
#include "Assets/AssetManager.h"

namespace Relentless
{
	class UI
	{
	public:
		enum class Alignment : uint8_t { Left, Center, Right };

		static void Initialize() noexcept;
		
		static AssetHandle SearchIconTextureHandle;
		static AssetHandle CancelIconTextureHandle;
		static AssetHandle ArrowDownIconTextureHandle;

		class Utility
		{
		public:
			static [[nodiscard]] std::string ShortenStringToFitClipRect(const std::string& originalString, const ImVec2& topLeft, const ImVec2& bottomRight) noexcept;
			static void DrawTitledSeparator(const std::string& title, const ImVec2& begin, const ImVec2& end) noexcept;
			static void DrawTooltip(const char* tooltip) noexcept;
			static [[nodiscard]] float CalculateTextHeight(const std::string& text, float wrapWidth = -1.0f) noexcept;
		};
	};
}