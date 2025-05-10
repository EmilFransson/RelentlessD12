#include "SearchBar.h"

#include "Assets/AssetManager.h"
#include "Graphics/RHI/ResourceViews.h"
#include "Input/Keyboard.h"
#include "UI.h"

namespace Relentless
{
	namespace COLOR
	{
		constexpr const Color SEARCHBAR_BACKGROUND = Color(17, 17, 17, 255);
		constexpr const Color SEARCHBAR_BORDER = Color(60, 60, 60, 200);
		constexpr const Color WIDGET_SELECTED = Color(30, 120, 255, 200);
		constexpr const Color WIDGET_HOVERED = Color(100, 100, 100, 200);
	}

	SearchBarEx::SearchBarEx(std::string_view id, std::string_view hintText, bool enableSearchHistory) noexcept
		:IStylableWidget{id}
		,m_HintText{hintText}
		,m_EnableSearchHistory{ enableSearchHistory }
	{
		SetFrameRounding(10.0f);
		SetBorderSize(2.0f);
		SetPadding({ ImGui::GetStyle().FramePadding.x, ImGui::GetStyle().FramePadding.y + 3.0f });
		
		SetBackgroundColor(Colors::Normalize(17.0f, 17.0f, 17.0f, 255.0f));
		SetBorderColor(Colors::Normalize(COLOR::SEARCHBAR_BORDER));
	}

	float SearchBarEx::CalcDesiredWidth() const noexcept
	{
		return 0.0f;
	}

	void SearchBarEx::OnRender() noexcept
	{
		//ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
		//ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
		//ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
		//ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(ImGui::GetStyle().FramePadding.x, ImGui::GetStyle().FramePadding.y + 3.0f));

		const Color borderCol = m_IsActive ? COLOR::WIDGET_SELECTED : m_IsHovered ? COLOR::WIDGET_HOVERED : COLOR::SEARCHBAR_BORDER;
		SetBorderColor(Colors::Normalize(borderCol));


		//const ImU32 borderColor = m_IsActive ? COLOR::WIDGET_SELECTED : m_IsHovered ? COLOR::WIDGET_HOVERED : COLOR::SEARCHBAR_BORDER;
		//ImGui::PushStyleColor(ImGuiCol_Border, borderColor);
		//ImGui::PushStyleColor(ImGuiCol_FrameBg, COLOR::SEARCHBAR_BACKGROUND);

		//ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
		//ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
		//ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

		const ImVec2 cursorPositionPreSearchBar = ImGui::GetCursorPos();
		DrawSearchBar();
		const ImVec2 cursorPositionPostSearchBar = ImGui::GetCursorPos();

		//ImGui::PopStyleVar(3);
		//ImGui::PopItemWidth();

		//if (!InputFieldContainsText())
		//{
		//	ImGui::SetCursorPos(ImVec2(cursorPositionPreSearchBar.x + 6.0f, cursorPositionPreSearchBar.y + 6.0f));
		//	DrawSearchIcon();
		//}
		//else
		//{
		//	ImGui::SetCursorPos(ImVec2(cursorPositionPreSearchBar.x + 9.0f, cursorPositionPreSearchBar.y + 12.0f));
		//	DrawCancelIcon();
		//}
		//
		//if (m_EnableSearchHistory)
		//{
		//	ImGui::SetCursorPos(ImVec2(cursorPositionPreSearchBar.x + m_Width - 26.0f, cursorPositionPreSearchBar.y + 9.0f));
		//	DrawSearchHistoryPopupIcon();
		//
		//	ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x + cursorPositionPreSearchBar.x, ImGui::GetWindowPos().y + cursorPositionPostSearchBar.y + ImGui::GetStyle().ItemSpacing.y - 3.0f));
		//	ImGui::SetNextWindowSizeConstraints(ImVec2(m_Width, 0), ImVec2(m_Width, FLT_MAX));
		//	DrawSearchHistoryPopup();
		//}

		//ImGui::PopStyleColor(5);
	}

