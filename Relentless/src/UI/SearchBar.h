#pragma once
#include "Callback/Callback.h"
#include "IWidget.h"

namespace Relentless
{
	enum class ETextCommitType : uint8 { OnEnter = 0u, OnUserMovedFocus, OnCleared };

	class SearchBarEx : public IStylableWidget
	{
	public:
		SearchBarEx(std::string_view id, std::string_view hintText = "", bool enableSearchHistory = false) noexcept;

		virtual [[nodiscard]] float CalcDesiredWidth() const noexcept override;

		template<typename Func>
		SearchBarEx* OnTextChanged(Func&& callback) noexcept;

		template<typename Func>
		SearchBarEx* OnTextCommitted(Func&& callback) noexcept;

	private:
		virtual void OnRender() noexcept override;

		void DrawSearchBar() noexcept;
		void DrawSearchIcon() noexcept;
		void DrawCancelIcon() noexcept;
		void DrawSearchHistoryPopupIcon() noexcept;
		void DrawSearchHistoryPopup() noexcept;
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
			SearchBarEx* OwningObject = nullptr;
		} m_CallbackUserData;

		float m_Width = 0.0f;

		char m_FullInputBuffer[128] = "        ";
		bool m_IsActive = false;
		bool m_IsHovered = false;
		bool m_SearchIconHovered = false;
		bool m_CancelIconHovered = false;
		bool m_ArrowDownIconHovered = false;
		bool m_WasFocused = false;

		bool m_EnableSearchHistory = false;
	};

	template<typename Func>
	SearchBarEx* SearchBarEx::OnTextChanged(Func&& callback) noexcept
	{
		m_OnTextChanged = std::move(callback);
	}

	template<typename Func>
	SearchBarEx* SearchBarEx::OnTextCommitted(Func&& callback) noexcept
	{
		m_OnTextCommitted = callback;
	}
}