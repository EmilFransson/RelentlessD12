#pragma once
#include <Relentless.h>
#include "IDetailLayoutBuilder.h"
#include "../Customizations/IDetailCustomization.h"

#include "../Customizations/TransformComponentDetailCustomization.h"
#include "../Customizations/DirectionalLightComponentDetailCustomization.h"
#include "../Customizations/PointLightComponentDetailCustomization.h"
#include "../Customizations/SpotLightComponentDetailCustomization.h"

#include "../EntityDetailsView.h"

namespace Relentless
{
	class Selection;

	class EntityDetailLayoutBuilder : public IDetailLayoutBuilder
	{
	public:
		EntityDetailLayoutBuilder(IDetailsView* aDetailsView, Scene& aScene) noexcept;
		virtual ~EntityDetailLayoutBuilder() noexcept override = default;

		template<typename T>
		void ConditionallyAddCustomization() noexcept;

		NO_DISCARD Scene& GetScene() const noexcept;

		NO_DISCARD std::vector<Ref<DetailNode>> Rebuild() noexcept;
		void RequestRefresh() noexcept;
	private:
		std::vector<UniquePtr<IDetailCustomization>> m_Customizations;
		Scene& m_Scene;
	};

	template<class T>
	struct DetailCustomizationFor;

	template<> struct DetailCustomizationFor<TransformComponent> { using type = TransformComponentDetailCustomization; };
	template<> struct DetailCustomizationFor<DirectionalLightComponent> { using type = DirectionalLightComponentDetailCustomization; };
	template<> struct DetailCustomizationFor<PointLightComponent> { using type = PointLightComponentDetailCustomization; };
	template<> struct DetailCustomizationFor<SpotLightComponent> { using type = SpotLightComponentDetailCustomization; };

	template<typename T>
	void EntityDetailLayoutBuilder::ConditionallyAddCustomization() noexcept
	{
		EntityDetailsView* pView = static_cast<EntityDetailsView*>(m_pView);

		const std::vector<entity>& inspectedEntities = pView->GetInspectedEntities();
		EntityManager& entityManager = m_Scene.GetEntityManager();

		if (!std::ranges::all_of(inspectedEntities, [&entityManager](entity aEntity) { return entityManager.Has<T>(aEntity); }))
			return;

		using C = typename DetailCustomizationFor<T>::type;
		m_Customizations.push_back(MakeUnique<C>());
		m_Customizations.back()->CustomizeDetails(static_cast<IDetailLayoutBuilder&>(*this));
	}

}