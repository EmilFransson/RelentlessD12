#include "SearchBar.h"

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

		ImGui::PopClipRect();
	}

	void SearchBar::DrawSearchBar() noexcept
	{
		const ImVec2 cursorPositionPreSearchBar = ImGui::GetCursorPos();

		//We do custom hint solution below
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
		m_AreaMin = Vector2(ImGui::GetItemRectMin().x, ImGui::GetItemRectMin().y);
		m_AreaMax = Vector2(ImGui::GetItemRectMax().x, ImGui::GetItemRectMax().y);

		const ImVec2 itemMin = ImVec2(m_AreaMin.x, m_AreaMin.y);
		const ImVec2 itemMax = ImVec2(m_AreaMax.x, m_AreaMax.y);
		ImGui::PushClipRect(itemMin, itemMax, true);

		ImGui::SetItemAllowOverlap();
		m_IsActive = ImGui::IsItemActive();
		m_IsHovered = ImGui::IsItemHovered() && !m_CancelIconHovered && !m_ChevronIconHovered && !m_MagnifyingGlassIconHovered;
		const bool isFocused = ImGui::IsItemFocused() && m_IsActive;

		if (!InputFieldContainsText())
		{
			//Draw hint text:
			ImGui::SetCursorPos(ImVec2(cursorPositionPreSearchBar.x + 40.0f, cursorPositionPreSearchBar.y + 6.0f));
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
			ImGui::Text("%s", m_HintText.c_str());
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

	Vector2 SearchBar::ReportSize() const noexcept
	{
		ImFont* pFont = GetStyle().GetFont();
		if (pFont) 
			ImGui::PushFont(pFont);

		const Vector2 padding = GetPadding() * 2.0f;
		const ImGuiStyle& style = ImGui::GetStyle();
		const float frameHeight = ImGui::GetFontSize() + padding.y;

		// Icon glyph sizes (Font Awesome)
		const ImVec2 magSize = ImGui::CalcTextSize(ICON_FA_MAGNIFYING_GLASS);
		const ImVec2 cancelSize = ImGui::CalcTextSize(ICON_FA_XMARK);
		const ImVec2 chevronSize = ImGui::CalcTextSize(ICON_FA_CHEVRON_DOWN);

		// Your hardcoded placements
		constexpr float iconMargin = 5.0f; // you use this on both sides
		const float inner = style.ItemInnerSpacing.x;

		// Inside-frame occlusions
		const float leftOcclusion = iconMargin + Math::Max(magSize.x, cancelSize.x);
		const float rightOcclusion = m_EnableSearchHistory ? (inner + chevronSize.x + iconMargin) : 0.0f;

		// Text budget: hint width OR N average glyphs so it doesn’t collapse
		const float hintW = m_HintText.empty() ? 0.0f : ImGui::CalcTextSize(m_HintText.c_str()).x;
		const int   minChars = 12;
		const float avgW = ImGui::CalcTextSize("M").x;
		const float desiredTextW = Math::Max(hintW, avgW * (float)minChars);

		// Preferred overall width = occlusions + desired text width
		float width = leftOcclusion + desiredTextW + rightOcclusion;
		float height = Math::Max(frameHeight, Math::Max(magSize.y, Math::Max(cancelSize.y, chevronSize.y)));

		// If you expose an explicit size, let it clamp upward (never smaller than preferred)
		if (m_Size.x > 0.0f) 
			width = Math::Max(width, m_Size.x);
		if (m_Size.y > 0.0f) 
			height = Math::Max(height, m_Size.y);

		if (pFont) 
			ImGui::PopFont();

		return { width, height };
	}
}
