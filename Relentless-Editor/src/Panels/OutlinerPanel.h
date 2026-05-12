#pragma once
#include <Relentless.h>

#include "../UI/Views/Outliner/EntityOutlinerView.h"
#include "../UI/Widgets/Panel.h"

namespace Relentless
{
	class EntityOutlinerView;

	class OutlinerPanel : public PanelBase
	{
	public:
		OutlinerPanel() noexcept;
		virtual ~OutlinerPanel() noexcept override = default;

		const Ref<EntityOutlinerView>& GetEntityOutlinerView() const noexcept;
		
		NO_DISCARD virtual String GetDisplayName() const noexcept override;
		NO_DISCARD virtual String GetPersistKey() const noexcept override;
	private:
		bool OnKeyPressedEvent(KeyPressedEvent& aEvent) noexcept override;
	private:
		Ref<EntityOutlinerView> m_pEntityOutlinerView = nullptr;
	};
}