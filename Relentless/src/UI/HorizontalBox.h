#pragma once
#include "IWidget.h"

namespace Relentless
{
	enum class EAlignmentPolicy : uint8 { Left, Center, Right };

	class HorizontalBox : public IWidget<HorizontalBox>
	{
	public:
		HorizontalBox(bool isChildRegion = false, const Vector2& size = Vector2::Zero) noexcept;

		template<typename T>
		T* Add(T* pWidget) noexcept
		{
			static_assert(std::is_base_of_v<IBaseWidget, T>, "[VerticalBox::Add]: Can only Add widgets derived from IBaseWidget");

			Ref<T> widgetRef(pWidget);
			m_Children.push_back(widgetRef);
			return widgetRef.Get();
		}

		template<typename T>
		T* Add(Ref<T> pWidget) noexcept
		{
			static_assert(std::is_base_of_v<IBaseWidget, T>, "[VerticalBox::Add]: Can only Add widgets derived from IBaseWidget");

			m_Children.push_back(pWidget);
			return pWidget.Get();
		}

		NO_DISCARD Ref<IBaseWidget> GetChild(uint32 index) const noexcept;

		void Remove(IBaseWidget* pWidget) noexcept
		{
			std::erase(m_Children, pWidget);
		}

		virtual [[nodiscard]] float CalcDesiredWidth() const noexcept override;
		[[nodiscard]] bool HasWidget(Ref<IBaseWidget> pWidget) noexcept;
		void SetAlignmentPolicy(EAlignmentPolicy alignmentPolicy) noexcept;
		void SetIsChildRegion(bool state) noexcept;
		void SetMargin(const FloatRect& margin) noexcept;
		void SetSize(const Vector2& size) noexcept;
		void SetSpacing(const Vector2& aSpacing) noexcept;
	protected:
		virtual void OnRender() noexcept override;
	private:
		virtual void SetWidthConstraint(float width) noexcept override;
	private:
		std::vector<Ref<IBaseWidget>> m_Children;
		FloatRect m_Margin;
		Vector2 m_Size = Vector2::Zero;
		Vector2 m_Spacing = Vector2::Zero;
		EAlignmentPolicy m_Alignment = EAlignmentPolicy::Left;
		bool m_IsChildRegion = false;
	};
}
