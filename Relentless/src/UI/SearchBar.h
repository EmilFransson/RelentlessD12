#pragma once
#include "Callback/Callback.h"
#include "IWidget.h"

namespace Relentless
{
	class SearchBar : public IStylableWidget<SearchBar>
	{
	public:
		SearchBar(std::string_view hintText = "", bool enableSearchHistory = false) noexcept;

		virtual [[nodiscard]] float CalcDesiredWidth() const noexcept override;

		template<typename Func>
		SearchBar* OnTextChanged(Func&& callback) noexcept
		{
			m_OnTextChanged = std::move(callback);
			return this;
		}

		template<typename InstanceType>
		SearchBar* OnTextChanged(InstanceType* instance, void(InstanceType::*method)(const char*)) noexcept
		{
			m_OnTextChanged = [instance, method](const char* pText) { return (instance->*method)(pText); };
			return this;
		}

		template<typename Func>
		SearchBar* OnTextCommitted(Func&& callback) noexcept
		{
			m_OnTextCommitted = callback;
			return this;
		}

		template<typename InstanceType>
		SearchBar* OnTextCommitted(InstanceType* instance, void(InstanceType::*method)(const char*, ETextCommitType)) noexcept
		{
			m_OnTextCommitted = [instance, method](const char* pText, ETextCommitType commitType) { return (instance->*method)(pText, commitType); };
			return this;
		}

		NO_DISCARD Vector2 ReportSize() const noexcept override;

	private:
		virtual void OnRender() noexcept override;

		void DrawSearchBar() noexcept;
		void DrawSearchIcon(const ImVec2& searchBarStartPos) noexcept;
		void DrawCancelIcon(const ImVec2& searchBarStartPos) noexcept;
		void DrawSearchHistoryPopupIcon(const ImVec2& searchBarStartPos) noexcept;
		void DrawSearchHistoryPopup(const ImVec2& searchBarStartPos) noexcept;
		[[nodiscard]] bool InputFieldContainsText() const noexcept;
		void ResetInputBuffer() noexcept;
	private:
		std::vector<String> m_SearchHistory;
		String m_HintText{};

		Callback<void(const char*)> m_OnTextChanged;
		Callback<void(const char* pText, ETextCommitType commitType)> m_OnTextCommitted;

		struct CallbackUserData
		{
			std::string Input;
			SearchBar* OwningObject = nullptr;
			bool ClearedInput = false;
		} m_CallbackUserData;

		Vector2 m_Size = Vector2::Zero;
		Vector2 m_AreaMin = Vector2::Zero;
		Vector2 m_AreaMax = Vector2::Zero;

		char m_FullInputBuffer[128] = "        ";
		bool m_IsActive = false;
		bool m_IsHovered = false;
		bool m_MagnifyingGlassIconHovered = false;
		bool m_CancelIconHovered = false;
		bool m_ChevronIconHovered = false;
		bool m_WasFocused = false;

		bool m_EnableSearchHistory = false;
	};
}