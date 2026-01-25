#include "EntityDetailCustomization.h"
#include "../EntityDetailsView.h"

namespace Relentless
{
	const std::vector<entity>& EntityDetailCustomization::GetInspectedEntities() const noexcept
	{
		auto* pView = static_cast<EntityDetailsView*>(m_pBuilder->GetDetailsView());
		const auto& entities = pView->GetInspectedEntities();
		RLS_ASSERT(!entities.empty(), "[EntityDetailCustomization::GetInspectedEntities]: No inspected entities.");
		return entities;
	}

	entity EntityDetailCustomization::GetPrimaryEntity() const noexcept
	{
		auto entities = GetInspectedEntities();
		return entities.back();
	}

	void EntityDetailCustomization::OnRevertButtonHoverBegin(Button* aButton) noexcept
	{
		aButton->SetTextColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
	}
	
	void EntityDetailCustomization::OnRevertButtonHoverEnd(Button* aButton) noexcept
	{
		aButton->SetTextColor(Color(1.0f, 1.0f, 1.0f, 0.5f));
	}
}
