#pragma once
#include <Relentless.h>
#include "EntityDetailsCustomizer.h"

namespace Relentless
{
	class EntityDetailsView : public IDetailsView
	{
	public:
		EntityDetailsView() noexcept;
		virtual ~EntityDetailsView() noexcept override = default;
		void SetEntities(const std::vector<entity>& entities, EntityManager& entityManager) noexcept;
	private:
		//std::vector<Ref<IDetailsTreeNode>> m_RootNodes;

		Ref<SearchBar> m_pSearchBar = nullptr;
		//Ref<ListView<Ref<IDetailsTreeNode>>> m_pTreeView = nullptr;
		Ref<EntityDetailsCustomizer> m_pCustomizer = nullptr;
	};
}