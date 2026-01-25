#pragma once
#include <Relentless.h>

#include "DirectionalLightComponentDetailCustomization.h"
#include "PointLightComponentDetailCustomization.h"
#include "SpotLightComponentDetailCustomization.h"
#include "TransformComponentDetailCustomization.h"

#include "UI/Views/Details/EntityDetailsView.h"
#include "UI/Views/Details/LayoutBuilders/EntityDetailLayoutBuilder.h"

namespace Relentless
{
	template<class T>
	struct DetailCustomizationFor;

	template<> struct DetailCustomizationFor<TransformComponent> { using type = TransformComponentDetailCustomization; };
	template<> struct DetailCustomizationFor<DirectionalLightComponent> { using type = DirectionalLightComponentDetailCustomization; };
	template<> struct DetailCustomizationFor<PointLightComponent> { using type = PointLightComponentDetailCustomization; };
	template<> struct DetailCustomizationFor<SpotLightComponent> { using type = SpotLightComponentDetailCustomization; };

	template<typename T>
	void ConditionallyAddCustomization(EntityDetailLayoutBuilder* aBuilder) noexcept
	{
		EntityDetailsView* pView = static_cast<EntityDetailsView*>(aBuilder->GetDetailsView());

		const std::vector<entity>& inspectedEntities = pView->GetInspectedEntities();
		EntityManager& entityManager = aBuilder->GetScene().GetEntityManager();

		if (!std::ranges::all_of(inspectedEntities, [&entityManager](entity aEntity) { return entityManager.Has<T>(aEntity); }))
			return;

		auto& customizations = aBuilder->GetCustomizations();

		using C = typename DetailCustomizationFor<T>::type;
		customizations.push_back(MakeUnique<C>());
		customizations.back()->CustomizeDetails(static_cast<IDetailLayoutBuilder&>(*aBuilder));
	}
}
