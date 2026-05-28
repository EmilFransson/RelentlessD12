#pragma once
#include "Panel.h"

namespace Relentless
{
	class EntityDetailsView;

	class EntityDetailsPanel : public PanelBase
	{
	public:
		EntityDetailsPanel() noexcept;
		virtual ~EntityDetailsPanel() noexcept override = default;
		
		NO_DISCARD virtual String GetDisplayName() const noexcept override;
		NO_DISCARD virtual String GetPersistKey() const noexcept override;
	private:
		EntityDetailsView* m_pEntityDetailsView = nullptr;
	};
}