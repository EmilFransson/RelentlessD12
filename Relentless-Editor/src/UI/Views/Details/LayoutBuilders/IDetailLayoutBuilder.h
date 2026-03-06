#pragma once
#include "IDetailCategoryBuilder.h" 

#include "Module/DetailsModule.h"
#include "Module/ModuleManager.h"

#include "UI/Views/Details/TableRows/DetailCategoryRow.h"

namespace Relentless
{
	class IDetailsView;

	class IDetailLayoutBuilder
	{
	public:
		IDetailLayoutBuilder(IDetailsView* pDetailView) noexcept;
		virtual ~IDetailLayoutBuilder() noexcept = default;

		NO_DISCARD IDetailCategoryBuilder& EditCategory(const char* aName) noexcept;

		NO_DISCARD IDetailsView* GetDetailsView() const noexcept;

		template<typename InspectedType>
		std::vector<Ref<DetailNode>> Build() noexcept;

	protected:
		std::unordered_map<String, UniquePtr<IDetailCategoryBuilder>> m_Categories;
		IDetailsView* m_pView = nullptr;
	};

	template<typename InspectedType>
	std::vector<Ref<DetailNode>> IDetailLayoutBuilder::Build() noexcept
	{
		const DetailCustomizationRegistry& registry = ModuleManager::LoadModuleChecked<DetailsModule>().GetRegistry();
		const std::vector<UniquePtr<IDetailCustomization>> customizations = registry.GetCustomizations<InspectedType>();

		for (const auto& customization : customizations)
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

			pCategoryNode->SetChildren(pBuilder->GetNodes());
			nodesToReturn.push_back(pCategoryNode);
		}

		return nodesToReturn;
	}
}