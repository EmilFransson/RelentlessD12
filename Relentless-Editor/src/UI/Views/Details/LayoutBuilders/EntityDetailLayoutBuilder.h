#pragma once
#include <Relentless.h>
#include "IDetailLayoutBuilder.h"
#include "../Customizations/IDetailCustomization.h"

#include "../Customizations/TransformComponentDetailCustomization.h"
#include "../Customizations/DirectionalLightComponentDetailCustomization.h"

#include "../../../../Core/Selection.h"

namespace Relentless
{
	class Selection;

	class EntityDetailLayoutBuilder : public IDetailLayoutBuilder
	{
	public:
		EntityDetailLayoutBuilder(IDetailsView* aDetailsView, Scene& aScene, Selection& aSelection) noexcept;
		virtual ~EntityDetailLayoutBuilder() noexcept override = default;

		template<typename T>
		void ConditionallyAddCustomization() noexcept;

		NO_DISCARD Scene& GetScene() const noexcept;
		NO_DISCARD Selection& GetSelection() const noexcept;

		NO_DISCARD std::vector<Ref<DetailNode>> Rebuild() noexcept;
	private:
		std::vector<UniquePtr<IDetailCustomization>> m_Customizations;
		Scene& m_Scene;
		Selection& m_Selection;
	};

	template<class T>
	struct DetailCustomizationFor;

	template<> struct DetailCustomizationFor<TransformComponent> { using type = TransformComponentDetailCustomization; };
	template<> struct DetailCustomizationFor<DirectionalLightComponent> { using type = DirectionalLightComponentDetailCustomization; };

	template<typename T>
	void EntityDetailLayoutBuilder::ConditionallyAddCustomization() noexcept
	{
		const std::vector<entity>& selectedEntities = m_Selection.GetSelectedEntities();
		EntityManager& entityManager = m_Scene.GetEntityManager();

		if (!std::ranges::all_of(selectedEntities, [&entityManager](entity aEntity) { return entityManager.Has<T>(aEntity); }))
			return;

		using C = typename DetailCustomizationFor<T>::type;
		m_Customizations.push_back(std::make_unique<C>());
		m_Customizations.back()->CustomizeDetails(static_cast<IDetailLayoutBuilder&>(*this));
	}

}