#pragma once
#include "IWidget.h"

namespace Relentless
{
	class VerticalBox : public IWidget
	{
	public:
		VerticalBox(std::string_view id, const Vector2& size = Vector2::Zero, bool isChildRegion = false) noexcept;

		void Add(Ref<IWidget> pWidget) noexcept;
		virtual [[nodiscard]] float CalcDesiredWidth() const noexcept override;
		[[nodiscard]] bool HasWidget(Ref<IWidget> pWidget) noexcept;
		void SetIsChildRegion(bool state) noexcept;
	protected:
		virtual void OnRender() noexcept override;
	private:
		std::vector<Ref<IWidget>> m_Children;
		Vector2 m_Size = Vector2::Zero;
		bool m_IsChildRegion = false;
	};
}
