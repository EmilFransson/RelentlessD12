#pragma once
#include "IDetailCategoryBuilder.h" 

#include "Module/DetailsModule.h"
#include "Module/ModuleManager.h"

#include "UI/Views/Details/TableRows/DetailCategoryRow.h"
#include "UI/Views/Details/TableRows/DetailGroupRow.h"

namespace Relentless
{
	class IDetailsView;

	class IDetailLayoutBuilder
	{
	public:
		IDetailLayoutBuilder(IDetailsView* pDetailView) noexcept;
		virtual ~IDetailLayoutBuilder() noexcept = default;

		template<typename InspectedType>
		std::vector<Ref<DetailNode>> Build() noexcept;
		
		NO_DISCARD IDetailCategoryBuilder& EditCategory(const char* aName) noexcept;

		void ForceRefreshDetails() noexcept;

		NO_DISCARD IDetailsView* GetDetailsView() const noexcept;

		void TearDown() noexcept;
	protected:
		std::unordered_map<String, UniquePtr<IDetailCategoryBuilder>> m_Categories;
		std::vector<UniquePtr<IDetailCustomization>> m_Customizations;
		IDetailsView* m_pView = nullptr;
	};

	template<typename InspectedType>
	std::vector<Ref<DetailNode>> IDetailLayoutBuilder::Build() noexcept
	{
		TearDown();

		const DetailCustomizationRegistry& registry = ModuleManager::LoadModuleChecked<DetailsModule>().GetRegistry();
		m_Customizations = registry.GetCustomizations<InspectedType>();

		for (const auto& customization : m_Customizations)
		{
			if (customization->ShouldCustomize(*this))
				customization->CustomizeDetails(*this);
		}

		std::vector<Ref<DetailNode>> nodesToReturn;
		for (auto& [name, pBuilder] : m_Categories)
		{
			Ref<DetailNode> pCategoryNode = RLS_NEW DetailNode(name.c_str());
			pCategoryNode->OnRequestRow([name](const ItemInfo& aItemInfo)
				{
					return RLS_NEW DetailCategoryRow(name, aItemInfo.IsExpanded);
				});

			for (auto& entry : pBuilder->GetEntries())
			{
				if (entry.IsGroup)
				{
					entry.Node->OnRequestRow([name = entry.GroupName](const ItemInfo& aItemInfo)
						{
							Ref<DetailGroupRow> pGroupRow = RLS_NEW DetailGroupRow(name, aItemInfo.IsExpanded);
							pGroupRow->SetIndentation(aItemInfo.Depth);

							return pGroupRow;
						});
				}
					
				pCategoryNode->AddChild(entry.Node);
			}

			nodesToReturn.push_back(pCategoryNode);
		}

		return nodesToReturn;
	}
}