#include "EntityDetailsView.h"

namespace Relentless
{
	EntityDetailsView::EntityDetailsView() noexcept
	{
		m_pSearchBar = new SearchBar("Search...");
	}

	void EntityDetailsView::SetEntities(const std::vector<entity>& entities, EntityManager& entityManager) noexcept
	{
	}
}
