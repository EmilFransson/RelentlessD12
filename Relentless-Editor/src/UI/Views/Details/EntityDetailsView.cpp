#include "EntityDetailsView.h"

#include "Core/Editor.h"

#include "ImGui/ImGuiFonts.h"

#include "Subsystem/EntityComponentDefinitionRegistry.h"
#include "Subsystem/SelectionSubsystem.h"

#include "UI/Views/Details/LayoutBuilders/ContextMenuBuilder.h"
#include "UI/Widgets/VerticalBox.h"
#include "UI/Widgets/WidgetSwitcher.h"

namespace Relentless
{
	EntityDetailsView::EntityDetailsView() noexcept
	{
		SetHorizontalSizePolicy(ESizePolicy::Stretch);
		SetVerticalSizePolicy(ESizePolicy::Stretch);

		Editor::OnEntityTransformed.Connect(this, &EntityDetailsView::OnEntityTransformed);
		CoreObjectBroadcasters::OnEntityComponentPropertyChanged.Connect(this, &EntityDetailsView::OnEntityComponentPropertyChanged);

		Editor* pEditor = Editor::Get();
		pEditor->OnSceneChange.Connect(this, &EntityDetailsView::OnSceneChange);
		pEditor->OnSceneChanged.Connect(this, &EntityDetailsView::OnSceneChanged);
		pEditor->GetSubsystem<SelectionSubsystem>()->OnSelectionChanged.Connect(this, &EntityDetailsView::OnSelectionChanged);

		RequestRebuildHeader();
		SetContext(&m_Context);
	}

	EntityDetailsView::~EntityDetailsView() noexcept
	{
		if (Editor* pEditor = Editor::Get())
		{
			if (SelectionSubsystem* pSelection = pEditor->GetSubsystem<SelectionSubsystem>())
				pSelection->OnSelectionChanged.Detach(this);

			pEditor->OnSceneChange.Detach(this);
			pEditor->OnSceneChanged.Detach(this);
		}

		if (m_pInspectedScene)
		{
			m_pInspectedScene->OnEntityDestroyed.Detach(this);
			m_pInspectedScene = nullptr;
		}

		Editor::OnEntityTransformed.Detach(this);
		CoreObjectBroadcasters::OnEntityComponentPropertyChanged.Detach(this);
	}

	bool EntityDetailsView::IsLocked() const noexcept
	{
		return m_IsLocked;
	}

	void EntityDetailsView::SetLocked(bool aLock) noexcept
	{
		if (m_IsLocked == aLock) 
			return;
		
		m_IsLocked = aLock;
		RequestRebuildHeader();
	}

	void EntityDetailsView::OnPreRequestSource(bool aFromManualTrigger) noexcept
	{
		if (aFromManualTrigger && !m_Context.Entities.empty())
			Rebuild<EntityDetailsContext>();
		else
		{
			if (m_Context.Entities.empty())
				m_RootNodes.clear();

			TearDown();
		}
	}

	Ref<Button> EntityDetailsView::BuildAddComponentButton() noexcept
	{
		Ref<Button> pButton = RLS_NEW Button(std::format("{} Add", ICON_FA_PLUS));
		pButton->SetFont(UI::Fonts::Get("Medium"));
		pButton->OnClicked(this, &EntityDetailsView::OnAddComponentButtonClicked);

		return pButton;
	}

	void EntityDetailsView::BuildEmptyHeader(HorizontalBox* aRow) noexcept
	{
		aRow->SetHorizontalAlignmentPolicy(EHorizontalAlignmentPolicy::Center);
		aRow->AddWidget(RLS_NEW Label("Select an entity to view its details."))
			->SetTextColor(Colors::TextInactive);
	}

	void EntityDetailsView::BuildHeader() noexcept
	{
		VerticalBox* pHeaderRow = GetHeader();
		pHeaderRow->SetMargin(FloatRect::Uniform(5.0f));
		pHeaderRow->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);
		pHeaderRow->RemoveAllWidgets();
		
		HorizontalBox* pHorizontalBox = pHeaderRow->AddWidget(RLS_NEW HorizontalBox());
		
