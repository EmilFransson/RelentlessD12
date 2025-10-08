#include "SearchBar.h"

#include "Graphics/RHI/ResourceViews.h"
#include "Input/Keyboard.h"

namespace Relentless
{
	namespace SearchBarEx_private
	{
		constexpr const Color SearchbarBackgroundColor	= Color(17.0f, 17.0f, 17.0f, 255.0f);
		constexpr const Color SearchbarBorderColor		= Color(60.0f, 60.0f, 60.0f, 200.0f);
		constexpr const Color SearchbarActiveColor		= Color(30.0f, 120.0f, 255.0f, 200.0f);
		constexpr const Color SearchbarHoveredColor		= Color(100.0f, 100.0f, 100.0f, 200.0f);
	}

	SearchBar::SearchBar(std::string_view hintText, bool enableSearchHistory) noexcept
		:m_HintText{hintText}
		,m_EnableSearchHistory{ enableSearchHistory }
	{
		SetFrameRounding(10.0f);
		SetBorderSize(2.0f);
		SetPadding({ ImGui::GetStyle().FramePadding.x, ImGui::GetStyle().FramePadding.y + 3.0f });
		
		SetBackgroundColor(Colors::Normalize(17.0f, 17.0f, 17.0f, 255.0f));
		SetBorderColor(Colors::Normalize(SearchBarEx_private::SearchbarBackgroundColor));
	}

	float SearchBar::CalcDesiredWidth() const noexcept
	{
		return 0.0f;
	}

	void SearchBar::OnRender() noexcept
	{
		const Color borderCol = m_IsActive ? SearchBarEx_private::SearchbarActiveColor : m_IsHovered ? SearchBarEx_private::SearchbarHoveredColor : SearchBarEx_private::SearchbarBorderColor;
		SetBorderColor(Colors::Normalize(borderCol));

		const ImVec2 cursorStartPosition = ImGui::GetCursorPos();
		DrawSearchBar();

		if (!InputFieldContainsText())
			DrawSearchIcon(cursorStartPosition);
		else
			DrawCancelIcon(cursorStartPosition);
		
		if (m_EnableSearchHistory)
		{
			DrawSearchHistoryPopupIcon(cursorStartPosition);

			if (ImGui::IsPopupOpen("SearchHistoryPopup"))
				DrawSearchHistoryPopup(cursorStartPosition);
		}
	}

	void SearchBar::DrawSearchBar() noexcept
	{
		const ImVec2 cursorPositionPreSearchBar = ImGui::GetCursorPos();

		//We do custom hint solution below
		ImGui::SetNextItemWidth(-100.0f);
		const bool inputDone = ImGui::InputTextWithHint("##SearchBar", nullptr, m_FullInputBuffer, IM_ARRAYSIZE(m_FullInputBuffer), ImGuiInputTextFlags_CallbackAlways,
			[](ImGuiInputTextCallbackData* data) -> int
			{
				constexpr const uint8 paddingLength = 8u;

				CallbackUserData* pUserData = static_cast<CallbackUserData*>(data->UserData);
				
				bool clearedInputThisFrame = pUserData->ClearedInput;
				if (pUserData->ClearedInput)
				{
					data->DeleteChars(0, strlen(data->Buf));
					pUserData->ClearedInput = false;
				}

				for (uint8 i = 0; i < paddingLength; ++i)
				{
					if (data->Buf[i] != ' ')
						data->InsertChars(i, " ");
				}

				if (data->CursorPos < paddingLength)
					data->CursorPos = paddingLength;

				const bool searchBarContainsText = !clearedInputThisFrame && paddingLength <= strlen(data->Buf);
				
				if (searchBarContainsText)
					pUserData->Input = std::string(data->Buf + paddingLength);

				return 0;
			}, &m_CallbackUserData);

		const ImVec2 cursorPos = ImGui::GetCursorPos();

		if (inputDone && m_OnTextChanged.IsSet())
			m_OnTextChanged(m_CallbackUserData.Input.c_str());

		if (ImGui::IsItemDeactivatedAfterEdit() && m_OnTextCommitted.IsSet())
		{
			ETextCommitType type = Keyboard::IsKeyDown(RLS_Key::Enter) ? ETextCommitType::OnEnter : ETextCommitType::OnUserMovedFocus;
			m_OnTextCommitted(m_CallbackUserData.Input.c_str(), type);
		}

		m_Size = Vector2(ImGui::GetItemRectSize().x, ImGui::GetItemRectSize().y);

		ImGui::SetItemAllowOverlap();
		m_IsActive = ImGui::IsItemActive();
		m_IsHovered = ImGui::IsItemHovered() && !m_CancelIconHovered && !m_ChevronIconHovered && !m_MagnifyingGlassIconHovered;
		const bool isFocused = ImGui::IsItemFocused() && m_IsActive;

		const ImVec2 inputTextWidgetSize = ImGui::GetItemRectSize();
		const ImVec2 inputTextWidgetStartPos = ImGui::GetItemRectMin();

		if (!InputFieldContainsText())
		{
			//Draw hint text:
			ImGui::SetCursorPos(ImVec2(cursorPositionPreSearchBar.x + 40.0f, cursorPositionPreSearchBar.y + 6.0f));
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
			ImGui::Text(m_HintText.c_str());
			ImGui::PopStyleVar();
		}

		const bool saveSearchHistory = m_EnableSearchHistory && InputFieldContainsText() && m_WasFocused && !isFocused;
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

		ImGui::SetCursorPos(cursorPos);
	}

