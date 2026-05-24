#pragma once
#include "IWidget.h"

namespace Relentless
{
	template<typename DerivedType>
	class IWidgetContainer : public IWidget<IWidgetContainer<DerivedType>>
	{
	public:
		IWidgetContainer(const Vector2 aSize = Vector2::Zero, bool aIsChildRegion = false) noexcept;
		virtual ~IWidgetContainer() noexcept = default;

		template<typename T>
		T* AddWidget(Ref<T> aWidget) noexcept;

		template<typename T>
		T* AddWidget(T* aWidget) noexcept;

		NO_DISCARD uint32 GetNumWidgets() const noexcept;

		template<typename T>
		NO_DISCARD T* GetWidget(uint32 aIndex) const noexcept;

		NO_DISCARD bool HasWidget(Ref<IBaseWidget> aWidget) const noexcept;

		NO_DISCARD bool IsContainer() const noexcept override;
		NO_DISCARD bool IsHorizontalScrollBarEnabled() const noexcept;
		NO_DISCARD bool IsMouseScrollingEnabled() const noexcept;
		NO_DISCARD bool IsScrollBarsVisible() const noexcept;

		void RemoveAllWidgets() noexcept;
		void RemoveWidget(IBaseWidget* aWidget) noexcept;

		DerivedType* SetHorizontalScrollBarEnabled(bool aEnabled) noexcept;
		DerivedType* SetMouseScrollingEnabled(bool aEnabled) noexcept;
		DerivedType* SetScrollBarsVisible(bool aVisible) noexcept;

		virtual DerivedType* SetSpacing(float aSpacing) noexcept = 0;

		Broadcaster<void(bool)> OnFocusChanged;
	protected:
		virtual void OnRender() noexcept override = 0;
	protected:
		std::vector<Ref<IBaseWidget>> m_Widgets;
		Vector2 m_Size = Vector2::Zero;
		Vector2 m_Spacing = Vector2::Zero;
		bool m_IsChildRegion = false;
		bool m_HorizontalScrollBarEnabled = false;
		bool m_ScrollBarsVisible = false;
		bool m_MouseScrollingEnabled = true;
		bool m_IsFocused = false;
	};

	template<typename DerivedType>
	void IWidgetContainer<DerivedType>::RemoveAllWidgets() noexcept
	{
		m_Widgets.clear();
	}

	template<typename DerivedType>
	IWidgetContainer<DerivedType>::IWidgetContainer(const Vector2 aSize, bool aIsChildRegion) noexcept
		: m_Size{ aSize }
		, m_IsChildRegion{ aIsChildRegion }
	{
	}

	template<typename DerivedType>
	template<typename T>
	T* IWidgetContainer<DerivedType>::AddWidget(Ref<T> aWidget) noexcept
	{
		static_assert(std::is_base_of_v<IBaseWidget, T>, "[IWidgetContainer::AddWidget]: Can only Add widgets derived from IBaseWidget");
		RLS_ASSERT(!HasWidget(aWidget), "[IWidgetContainer::AddWidget]: Widget already exist in container.");

		m_Widgets.push_back(aWidget);
		return static_cast<T*>(m_Widgets.back().Get());
	}

	template<typename DerivedType>
	template<typename T>
	T* IWidgetContainer<DerivedType>::AddWidget(T* aWidget) noexcept
	{
		return AddWidget(Ref<T>(aWidget));
	}

	template<typename DerivedType>
	uint32 IWidgetContainer<DerivedType>::GetNumWidgets() const noexcept
	{
		return static_cast<uint32>(m_Widgets.size());
	}

	template<typename DerivedType>
	template<typename T>
	T* IWidgetContainer<DerivedType>::GetWidget(uint32 aIndex) const noexcept
	{
		return aIndex < m_Widgets.size() ? static_cast<T*>(m_Widgets[aIndex].Get()) : nullptr;
	}

	template<typename DerivedType>
	bool IWidgetContainer<DerivedType>::HasWidget(Ref<IBaseWidget> aWidget) const noexcept
	{
		return std::ranges::find_if(m_Widgets, [&](const Ref<IBaseWidget>& aChildWidget) { return aChildWidget == aWidget; }) != m_Widgets.end();
	}

	template<typename DerivedType>
	bool IWidgetContainer<DerivedType>::IsContainer() const noexcept
	{
		return true;
	}

	template<typename DerivedType>
	bool IWidgetContainer<DerivedType>::IsHorizontalScrollBarEnabled() const noexcept
	{
		return m_HorizontalScrollBarEnabled;
	}

	template<typename DerivedType>
	bool IWidgetContainer<DerivedType>::IsMouseScrollingEnabled() const noexcept
	{
		return m_MouseScrollingEnabled;
	}

	template<typename DerivedType>
	bool IWidgetContainer<DerivedType>::IsScrollBarsVisible() const noexcept
	{
		return m_ScrollBarsVisible;
	}

	template<typename DerivedType>
	void IWidgetContainer<DerivedType>::RemoveWidget(IBaseWidget* aWidget) noexcept
	{
		std::erase_if(m_Widgets, [aWidget](const Ref<IBaseWidget>& aChildWidget) { return aChildWidget.Get() == aWidget; });
	}

	template<typename DerivedType>
	DerivedType* IWidgetContainer<DerivedType>::SetHorizontalScrollBarEnabled(bool aEnabled) noexcept
	{
		m_HorizontalScrollBarEnabled = aEnabled;
		return static_cast<DerivedType*>(this);
	}

	template<typename DerivedType>
	DerivedType* IWidgetContainer<DerivedType>::SetMouseScrollingEnabled(bool aEnabled) noexcept
	{
		m_MouseScrollingEnabled = aEnabled;
		return static_cast<DerivedType*>(this);
	}

	template<typename DerivedType>
	DerivedType* IWidgetContainer<DerivedType>::SetScrollBarsVisible(bool aVisible) noexcept
	{
		m_ScrollBarsVisible = aVisible;
		return static_cast<DerivedType*>(this);
	}
}