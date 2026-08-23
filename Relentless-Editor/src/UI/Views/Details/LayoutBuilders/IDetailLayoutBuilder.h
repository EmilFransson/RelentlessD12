#pragma once
#include "ContextMenuBuilder.h"

#include "IDetailCategoryBuilder.h" 
#include "ImGui/ImGuiFonts.h"

#include "Module/DetailsModule.h"
#include "Module/ModuleManager.h"

#include "UI/Views/Details/TableRows/DetailCategoryRow.h"
#include "UI/Views/Details/TableRows/DetailGroupRow.h"

namespace Relentless
{
	class IDetailsView;

	class IDetailLayoutBuilder : public SharedFromThis<IDetailLayoutBuilder>
	{
	public:
		IDetailLayoutBuilder(IDetailsView* pDetailView) noexcept;
		virtual ~IDetailLayoutBuilder() noexcept;

		template<typename InspectedType>
		std::vector<Ref<DetailNode>> Build() noexcept;

		void CollapseAll() noexcept;
		
		NO_DISCARD IDetailCategoryBuilder& EditCategory(const char* aName) noexcept;
		void ExpandAll() noexcept;

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

		SharedPtr<IDetailLayoutBuilder> pSharedPtr = GetWeakPtr().lock();

		for (const auto& customization : m_Customizations)
		{
			if (!customization->ShouldCustomize(*this))
				continue;

			customization->CustomizeDetails(pSharedPtr);
			customization->CustomizeDetails(*this);
		}

		std::vector<Ref<DetailNode>> nodesToReturn;
		for (auto& [name, pBuilder] : m_Categories)
		{
			Ref<DetailNode> pCategoryNode = RLS_NEW DetailNode(name.c_str());
			pCategoryNode->OnRequestRow([this, name](const ItemInfo& aItemInfo)
				{
					Ref<DetailCategoryRow> pRow = RLS_NEW DetailCategoryRow(name, aItemInfo.IsExpanded);

					pRow->OnContextMenuOpening([this, name]()
						{
							ContextMenuBuilder builder;
							if (!m_Categories.contains(name))
								return builder.BuildContextMenu();

							const std::vector<HeaderAction>& headerActions = m_Categories.at(name)->GetHeaderActions();
							if (!headerActions.empty())
							{
								builder.AddSection("Actions");

								for (const HeaderAction& headerAction : headerActions)
								{
									builder.AddItem(headerAction.Label,	[onClicked = headerAction.OnClicked, closeOnSelection = headerAction.CloseOnSelection]()
										{
											const bool bClose = closeOnSelection;
											onClicked();
											if (bClose)
												ModuleManager::LoadModuleChecked<UIModule>().DestroyActiveContextMenu();
										});
								}
							}

							builder.AddSection("Expansion");

							builder.AddItem("Collapse All", [this]() 
								{ 
									CollapseAll(); 
									ModuleManager::LoadModuleChecked<UIModule>().DestroyActiveContextMenu();
								});
							builder.AddItem("Expand All", [this]() 
								{
									ExpandAll();
									ModuleManager::LoadModuleChecked<UIModule>().DestroyActiveContextMenu();
								});

							return builder.BuildContextMenu();
						});

					return pRow;
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