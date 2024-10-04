#pragma once
#include "ImGui/ImguiLayer.h"

namespace Relentless
{
	class SearchBar
	{
	public:		
		[[nodiscard]] std::string Draw(float width, const char* hintText, bool enableSearchHistory) noexcept;
	private:
		void DrawSearchBar(std::string_view hintText, bool shouldSaveToSearchHistory) noexcept;
		void DrawSearchIcon() noexcept;
		void DrawCancelIcon() noexcept;
		void DrawSearchHistoryPopupIcon() noexcept;
		void DrawSearchHistoryPopup() noexcept;
		[[nodiscard]] bool InputFieldContainsText() const noexcept;
		[[nodiscard]] int InputFieldCallback(ImGuiInputTextCallbackData* data) noexcept;
		void ResetInputBuffer() noexcept;
	private:
		struct CallbackUserData
		{
			std::string Input;
			SearchBar* OwningObject = nullptr;
		} m_CallbackUserData;

		char m_FullInputBuffer[128] = "        ";
		std::vector<std::string> m_SearchHistory;
		bool m_IsActive = false;
		bool m_IsHovered = false;
		bool m_SearchIconHovered = false;
		bool m_CancelIconHovered = false;
		bool m_ArrowDownIconHovered = false;
		bool m_WasFocused = false;
	};

	class UI
	{
	public:
		enum class Alignment : uint8_t { Left, Center, Right };

		static void Initialize() noexcept;
		static [[nodiscard]] std::string SearchBar(const char* uniqueID, const char* hintText, bool displaySearchHistory = false, float width = ImGui::GetContentRegionAvail().x) noexcept;
		
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