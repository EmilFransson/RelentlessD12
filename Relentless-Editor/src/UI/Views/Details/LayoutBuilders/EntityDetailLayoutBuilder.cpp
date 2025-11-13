#include "EntityDetailLayoutBuilder.h"

#include "../Customizations/TransformComponentDetailCustomization.h"
#include "../TableRows/EntityDetailCategoryRow.h"

namespace Relentless
{
	EntityDetailLayoutBuilder::EntityDetailLayoutBuilder(IDetailsView* aDetailsView, Scene& aScene, Selection& aSelection) noexcept
		:IDetailLayoutBuilder(aDetailsView),
		 m_Scene{ aScene },
		 m_Selection{ aSelection }
	{
	}

	Scene& EntityDetailLayoutBuilder::GetScene() const noexcept
	{
		return m_Scene;
	}

	Selection& EntityDetailLayoutBuilder::GetSelection() const noexcept
	{
		return m_Selection;
	}

	std::vector<Ref<DetailNode>> EntityDetailLayoutBuilder::Rebuild() noexcept
	{
		m_Customizations.clear();
		m_Categories.clear();

		std::vector<Ref<DetailNode>> nodesToReturn;
		if (m_Selection.GetSelectedEntityCount() == 0)
			return nodesToReturn;

		ConditionallyAddCustomization<TransformComponent>();
		ConditionallyAddCustomization<DirectionalLightComponent>();

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
}
