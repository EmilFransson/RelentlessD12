#pragma once
#include "IWidget.h"

namespace Relentless
{
	class VerticalBox : public IWidget<VerticalBox>
	{
	public:
		VerticalBox(const Vector2& size = Vector2::Zero, bool isChildRegion = false) noexcept;

		template<typename T>
		T* Add(T* pWidget) noexcept
		{
			static_assert(std::is_base_of_v<IBaseWidget, T>, "[VerticalBox::Add]: Can only Add widgets derived from IWidget");

			Ref<T> widgetRef(pWidget);
			m_Children.push_back(widgetRef);
			return widgetRef.Get();
		}

		template<typename T>
		T* Add(Ref<T> pWidget) noexcept
		{
			static_assert(std::is_base_of_v<IBaseWidget, T>, "[VerticalBox::Add]: Can only Add widgets derived from IWidget");

			m_Children.push_back(pWidget);
			return pWidget.Get();
		}

		virtual [[nodiscard]] float CalcDesiredWidth() const noexcept override;
		[[nodiscard]] bool HasWidget(Ref<IBaseWidget> pWidget) noexcept;
		void SetIsChildRegion(bool state) noexcept;
	protected:
		virtual void OnRender() noexcept override;
	private:
		std::vector<Ref<IBaseWidget>> m_Children;
		Vector2 m_Size = Vector2::Zero;
		bool m_IsChildRegion = false;
	};
}