		SelectionSubsystem* pSelectionSubsystem = Editor::Get()->GetSubsystem<SelectionSubsystem>();
		const std::vector<entity>& selectedEntities = pSelectionSubsystem->GetSelectedEntities();

		if (selectedEntities.empty())
			BuildEmptyHeader(pHorizontalBox);
		else if (selectedEntities.size() == 1)
			BuildSingleEntityHeader(pHorizontalBox, selectedEntities.front());
		else
			BuildMultiEntityHeader(pHorizontalBox, static_cast<uint32>(selectedEntities.size()));
	}

	void EntityDetailsView::BuildSingleEntityHeader(HorizontalBox* aRow, entity aEntity) noexcept
	{
		const NameComponent& nameComponent = m_pInspectedScene->GetEntityManager().Get<NameComponent>(aEntity);

		auto [pLeftBox, pRightBox] = BuildTwoSlotLayout(aRow);

		pLeftBox->AddWidget(RLS_NEW Label(ICON_FA_CUBE, UI::Fonts::Get("Medium")));

		WidgetSwitcher* pSwitcher = pLeftBox->AddWidget(RLS_NEW WidgetSwitcher());
		Label* pNameLabel = pSwitcher->Add(RLS_NEW Label(nameComponent.GetName().c_str(), UI::Fonts::Get("Medium")));
		EditableTextBox* pNameEdit = pSwitcher->Add(RLS_NEW EditableTextBox());
		pNameEdit->SetFont(UI::Fonts::Get("Medium"));
		pSwitcher->SetActiveWidgetIndex(0);

		pNameLabel->SetTooltip(RLS_NEW Tooltip(std::format("Rename the selected entity '{}'", nameComponent.GetName())));
		pNameLabel->OnMouseDoubleClick.Connect([pSwitcher, pNameEdit, pNameLabel](const WidgetGeometry&, const PointerInfo&)
			{
				pNameEdit->SetText(pNameLabel->GetText());
				pSwitcher->SetActiveWidgetIndex(1);
				pNameEdit->ForceKeyboardFocus();
			});

		pNameEdit->OnTextCommitted([this, pSwitcher, pNameLabel](const char* aText, ETextCommitType)
			{
				SelectionSubsystem* pSelectionSubsystem = Editor::Get()->GetSubsystem<SelectionSubsystem>();
				const std::vector<entity>& selection = pSelectionSubsystem->GetSelectedEntities();
				if (selection.empty())
					return;

				ScopedSuspend scopedSuspend(m_SuspendEditCallback);

				m_pInspectedScene->GetEntityManager().Get<NameComponent>(selection.front()).SetName(aText);
				pNameLabel->SetText(aText);
				pSwitcher->SetActiveWidgetIndex(0);
			});

		pRightBox->AddWidget(BuildAddComponentButton());
		pRightBox->AddWidget(BuildLockButton());
	}

	void EntityDetailsView::BuildMultiEntityHeader(HorizontalBox* aRow, uint32 aEntityCount) noexcept
	{
		auto [pLeftBox, pMiddleBox, pRightBox] = BuildThreeSlotLayout(aRow);

		pLeftBox->AddWidget(RLS_NEW Label(ICON_FA_CUBES, UI::Fonts::Get("Medium")))
			->SetTextColor(Colors::TextInactive);

		pMiddleBox->AddWidget(RLS_NEW Label(std::format("{} entities", aEntityCount), UI::Fonts::Get("Medium")))
			->SetTextColor(Colors::TextInactive);

		pRightBox->AddWidget(BuildAddComponentButton());
		pRightBox->AddWidget(BuildLockButton());
	}

	Ref<Button> EntityDetailsView::BuildLockButton() noexcept
	{
		UI::Fonts::PushFont("Medium");
		const Vector2 textSizeLocked = UI::CalculateTextSize(ICON_FA_LOCK);
		const Vector2 textSizeUnlocked = UI::CalculateTextSize(ICON_FA_LOCK_OPEN);
		constexpr float buttonPadding = 5.0f;
		const Vector2 buttonSize = Vector2(Math::Max(textSizeLocked.x, textSizeUnlocked.x) + buttonPadding, Math::Max(textSizeLocked.y, textSizeUnlocked.y) + buttonPadding);
		UI::Fonts::PopFont();

		Ref<Button> pLockButton = RLS_NEW Button(IsLocked() ? ICON_FA_LOCK : ICON_FA_LOCK_OPEN);
		pLockButton->SetBackgroundColor(Colors::Transparent);
		pLockButton->SetBorderColor(Colors::Transparent);
		pLockButton->SetFont(UI::Fonts::Get("Medium"));
		pLockButton->SetHorizontalSizePolicy(ESizePolicy::Fixed);
		pLockButton->SetVerticalSizePolicy(ESizePolicy::Fixed);
		pLockButton->SetSize(buttonSize);
		pLockButton->SetTextColor(Colors::TextInactive);
		pLockButton->SetTooltip(RLS_NEW Tooltip(std::format("{} the current selection for the Details panel", IsLocked() ? "Unlocks" : "Locks")));
		pLockButton->OnClicked([this]()
			{
				SetLocked(!IsLocked());
			});
		pLockButton->OnMouseEnter([](Button* aButton) { aButton->SetTextColor(Colors::TextDefault); });
		pLockButton->OnMouseExit([](Button* aButton) { aButton->SetTextColor(Colors::TextInactive); });

		return pLockButton;
	}

	ThreeSlotLayout EntityDetailsView::BuildThreeSlotLayout(HorizontalBox* aRow) noexcept
	{
		HorizontalBox* pLeft = aRow->AddWidget(RLS_NEW HorizontalBox());
		HorizontalBox* pMiddle = aRow->AddWidget(RLS_NEW HorizontalBox());
		HorizontalBox* pRight = aRow->AddWidget(RLS_NEW HorizontalBox());

		pLeft->SetHorizontalSizePolicy(ESizePolicy::Stretch);
		pMiddle->SetHorizontalSizePolicy(ESizePolicy::Stretch);
		pRight->SetHorizontalSizePolicy(ESizePolicy::Stretch);

		pMiddle->SetHorizontalAlignmentPolicy(EHorizontalAlignmentPolicy::Center);
		pRight->SetHorizontalAlignmentPolicy(EHorizontalAlignmentPolicy::Right);

		return { pLeft, pMiddle, pRight };
	}

	TwoSlotLayout EntityDetailsView::BuildTwoSlotLayout(HorizontalBox* aRow) noexcept
	{
		HorizontalBox* pLeft = aRow->AddWidget(RLS_NEW HorizontalBox());
		HorizontalBox* pRight = aRow->AddWidget(RLS_NEW HorizontalBox());

		pLeft->SetHorizontalSizePolicy(ESizePolicy::Stretch);
		pRight->SetHorizontalAlignmentPolicy(EHorizontalAlignmentPolicy::Right);

		return { pLeft, pRight };
	}

	void EntityDetailsView::OnAddComponentButtonClicked() noexcept
	{
		EntityComponentDefinitionRegistry* pComponentDefinitionRegistry = Editor::Get()->GetSubsystem<EntityComponentDefinitionRegistry>();

		std::flat_map<String, std::vector<Ref<IEntityComponentDefinition>>> categoryMap;

		for (const Ref<IEntityComponentDefinition>& pDefinition : pComponentDefinitionRegistry->GetAllComponentDefinitions() 
			| std::views::filter([](const auto& aDefinition) { return aDefinition->CanShowInEditor(); }))
		{
			categoryMap[pDefinition->GetCategory()].push_back(pDefinition);
		}

		for (auto&& [_, definitions] : categoryMap)
			std::ranges::sort(definitions, {}, [](const auto& aDefinition) { return aDefinition->GetDisplayName(); });

		ContextMenuBuilder builder;
		for (const auto&[category, definitions] : categoryMap)
		{
			builder.AddSection(category)
					.Font(UI::Fonts::Get("Small"))
					.SeparatorColor(Color(1.0f, 1.0f, 1.0f, 0.25f))
					.TextColor(Colors::TextInactive)
					.Thickness(0.5f);
				
			for (const Ref<IEntityComponentDefinition>& pDefinition : definitions)
			{
				builder.AddItem(pDefinition->GetDisplayName())
						.Icon(pDefinition->GetIcon())
						.Tooltip(pDefinition->GetDescription())
						.DisabledTooltip("Selection already has the given component.")
						.Enabled(std::ranges::any_of(m_Context.Entities, [this, pDefinition](entity aEntity){ return !pDefinition->Has(*m_Context.EntityManager, aEntity);}))
						.OnClicked([this, pDefinition]()
						{
							for (entity aEntity : m_Context.Entities)
							{
								if (pDefinition->Has(*m_Context.EntityManager, aEntity))
									continue;

								pDefinition->Add(*m_Context.EntityManager, aEntity);
							}

							ModuleManager::LoadModuleChecked<UIModule>().DestroyActiveContextMenu();
							RequestRefresh();
						});
			}
		}

		ModuleManager::LoadModuleChecked<UIModule>().SetActiveContextMenu(builder.BuildContextMenu());
	}

	void EntityDetailsView::OnEntityDestroyed(entity aDestroyedEntity) noexcept
	{
		const size_t numEntitiesPreRemove = m_Context.Entities.size();
		std::erase_if(m_Context.Entities, [aDestroyedEntity](entity aEntity) { return aEntity == aDestroyedEntity; });

		const bool removedAnyEntity = numEntitiesPreRemove > m_Context.Entities.size();
		if (removedAnyEntity)
		{
			SetLocked(false);
			RequestRefresh();
		}
	}

	void EntityDetailsView::OnEntityTransformed(entity aTransformedEntity) noexcept
	{
		if (std::ranges::any_of(m_Context.Entities, [aTransformedEntity](entity aEntity) { return aEntity == aTransformedEntity; }))
			RequestRefresh();
	}

	void EntityDetailsView::OnEntityComponentPropertyChanged(entity aEntity, TypeIndex aComponentType, MAYBE_UNUSED IComponent* aComponent, uint64 aProperty) noexcept
	{
		const bool nameIsDisplayed = !m_SuspendEditCallback
			&& m_Context.Entities.size() == 1
			&& m_Context.Entities.front() == aEntity
			&& aComponentType == NameComponent::StaticType()
			&& aProperty == "m_Name"_h;
		
		if (nameIsDisplayed)
			RequestRebuildHeader();
	}

	void EntityDetailsView::OnSceneChange(Scene* aCurrentScene) noexcept
	{
		if (aCurrentScene && aCurrentScene == m_pInspectedScene)
		{
			m_pInspectedScene->OnEntityDestroyed.Detach(this);
			m_pInspectedScene = nullptr;
		}
	}

	void EntityDetailsView::OnSceneChanged(Scene* aScene) noexcept
	{
		SetLocked(false);

		m_Context.Entities.clear();

		if (!aScene)
		{
			m_Context.EntityManager = nullptr;
			m_Context.Scene = nullptr;
		}
		else
		{
			m_pInspectedScene = aScene;
			m_pInspectedScene->OnEntityDestroyed.Connect(this, &EntityDetailsView::OnEntityDestroyed);
			m_Context.EntityManager = &aScene->GetEntityManager();
			m_Context.Scene = aScene;
		}
	
		RequestRefresh();
	}

	void EntityDetailsView::OnSelectionChanged(MAYBE_UNUSED entity aEntity, MAYBE_UNUSED ESelectionState aSelectionState) noexcept
	{
		if (IsLocked()) 
			return;
		
		m_Context.Entities = Editor::Get()->GetSubsystem<SelectionSubsystem>()->GetSelectedEntities();
		RequestRebuildHeader();
		RequestRefresh();
	}

	void EntityDetailsView::RequestRebuildHeader() noexcept
	{
		if (m_HeaderRebuildPending) 
			return;
		
		m_HeaderRebuildPending = true;

		Ref<EntityDetailsView> pThis(this);
		Application::Get().SubmitToMainThread([pThis]()
			{
				pThis->m_HeaderRebuildPending = false;
				pThis->BuildHeader();
			});
	}
}