	void SearchBar::DrawSearchIcon(const ImVec2& searchBarStartPos) noexcept
	{
		const ImVec2 magnifyingGlassSize = ImGui::CalcTextSize(ICON_FA_MAGNIFYING_GLASS);
		constexpr float margin = 5.0f;
		const float magnifyingGlassXPosition = searchBarStartPos.x + margin;
		const float magnifyingGlassYPosition = searchBarStartPos.y + ((m_Size.y - magnifyingGlassSize.y) * 0.5f);

		ImGui::SetCursorPos(ImVec2(magnifyingGlassXPosition, magnifyingGlassYPosition));
		ImGui::TextColored(m_IsActive ? ImVec4(0.9f, 0.9f, 0.9f, 1.0f) : ImVec4(0.7f, 0.7f, 0.7f, 1.0f), ICON_FA_MAGNIFYING_GLASS);

		m_MagnifyingGlassIconHovered = ImGui::IsItemHovered();
		if (m_MagnifyingGlassIconHovered)
			ImGui::SetMouseCursor(ImGuiMouseCursor_::ImGuiMouseCursor_Arrow);
	}

	void SearchBar::DrawCancelIcon(const ImVec2& searchBarStartPos) noexcept
	{
		const ImVec2 xMarkSize = ImGui::CalcTextSize(ICON_FA_XMARK);
		constexpr float margin = 5.0f;
		const float xMarkXPosition = searchBarStartPos.x + margin;
		const float xMarkYPosition = searchBarStartPos.y + ((m_Size.y - xMarkSize.y) * 0.5f);

		ImGui::SetCursorPos(ImVec2(xMarkXPosition, xMarkYPosition));
		ImGui::TextColored(m_CancelIconHovered ? ImVec4(0.9f, 0.9f, 0.9f, 1.0f) : ImVec4(0.7f, 0.7f, 0.7f, 1.0f), ICON_FA_XMARK);

		m_CancelIconHovered = ImGui::IsItemHovered();
		if (m_CancelIconHovered)
			ImGui::SetMouseCursor(ImGuiMouseCursor_::ImGuiMouseCursor_Arrow);

		if (ImGui::IsItemClicked())
		{
			memset(m_FullInputBuffer, 0, sizeof(m_FullInputBuffer));
			m_CallbackUserData.Input.clear();
			m_CancelIconHovered = false;
			m_CallbackUserData.ClearedInput = true;
		}
	}

	void SearchBar::DrawSearchHistoryPopupIcon(const ImVec2& searchBarStartPos) noexcept
	{
		const ImVec2 chevronSize = ImGui::CalcTextSize(ICON_FA_CHEVRON_DOWN);
		constexpr float margin = 5.0f;
		const float chevronXPosition = searchBarStartPos.x + m_Size.x - chevronSize.x - margin;
		const float chevronYPosition = searchBarStartPos.y + ((m_Size.y - chevronSize.y) * 0.5f);

		ImGui::SetCursorPos(ImVec2(chevronXPosition, chevronYPosition));
		ImGui::TextColored(m_ChevronIconHovered ? ImVec4(0.9f, 0.9f, 0.9f, 1.0f) : ImVec4(0.7f, 0.7f, 0.7f, 1.0f), ICON_FA_CHEVRON_DOWN);
		m_ChevronIconHovered = ImGui::IsItemHovered();

		if (ImGui::IsItemClicked())
			ImGui::OpenPopup("SearchHistoryPopup");

		if (m_ChevronIconHovered)
		{
			ImGui::SetMouseCursor(ImGuiMouseCursor_::ImGuiMouseCursor_Arrow);

			constexpr ImVec4 popupBGColor = ImVec4(240.0f / 255.0f, 240.0f / 255.0f, 240.0f / 255.0f, 1.0f);
			ImGui::PushStyleColor(ImGuiCol_PopupBg, popupBGColor);
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
			ImGui::SetTooltip("Click to show the Search History");
			ImGui::PopStyleColor(2);
		}
	}

	void SearchBar::DrawSearchHistoryPopup(const ImVec2& searchBarStartPos) noexcept
	{
		ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x + searchBarStartPos.x, ImGui::GetWindowPos().y + searchBarStartPos.y + m_Size.y));
		ImGui::SetNextWindowSizeConstraints(ImVec2(m_Size.x, 0), ImVec2(m_Size.x, FLT_MAX));

		if (!ImGui::BeginPopup("SearchHistoryPopup"))
			return;

		if (m_SearchHistory.empty())
			ImGui::MenuItem("The Search History Is Empty");
		else
		{
			for (uint8 i{ 0u }; i < m_SearchHistory.size(); ++i)
			{
				if (ImGui::MenuItem(m_SearchHistory[i].c_str()))
				{
					m_CallbackUserData.Input = m_SearchHistory[i];
					ResetInputBuffer();
					std::memcpy(m_FullInputBuffer + 8, m_CallbackUserData.Input.c_str(), m_CallbackUserData.Input.length());
					m_OnTextChanged.ExecuteIfSet(m_CallbackUserData.Input.c_str());
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
			m_FullInputBuffer[i] = ' ';
	}
}
