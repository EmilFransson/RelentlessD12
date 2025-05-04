#pragma once
#include "IWidget.h"

namespace Relentless
{
	class VerticalBox : public IWidget
	{
	public:
		VerticalBox(std::string_view id) noexcept;

		void Add(Ref<IWidget> pWidget) noexcept;
		[[nodiscard]] bool HasWidget(Ref<IWidget> pWidget) noexcept;
	protected:
		virtual void OnRender() noexcept override;
	private:
		std::vector<Ref<IWidget>> m_Children;
	};
}
