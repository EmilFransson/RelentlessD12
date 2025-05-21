#pragma once

#include "Callback/Broadcaster.h"
#include "IWidget.h"

namespace Relentless
{
	class CollapsibleSection : public IStylableWidget
	{
	public:
		CollapsibleSection(std::string_view id) noexcept;

		template<typename T>
		T* Add(T* pWidget) noexcept
		{
			static_assert(std::is_base_of_v<IWidget, T>, "[CollapsibleSection::Add]: Can only Add widgets derived from IWidget");

			Ref<T> widgetRef(pWidget);
			m_Children.push_back(widgetRef);
			return widgetRef.Get();
		}

		template<typename T>
		T* Add(Ref<T> pWidget) noexcept
		{
			static_assert(std::is_base_of_v<IWidget, T>, "[CollapsibleSection::Add]: Can only Add widgets derived from IWidget");

			m_Children.push_back(pWidget);
			return pWidget.Get();
		}

		virtual [[nodiscard]] float CalcDesiredWidth() const noexcept override;

		[[nodiscard]] bool HasWidget(Ref<IWidget> pWidget) const noexcept;

		virtual void SetActiveColor(const Color& color) noexcept override;
		virtual void SetBackgroundColor(const Color& color) noexcept override;
		virtual void SetHoverColor(const Color& color) noexcept override;

		Broadcaster<void(bool state)> OnOpenStateChanged;
	protected:
		virtual void OnRender() noexcept override;
	private:
		void DetermineOpenState(bool isOpenThisFrame) noexcept;
	private:
		std::vector<Ref<IWidget>> m_Children;
		bool m_IsOpen = true;
	};
}