#pragma once
#include "IWidget.h"

namespace Relentless
{
	enum class EAlignmentPolicy : uint8 { Left, Center, Right };

	class HorizontalBox : public IWidget
	{
	public:
		HorizontalBox(std::string_view id, bool isChildRegion = false, const Vector2& size = Vector2::Zero) noexcept;

		void Add(Ref<IWidget> pWidget) noexcept;
		virtual [[nodiscard]] float CalcDesiredWidth() const noexcept override;
		[[nodiscard]] bool HasWidget(Ref<IWidget> pWidget) noexcept;
		void SetAlignmentPolicy(EAlignmentPolicy alignmentPolicy) noexcept;
		void SetIsChildRegion(bool state) noexcept;
		void SetMargin(const FloatRect& margin) noexcept;
	protected:
		virtual void OnRender() noexcept override;
	private:
		virtual void SetWidthConstraint(float width) noexcept override;
	private:
		std::vector<Ref<IWidget>> m_Children;
		FloatRect m_Margin;
		Vector2 m_Size = Vector2::Zero;
		EAlignmentPolicy m_Alignment = EAlignmentPolicy::Left;
		bool m_IsChildRegion = false;
	};
}