	void SearchBarEx::DrawSearchBar() noexcept
	{
		const ImVec2 cursorPositionPreSearchBar = ImGui::GetCursorPos();

		//We do custom hint solution below
		const bool inputDone = ImGui::InputTextWithHint("##SearchBar", nullptr, m_FullInputBuffer, IM_ARRAYSIZE(m_FullInputBuffer), ImGuiInputTextFlags_CallbackAlways,
			[](ImGuiInputTextCallbackData* data) -> int
			{
				constexpr const uint8 paddingLength = 8u;

				for (uint8 i = 0; i < paddingLength; ++i)
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

		if (inputDone && m_OnTextChanged.IsSet())
			m_OnTextChanged(m_CallbackUserData.Input.c_str());

		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			ETextCommitType type = Keyboard::IsKeyDown(RLS_Key::Enter) ? ETextCommitType::OnEnter : ETextCommitType::OnUserMovedFocus;
			m_OnTextCommitted(m_CallbackUserData.Input.c_str(), type);
		}

		m_Width = ImGui::GetItemRectSize().x;

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
	}

	void SearchBarEx::DrawSearchIcon() noexcept
	{
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

		const Ref<Texture> texture = AssetManager::Get<Texture>(UI::SearchIconTextureHandle);
		const ImVec4 tintCol = m_IsActive ? ImVec4(0.9f, 0.9f, 0.9f, 1.0f) : ImVec4(0.7f, 0.7f, 0.7f, 1.0f);
		ImGui::ImageButton((ImTextureID)texture->GetSRV()->GetGPUHandle().ptr, ImVec2(24.0f, 24.0f), ImVec2(0, 0), ImVec2(1, 1), -1, ImVec4(0, 0, 0, 0), tintCol);

		m_SearchIconHovered = ImGui::IsItemHovered();
		if (m_SearchIconHovered)
			ImGui::SetMouseCursor(ImGuiMouseCursor_::ImGuiMouseCursor_Arrow);

		ImGui::PopStyleVar();
	}

	void SearchBarEx::DrawCancelIcon() noexcept
	{
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

		const Ref<Texture> texture = AssetManager::Get<Texture>(UI::CancelIconTextureHandle);
		const ImVec4 tintCol = m_CancelIconHovered ? ImVec4(1.0f, 1.0f, 1.0f, 1.0f) : ImVec4(0.7f, 0.7f, 0.7f, 1.0f);
		ImGui::ImageButton((ImTextureID)texture->GetSRV()->GetGPUHandle().ptr, ImVec2(15.0f, 15.0f), ImVec2(0, 0), ImVec2(1, 1), -1, ImVec4(0, 0, 0, 0), tintCol);

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

	void SearchBarEx::DrawSearchHistoryPopupIcon() noexcept
	{
		const ImVec4 tintCol = m_ArrowDownIconHovered ? ImVec4(1.0f, 1.0f, 1.0f, 1.0f) : ImVec4(0.7f, 0.7f, 0.7f, 1.0f);
		const Ref<Texture> pTexture = AssetManager::Get<Texture>(UI::ArrowDownIconTextureHandle);

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

		ImGui::ImageButton((ImTextureID)pTexture->GetSRV()->GetGPUHandle().ptr, ImVec2(17.0f, 17.0f), ImVec2(0, 0), ImVec2(1, 1), -1, ImVec4(0, 0, 0, 0), tintCol);

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

	void SearchBarEx::DrawSearchHistoryPopup() noexcept
	{
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
				}
			}
		}
		ImGui::EndPopup();
	}

	bool SearchBarEx::InputFieldContainsText() const noexcept
	{
		return strlen(m_FullInputBuffer) > 8;
	}

	void SearchBarEx::ResetInputBuffer() noexcept
	{
		memset(m_FullInputBuffer, 0, 128);

		for (uint8_t i{ 0u }; i < 8; ++i)
			m_FullInputBuffer[i] = ' ';
	}
}
