#pragma once
#include <Relentless.h>
#include "IEditorPanel.h"

namespace Relentless
{
	class EntityOutlinerView;

	class OutlinerPanel : public IEditorPanel
	{
	public:
		OutlinerPanel(std::weak_ptr<Editor> aEditor) noexcept;
		virtual ~OutlinerPanel() noexcept override = default;

		const Ref<EntityOutlinerView>& GetEntityOutlinerView() const noexcept;
	private:
		bool OnKeyPressedEvent(KeyPressedEvent& aEvent) noexcept;
	private:
		Ref<EntityOutlinerView> m_pEntityOutlinerView = nullptr;
	};
}