#pragma once
#include <UI/Widgets/Panel.h>

namespace Relentless
{
	class EntityDetailsView;

	class EntityDetailsPanel : public PanelBase
	{
	public:
		EntityDetailsPanel() noexcept;
		virtual ~EntityDetailsPanel() noexcept override;
	private:
		Ref<EntityDetailsView> m_pEntityDetailsView;
	};
}