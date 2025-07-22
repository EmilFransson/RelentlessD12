#pragma once

#include "Callback/Broadcaster.h"
#include "IWidget.h"

namespace Relentless
{
	class CollapsibleSection : public IStylableWidget<CollapsibleSection>
	{
	public:
		CollapsibleSection(std::string_view text) noexcept;
		virtual ~CollapsibleSection() noexcept override = default;

		template<typename T>
		T* Add(T* pWidget) noexcept
		{
			static_assert(std::is_base_of_v<IBaseWidget, T>, "[CollapsibleSection::Add]: Can only Add widgets derived from IWidget");

			Ref<T> widgetRef(pWidget);
			m_Children.push_back(widgetRef);
			return widgetRef.Get();
		}

		template<typename T>
		T* Add(Ref<T> pWidget) noexcept
		{
			static_assert(std::is_base_of_v<IBaseWidget, T>, "[CollapsibleSection::Add]: Can only Add widgets derived from IWidget");

			m_Children.push_back(pWidget);
			return pWidget.Get();
		}

		virtual [[nodiscard]] float CalcDesiredWidth() const noexcept override;

		[[nodiscard]] bool HasWidget(Ref<IWidget> pWidget) const noexcept;

		virtual CollapsibleSection* SetActiveColor(const Color& color) noexcept override;
		virtual CollapsibleSection* SetBackgroundColor(const Color& color) noexcept override;
		virtual CollapsibleSection* SetHoverColor(const Color& color) noexcept override;

		Broadcaster<void(bool state)> OnOpenStateChanged;
	protected:
		virtual void OnRender() noexcept override;
	private:
		void DetermineOpenState(bool isOpenThisFrame) noexcept;
	private:
		std::vector<Ref<IBaseWidget>> m_Children;

		String m_Text;
		bool m_IsOpen = true;
	};
}