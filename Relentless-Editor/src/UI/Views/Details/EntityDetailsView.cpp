#include "EntityDetailsView.h"

#include "Core/Editor.h"

#include "Subsystem/SelectionSubsystem.h"

namespace Relentless
{
	EntityDetailsView::EntityDetailsView() noexcept
	{
		SetHorizontalSizePolicy(ESizePolicy::Stretch);
		SetVerticalSizePolicy(ESizePolicy::Stretch);

		Editor::OnEntityTransformed.Connect(this, &EntityDetailsView::OnEntityTransformed);

		Editor* pEditor = Editor::Get();
		pEditor->OnSceneChanged.Connect(this, &EntityDetailsView::OnSceneChanged);
		pEditor->GetSubsystem<SelectionSubsystem>()->OnSelectionChanged.Connect(this, &EntityDetailsView::OnSelectionChanged);

		SetContext(&m_Context);
	}

	EntityDetailsView::~EntityDetailsView() noexcept
	{
		if (Editor* pEditor = Editor::Get())
		{
			if (const auto& pSelection = pEditor->GetSubsystem<SelectionSubsystem>())
				pSelection->OnSelectionChanged.Detach(this);

			pEditor->OnSceneChanged.Detach(this);
		}

		if (m_pInspectedScene)
		{
			m_pInspectedScene->OnEntityDestroyed.Detach(this);
			m_pInspectedScene = nullptr;
		}

		Editor::OnEntityTransformed.Detach(this);
	}

	bool EntityDetailsView::IsLocked() const noexcept
	{
		return m_IsLocked;
	}

	void EntityDetailsView::SetLocked(bool aLock) noexcept
	{
		m_IsLocked = aLock;
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

	void EntityDetailsView::OnSceneChanged(Scene* aScene) noexcept
	{
		SetLocked(false);

		if (m_pInspectedScene)
		{
			m_pInspectedScene->OnEntityDestroyed.Detach(this);
			m_pInspectedScene = nullptr;
		}

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

	void EntityDetailsView::OnSelectionChanged(entity aEntity, ESelectionState aSelectionState) noexcept
	{
		if (IsLocked())
			return;

		if (aSelectionState == ESelectionState::Selected)
			m_Context.Entities.push_back(aEntity);
		else
			m_Context.Entities.erase(std::remove(m_Context.Entities.begin(), m_Context.Entities.end(), aEntity), m_Context.Entities.end());
		
		RequestRefresh();
	}
}