#pragma once
#include <UI/Widgets/Panel.h>

namespace Relentless
{
	class EntityDetailsView;

	class EntityDetailsPanel : public PanelBase
	{
	public:
		EntityDetailsPanel() noexcept;
		virtual ~EntityDetailsPanel() noexcept override = default;
	private:
		EntityDetailsView* m_pEntityDetailsView = nullptr;
	};
}