#include "UI.h"
#include "Assets/AssetManager.h"

namespace Relentless
{
	namespace COLOR
	{
		constexpr const ImU32 SEARCHBAR_BACKGROUND = IM_COL32(17, 17, 17, 255);
		constexpr const ImU32 SEARCHBAR_BORDER = IM_COL32(60, 60, 60, 200);
		constexpr const ImU32 WIDGET_SELECTED = IM_COL32(30, 120, 255, 200);
		constexpr const ImU32 WIDGET_HOVERED = IM_COL32(100, 100, 100, 200);
	}

	struct GlobalData
	{
		AssetHandle SearchIconTextureHandle = NULL_HANDLE;
		AssetHandle CancelIconTextureHandle = NULL_HANDLE;
		AssetHandle ArrowDownIconTextureHandle = NULL_HANDLE;
	};

	static GlobalData s_GlobalData;

	std::string SearchBar::Draw(float width, const char* hintText, bool enableSearchHistory) noexcept
	{
		ImGui::PushID(this);

		ImGui::PushItemWidth(width);
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(ImGui::GetStyle().FramePadding.x, ImGui::GetStyle().FramePadding.y + 3.0f));

		const ImU32 borderColor = m_IsActive ? COLOR::WIDGET_SELECTED : m_IsHovered ? COLOR::WIDGET_HOVERED : COLOR::SEARCHBAR_BORDER;
		ImGui::PushStyleColor(ImGuiCol_Border, borderColor);
		ImGui::PushStyleColor(ImGuiCol_FrameBg, COLOR::SEARCHBAR_BACKGROUND);

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));


		const ImVec2 cursorPositionPreSearchBar = ImGui::GetCursorPos();
		DrawSearchBar(hintText, enableSearchHistory);
		const ImVec2 cursorPositionPostSearchBar = ImGui::GetCursorPos();

		ImGui::PopStyleVar(3);
		ImGui::PopItemWidth();

		if (!InputFieldContainsText())
		{
			ImGui::SetCursorPos(ImVec2(cursorPositionPreSearchBar.x + 6.0f, cursorPositionPreSearchBar.y + 6.0f));
			DrawSearchIcon();
		}
		else
		{
			ImGui::SetCursorPos(ImVec2(cursorPositionPreSearchBar.x + 9.0f, cursorPositionPreSearchBar.y + 12.0f));
			DrawCancelIcon();
		}

		if (enableSearchHistory)
		{
			ImGui::SetCursorPos(ImVec2(cursorPositionPreSearchBar.x + width - 26.0f, cursorPositionPreSearchBar.y + 9.0f));
			DrawSearchHistoryPopupIcon();

			ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x + cursorPositionPreSearchBar.x, ImGui::GetWindowPos().y + cursorPositionPostSearchBar.y + ImGui::GetStyle().ItemSpacing.y - 3.0f));
			ImGui::SetNextWindowSizeConstraints(ImVec2(width, 0), ImVec2(width, FLT_MAX));
			DrawSearchHistoryPopup();
		}
		
		ImGui::PopStyleColor(5);

		ImGui::PopID();
		return m_CallbackUserData.Input;
	}

	void SearchBar::DrawSearchBar(std::string_view hintText, bool shouldSaveToSearchHistory) noexcept
	{
		const ImVec2 cursorPositionPreSearchBar = ImGui::GetCursorPos();

		//We do custom hint solution below
		ImGui::InputTextWithHint("##SearchBar", nullptr, m_FullInputBuffer, IM_ARRAYSIZE(m_FullInputBuffer), ImGuiInputTextFlags_CallbackAlways,
			[](ImGuiInputTextCallbackData* data) -> int
			{
				constexpr const uint8_t paddingLength = 8u;

				for (uint8_t i = 0; i < paddingLength; ++i)
				{
					if (data->Buf[i] != ' ')
						data->InsertChars(i, " ");
				}

				if (data->CursorPos < paddingLength)
					data->CursorPos = paddingLength;

				const bool searchBarContainsText = paddingLength <= strlen(data->Buf);
				if (searchBarContainsText)
				{
					CallbackUserData* pUserData = static_cast<CallbackUserData*>(data->UserData);
					pUserData->Input = std::string(data->Buf + paddingLength);
				}

				return 0;
			}, &m_CallbackUserData);

		ImGui::SetItemAllowOverlap();
		m_IsActive = ImGui::IsItemActive();
		m_IsHovered = ImGui::IsItemHovered() && !m_CancelIconHovered && !m_ArrowDownIconHovered && !m_SearchIconHovered;
		const bool isFocused = ImGui::IsItemFocused() && m_IsActive;

		const ImVec2 inputTextWidgetSize = ImGui::GetItemRectSize();
		const ImVec2 inputTextWidgetStartPos = ImGui::GetItemRectMin();

		if (!InputFieldContainsText())
		{
			//Draw hint text:
			ImGui::SetCursorPos(ImVec2(cursorPositionPreSearchBar.x + 40.0f, cursorPositionPreSearchBar.y + 6.0f));
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
			ImGui::Text(hintText.data());
			ImGui::PopStyleVar();
		}

		const bool saveSearchHistory = shouldSaveToSearchHistory && InputFieldContainsText() &&m_WasFocused && !isFocused;
		m_WasFocused = isFocused;
		
		if (saveSearchHistory)
		{
			const bool previousEntryIsSame = m_SearchHistory.size() > 0 && m_SearchHistory[0] == m_CallbackUserData.Input;
			if (previousEntryIsSame)
				return;
			
			m_SearchHistory.insert(m_SearchHistory.begin(), m_CallbackUserData.Input);
			if (m_SearchHistory.size() > 5)
				m_SearchHistory.pop_back();
		}

	}

	void SearchBar::DrawSearchIcon() noexcept
	{
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

		const std::shared_ptr<Texture> texture = AssetManager::Get<Texture2D>(s_GlobalData.SearchIconTextureHandle);
		const ImVec4 tintCol = m_IsActive ? ImVec4(0.9f, 0.9f, 0.9f, 1.0f) : ImVec4(0.7f, 0.7f, 0.7f, 1.0f);
		ImGui::ImageButton((ImTextureID)texture->GetSRVDescriptorHandle().GPUHandle.ptr, ImVec2(24.0f, 24.0f), ImVec2(0, 0), ImVec2(1, 1), -1, ImVec4(0, 0, 0, 0), tintCol);
		
		m_SearchIconHovered = ImGui::IsItemHovered();
		if (m_SearchIconHovered)
			ImGui::SetMouseCursor(ImGuiMouseCursor_::ImGuiMouseCursor_Arrow);

		ImGui::PopStyleVar();
	}

	void SearchBar::DrawCancelIcon() noexcept
	{
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

		const std::shared_ptr<Texture> texture = AssetManager::Get<Texture2D>(s_GlobalData.CancelIconTextureHandle);
		const ImVec4 tintCol = m_CancelIconHovered ? ImVec4(1.0f, 1.0f, 1.0f, 1.0f) : ImVec4(0.7f, 0.7f, 0.7f, 1.0f);
		ImGui::ImageButton((ImTextureID)texture->GetSRVDescriptorHandle().GPUHandle.ptr, ImVec2(15.0f, 15.0f), ImVec2(0, 0), ImVec2(1, 1), -1, ImVec4(0, 0, 0, 0), tintCol);

		m_CancelIconHovered = ImGui::IsItemHovered();
		if (m_CancelIconHovered)
			ImGui::SetMouseCursor(ImGuiMouseCursor_::ImGuiMouseCursor_Arrow);

		if (ImGui::IsItemClicked())
		{
			memset(m_FullInputBuffer, 0, sizeof(m_FullInputBuffer));
			m_CallbackUserData.Input.clear();
			m_CancelIconHovered = false;
		}

		ImGui::PopStyleVar();
	}

	void SearchBar::DrawSearchHistoryPopupIcon() noexcept
	{
		const ImVec4 tintCol = m_ArrowDownIconHovered ? ImVec4(1.0f, 1.0f, 1.0f, 1.0f) : ImVec4(0.7f, 0.7f, 0.7f, 1.0f);
		const std::shared_ptr<Texture2D> texture = AssetManager::Get<Texture2D>(s_GlobalData.ArrowDownIconTextureHandle);

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

		ImGui::ImageButton((ImTextureID)texture->GetSRVDescriptorHandle().GPUHandle.ptr, ImVec2(17.0f, 17.0f), ImVec2(0, 0), ImVec2(1, 1), -1, ImVec4(0, 0, 0, 0), tintCol);

		bool shouldOpenNew = false;
		if (ImGui::IsItemClicked())
		{
			shouldOpenNew = true;
			ImGui::OpenPopup("SearchHistoryPopup");
		}

		m_ArrowDownIconHovered = ImGui::IsItemHovered();
		if (m_ArrowDownIconHovered)
		{
			ImGui::SetMouseCursor(ImGuiMouseCursor_::ImGuiMouseCursor_Arrow);
			ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(240.0f / 255.0f, 240.0f / 255.0f, 240.0f / 255.0f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
			ImGui::SetTooltip("Click to show the Search History");
			ImGui::PopStyleColor(2);
		}

		ImGui::PopStyleVar();
	}

	void SearchBar::DrawSearchHistoryPopup() noexcept
	{
		if (!ImGui::BeginPopup("SearchHistoryPopup"))
			return;
			
		if (m_SearchHistory.empty())
		{
			ImGui::MenuItem("The Search History Is Empty");
		}
		else
		{
			for (uint8_t i{ 0u }; i < m_SearchHistory.size(); ++i)
			{
				if (ImGui::MenuItem(m_SearchHistory[i].c_str()))
				{
					m_CallbackUserData.Input = m_SearchHistory[i];
					ResetInputBuffer();
					std::memcpy(m_FullInputBuffer + 8, m_CallbackUserData.Input.c_str(), m_CallbackUserData.Input.length());
				}
			}
		}
		ImGui::EndPopup();
	}

	bool SearchBar::InputFieldContainsText() const noexcept
	{
		return strlen(m_FullInputBuffer) > 8;
	}

	void SearchBar::ResetInputBuffer() noexcept
	{
		memset(m_FullInputBuffer, 0, 128);
		for (uint8_t i{ 0u }; i < 8; ++i)
		{
			m_FullInputBuffer[i] = ' ';
		}
	}

	struct UIElements
	{
		std::unordered_map<const char*, SearchBar> SearchBars;
	};

	static UIElements s_UIElements;

	void UI::Initialize() noexcept
	{
		RLS_VERIFY(AssetManager::RequestLoadAsset(std::string(ENGINE_ASSET_DIRECTORY) + "Textures\\Icons\\searchicon.rasset", s_GlobalData.SearchIconTextureHandle), "Core engine icon missing.");
		RLS_VERIFY(AssetManager::RequestLoadAsset(std::string(ENGINE_ASSET_DIRECTORY) + "Textures\\Icons\\cancelicon.rasset", s_GlobalData.CancelIconTextureHandle), "Core engine icon missing.");
		RLS_VERIFY(AssetManager::RequestLoadAsset(std::string(ENGINE_ASSET_DIRECTORY) + "Textures\\Icons\\arrowdownicon.rasset", s_GlobalData.ArrowDownIconTextureHandle), "Core engine icon missing.");
	}

	std::string UI::SearchBar(const char* uniqueID, const char* hintText, bool displaySearchHistory, float width) noexcept
	{
		return s_UIElements.SearchBars[uniqueID].Draw(width, hintText, displaySearchHistory);
	}

	std::string UI::Utility::ShortenStringToFitClipRect(const std::string& originalString, const ImVec2& topLeft, const ImVec2& bottomRight) noexcept
	{
		// Calculate available width from the provided clip rect
		const float availableHeight = bottomRight.y - topLeft.y;

		// Measure the original string
		const ImVec2 textSize = ImGui::CalcTextSize(originalString.c_str());
		const uint32_t possibleNrOfRows = std::floor(availableHeight / textSize.y);
		const float availableWidth = (bottomRight.x - topLeft.x) * possibleNrOfRows;

		// If the string fits within the available width, no need to shorten
		if (textSize.x <= availableWidth) 
		{
			return originalString;
		}

		constexpr const char* ellipsis = "...";
		const ImVec2 ellipsisSize = ImGui::CalcTextSize(ellipsis);

		// Ensure available width can at least fit the ellipsis
		if (availableWidth <= ellipsisSize.x) 
		{
			// If available width is too small even for ellipsis, return as much as we can fit
			return "";
		}

		std::string modified = originalString;
		// Iteratively remove characters until the string plus ellipsis fits
		while (!modified.empty() && (ImGui::CalcTextSize((modified + ellipsis).c_str()).x > availableWidth)) 
		{
			modified.pop_back(); // Remove one character from the end
		}

		return modified + ellipsis;
	}

	void UI::Utility::DrawTitledSeparator(const std::string& title, const ImVec2& begin, const ImVec2& end) noexcept
	{
		ImDrawList* pDrawList = ImGui::GetWindowDrawList();
		if (!pDrawList)
			return;
		
		const float windowWidth = ImGui::GetWindowWidth();

		ImGui::SetWindowFontScale(0.8f);
		const ImVec2 textSize = ImGui::CalcTextSize(title.c_str());

		constexpr ImU32 textColor = IM_COL32(255.0f, 255.0f, 255.0f, 128.0f);
		pDrawList->AddText(ImVec2(begin.x + 10.0f, begin.y - (textSize.y / 2.0f)), textColor, title.c_str());
		
		ImGui::SetWindowFontScale(1.0f);

		float remainingWidth = windowWidth - textSize.x;

		ImGui::SetCursorScreenPos(ImVec2(begin.x + 10.0f + textSize.x + 20.0f, begin.y));

		constexpr const ImVec4 splitterColor = ImVec4(200.0f / 255, 200.0f / 255, 200.0f / 255, 128.0f / 255.0f);
		ImGui::PushStyleColor(ImGuiCol_Button, splitterColor);
		
		constexpr float splitterThickness = 1.0f;
		ImGui::Button("##Splitter2", ImVec2(remainingWidth - 30.0f - 15.0f, splitterThickness));
		
		ImGui::PopStyleColor();
	}

	void UI::Utility::DrawTooltip(const char* tooltip) noexcept
	{
		ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(240.0f / 255.0f, 240.0f / 255.0f, 240.0f / 255.0f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
		ImGui::SetTooltip(tooltip);
		ImGui::PopStyleColor(2);
	}

}