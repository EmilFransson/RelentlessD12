#pragma once
#include "IWidget.h"
#include "Callback/Callback.h"

namespace Relentless
{
	class ComboBox : public IStylableWidget<ComboBox>
	{
	public:
		ComboBox(int flags = 0) noexcept;

		struct SelectionInfo
		{
			const char* Name = nullptr;
			int Index = 0;
		};
		
		ComboBox* AddSelectables(Span<const char*> selectables) noexcept;
		virtual [[nodiscard]] float CalcDesiredWidth() const noexcept override;
		[[nodiscard]] const char* GetSelectedItem() const;
		[[nodiscard]] int GetSelectedIndex() const;

		template<typename InstanceType>
		ComboBox* OnSelectionChanged(InstanceType* instance, void(InstanceType::* method)(const SelectionInfo&)) noexcept
		{
			m_OnSelectionChanged = [instance, method](const SelectionInfo& aSelection) { return (instance->*method)(aSelection); };
			return this;
		}

		template<typename T>
		ComboBox* OnSelectionChanged(T&& callback) noexcept
		{
			m_OnSelectionChanged = Callback<void(const SelectionInfo&)>(std::forward<T>(callback));
			return this;
		}

		NO_DISCARD virtual Vector2 ReportSize() const noexcept override;

		void SetDropDownButtonColor(const Color& color) noexcept;
		void SetDropDownButtonActiveColor(const Color& color) noexcept;
		void SetDropDownButtonHoveredColor(const Color& color) noexcept;

		ComboBox* SetSelectedItem(const char* pItem) noexcept;
		ComboBox* SetSelectedItem(int aIndex) noexcept;

		void SetSelectableBackgroundColor(const Color& color) noexcept;

	protected:
		virtual void OnPreRender() noexcept override;
		virtual void OnRender() noexcept override;
	private:
		Callback<void(const SelectionInfo&)> m_OnSelectionChanged;

		std::vector<const char*> m_Selectables;
		SelectionInfo m_CurrentSelection;
		bool m_IsHovered = false;
	};
}