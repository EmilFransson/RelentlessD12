#pragma once
#include <Relentless.h>

namespace Relentless
{
	class EntityOutlinerView;

	class OutlinerPanel : public PanelBase
	{
	public:
		OutlinerPanel() noexcept;
		virtual ~OutlinerPanel() noexcept override = default;

		const Ref<EntityOutlinerView>& GetEntityOutlinerView() const noexcept;
	private:
		bool OnKeyPressedEvent(KeyPressedEvent& aEvent) noexcept;
	private:
		Ref<EntityOutlinerView> m_pEntityOutlinerView = nullptr;
	};
}