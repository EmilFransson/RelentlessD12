#include "EntityDetailLayoutBuilder.h"

#include "../Customizations/TransformComponentDetailCustomization.h"
#include "../TableRows/EntityDetailCategoryRow.h"

namespace Relentless
{
	EntityDetailLayoutBuilder::EntityDetailLayoutBuilder(IDetailsView* aDetailsView, Scene& aScene) noexcept
		:IDetailLayoutBuilder(aDetailsView),
		 m_Scene{ aScene }
	{
	}

	Scene& EntityDetailLayoutBuilder::GetScene() const noexcept
	{
		return m_Scene;
	}

	std::vector<Ref<DetailNode>> EntityDetailLayoutBuilder::Rebuild() noexcept
	{
		m_Customizations.clear();
		m_Categories.clear();

		EntityDetailsView* pView = static_cast<EntityDetailsView*>(m_pView);

		std::vector<Ref<DetailNode>> nodesToReturn;
		if (pView->GetNumInspectedEntities() == 0u)
			return nodesToReturn;

		ConditionallyAddCustomization<TransformComponent>();
		ConditionallyAddCustomization<DirectionalLightComponent>();
		ConditionallyAddCustomization<PointLightComponent>();
		ConditionallyAddCustomization<SpotLightComponent>();

		for (auto& [name, pBuilder] : m_Categories)
		{
			Ref<DetailNode> pCategoryNode = new DetailNode(name.c_str());
			pCategoryNode->OnRequestRow([&](const ItemInfo& aItemInfo) 
				{ 
					return new EntityDetailCategoryRow(name, aItemInfo.IsExpanded);
				});

			pCategoryNode->SetChildren(pBuilder->GetNodes());
			nodesToReturn.push_back(pCategoryNode);
		}
		
		return nodesToReturn;
	}

	void EntityDetailLayoutBuilder::RequestRefresh() noexcept
	{
		m_pView->RequestRefresh();
	}

}
