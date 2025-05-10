#pragma once

#include "Callback/Broadcaster.h"
#include "IWidget.h"

namespace Relentless
{
	class CollapsibleSection : public IStylableWidget
	{
	public:
		CollapsibleSection(std::string_view id) noexcept;

		void Add(Ref<IWidget> pWidget) noexcept;
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