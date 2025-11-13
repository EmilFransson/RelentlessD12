#pragma once
#include "IWidget.h"
#include "Callback/Callback.h"

namespace Relentless
{
	class ComboBox : public IStylableWidget<ComboBox>
	{
	public:
		ComboBox(int flags = 0) noexcept;

		ComboBox* AddSelectables(Span<const char*> selectables) noexcept;
		virtual [[nodiscard]] float CalcDesiredWidth() const noexcept override;
		[[nodiscard]] const char* GetSelectedItem() const;
		[[nodiscard]] int GetSelectedIndex() const;

		template<typename InstanceType>
		ComboBox* OnSelectionChanged(InstanceType* instance, void(InstanceType::* method)(const char*)) noexcept
		{
			m_OnSelectionChanged = [instance, method](const char* selected) { return (instance->*method)(selected); };
			return this;
		}

		template<typename T>
		ComboBox* OnSelectionChanged(T&& callback) noexcept
		{
			m_OnSelectionChanged = Callback<void(const char*)>(std::forward<T>(callback));
			return this;
		}

		NO_DISCARD virtual Vector2 ReportSize() const noexcept override;

		void SetDropDownButtonColor(const Color& color) noexcept;
		void SetDropDownButtonActiveColor(const Color& color) noexcept;
		void SetDropDownButtonHoveredColor(const Color& color) noexcept;

		ComboBox* SetInitiallySelectedItem(const char* pItem) noexcept;

		void SetSelectableBackgroundColor(const Color& color) noexcept;

	protected:
		virtual void OnPreRender() noexcept override;
		virtual void OnRender() noexcept override;
	private:
		Callback<void(const char* selected)> m_OnSelectionChanged;

		std::vector<const char*> m_Selectables;
		int m_Selected = 0;
		bool m_IsHovered = false;
	};
}