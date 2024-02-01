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
		static void Initialize() noexcept;
		[[nodiscard]] static std::string SearchBar(const char* uniqueID, const char* hintText, bool displaySearchHistory = false, float width = ImGui::GetContentRegionAvail().x) noexcept;

	};
}