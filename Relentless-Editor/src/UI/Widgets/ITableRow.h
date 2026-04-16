#pragma once
#include <Relentless.h>
#include "HorizontalBox.h"
#include "IStylableWidget.h"
#include "IWidgetContainer.h"

namespace Relentless
{
	class ITableRow : public IStylableWidget<ITableRow>
	{
	public:
		ITableRow() noexcept = default;
		virtual ~ITableRow() noexcept override = default;

		template<typename T>
		ITableRow* OnClicked(T&& callback) noexcept
		{
			m_OnClickedCallback = Callback<void(const PointerInfo&)>(std::forward<T>(callback));
			return this;
		}

		template<typename InstanceType>
		ITableRow* OnClicked(InstanceType* instance, void(InstanceType::* method)(const PointerInfo&)) noexcept
		{
			m_OnClickedCallback = [instance, method](const PointerInfo& pointerInfo) { return (instance->*method)(pointerInfo); };
			return this;
		}

		template<typename T>
		ITableRow* OnDoubleClicked(T&& callback) noexcept
		{
			m_OnDoubleClickedCallback = Callback<void(const PointerInfo&, ITableRow*)>(std::forward<T>(callback));
			return this;
		}

		template<typename InstanceType>
		ITableRow* OnDoubleClicked(InstanceType* instance, void(InstanceType::*method)(const PointerInfo&, ITableRow*)) noexcept
		{
			m_OnDoubleClickedCallback = [instance, method](const PointerInfo& aPointerInfo, ITableRow* aTableRow) { return (instance->*method)(aPointerInfo, aTableRow); };
			return this;
		}

		virtual const Color& GetBackgroundColor() const noexcept
		{
			static Color defaultColor = Colors::Transparent;
			return defaultColor;
		};

		virtual uint32 GetNumColumns() noexcept = 0;

		virtual void OnRender() noexcept override;
		virtual void OnRenderColumn(uint32 column) noexcept = 0;

		NO_DISCARD Vector2 ReportSize() const noexcept override;

		void SetColumnWidget(uint8 aColumnIndex, const Ref<HorizontalBox>& aWidget) noexcept;
		void SetIndentation(uint32 aIndentation) noexcept;
	protected:
		std::vector<Ref<HorizontalBox>> m_ColumnWidgets;

		Callback<void(const PointerInfo& pointerInfo)> m_OnClickedCallback;
		Callback<void(const PointerInfo& pointerInfo, ITableRow* aTableRow)> m_OnDoubleClickedCallback;
		
		uint32 m_IndentationLevel = 0;

		bool m_CustomHoverLogic = false;
		bool m_Hovered = false;
		bool m_Tiled = false;
	};
}