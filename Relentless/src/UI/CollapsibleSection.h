#pragma once

#include "Callback/Broadcaster.h"
#include "IWidget.h"

namespace Relentless
{
	class CollapsibleSection : public IWidget
	{
	public:
		CollapsibleSection(std::string_view id) noexcept;

		void Add(Ref<IWidget> pWidget) noexcept;
		[[nodiscard]] bool HasWidget(Ref<IWidget> pWidget) const noexcept;

		Broadcaster<void(bool state)> OnOpenStateChanged;
	protected:
		virtual void OnRender() noexcept override;
	private:
		void DetermineOpenState(bool isOpenThisFrame) noexcept;
		void SetColorsAndStyles() noexcept;
	private:
		std::vector<Ref<IWidget>> m_Children;
		bool m_IsOpen = true;
	};
}

//TODO, container..?