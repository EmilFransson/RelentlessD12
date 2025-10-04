#include "EntityOutlinerPolicies.h"

#include "../../../Core/EntityFolders.h"

namespace Relentless
{
	static NO_DISCARD std::string_view locGetEntityName(entity aEntity, const Scene& aScene)
	{
		return aScene.GetEntityManager().Get<NameComponent>(aEntity).Name;
	}

	EntityOutlinerPolicies::MovePlan EntityOutlinerPolicies::ResolveMoveRequest(const Context& aContext) const noexcept
	{
		if (std::holds_alternative<entity>(aContext.TargetPayload))
			return ResolveMoveToEntity(aContext);
		else if (std::holds_alternative<EntityFolder*>(aContext.TargetPayload))
			return ResolveMoveToFolder(aContext);
		else if (std::holds_alternative<Scene*>(aContext.TargetPayload))
			return ResolveMoveToRoot(aContext);

		RLS_ASSERT(false, "[EntityOutlinerPolicies::ResolveMoveRequest]: Unreachable.");
		return {};
	}

	EntityOutlinerPolicies::ValidationResponse EntityOutlinerPolicies::ValidateMoveRequest(const Context& aContext) const noexcept
	{
		if (std::holds_alternative<entity>(aContext.TargetPayload))
			return ValidateMoveToEntity(aContext);
		else if (std::holds_alternative<EntityFolder*>(aContext.TargetPayload))
			return ValidateMoveToFolder(aContext);
		else if (std::holds_alternative<Scene*>(aContext.TargetPayload))
			return ValidateMoveToScene(aContext);

		RLS_ASSERT(false, "[EntityOutlinerPolicies::ValidateMoveRequest]: Unreachable.");
		return { "Invalid target.", false };
	}


	EntityOutlinerPolicies::ValidationResponse EntityOutlinerPolicies::ValidateMoveToEntity(const Context& aContext) const
	{
		const entity targetEntity = std::get<entity>(aContext.TargetPayload);
		const std::string_view targetEntityName = locGetEntityName(targetEntity, aContext.Scene);

		if (WouldCreateCycle(targetEntity, aContext.DraggedEntities, aContext.Scene))
			return { std::format("{}. Parent cannot become the child of their descendant.", targetEntityName), false };
		else if (aContext.DraggedFolders.GetSize() > 0)
			return { std::format("{}.", targetEntityName), false };
		
		return { std::format("{}.", targetEntityName), true };
	}

	EntityOutlinerPolicies::ValidationResponse EntityOutlinerPolicies::ValidateMoveToFolder(const Context& aContext) const
	{
		const EntityFolder* pTargetFolder = std::get<EntityFolder*>(aContext.TargetPayload);
		const std::string_view targetFolderLabel = pTargetFolder->GetLabel();

		entity outRejectedEntity = NULL_ENTITY;
		EntityFolder* pOutRejectedFolder = nullptr;

		if (FolderAlreadyContainsEntity(aContext.Scene, pTargetFolder->GetPath(), aContext.DraggedEntities, outRejectedEntity))
			return { std::format("{} is already assigned to {}.", locGetEntityName(outRejectedEntity , aContext.Scene), pTargetFolder->GetLabel()), false };
		else if (WouldCreateCycle(pTargetFolder, aContext.DraggedFolders))
			return { std::format("{}. Parent cannot become the child of their descendant.", targetFolderLabel), false };
		else if (FolderAlreadyContainsFolder(pTargetFolder, aContext.DraggedFolders, pOutRejectedFolder))
			return { std::format("{} is already assigned to {}.", pOutRejectedFolder->GetLabel(), targetFolderLabel), false };
		else if (WouldCreateNameCollision(aContext.FoldersManager, aContext.Scene, pTargetFolder, aContext.DraggedFolders, pOutRejectedFolder))
			return { std::format("{} already contains a folder child named '{}'.", pTargetFolder->GetLabel(), pOutRejectedFolder->GetLabel()), false };

		return { std::format("Move into '{}'.", targetFolderLabel), true };
	}

	EntityOutlinerPolicies::ValidationResponse EntityOutlinerPolicies::ValidateMoveToScene(const Context& aContext) const
	{
		Scene* pTargetScene = std::get<Scene*>(aContext.TargetPayload);
		const String& targetSceneName = pTargetScene->GetName();

		entity outRejectedEntity = NULL_ENTITY;
		
		if (aContext.DraggedFolders.GetSize() > 0)
			return { std::format("{}.", targetSceneName), false };
		else if (RootAlreadyContainsEntity(pTargetScene, aContext.DraggedEntities, outRejectedEntity))
			return { std::format("{} is already assigned to root.", locGetEntityName(outRejectedEntity, *pTargetScene)), false };

		return { "Move to root.", true };
	}

	bool EntityOutlinerPolicies::AllEntitiesAreChildrenToTarget(entity aTargetEntity, Span<const entity> someSourceEntities, const Scene& aScene) const noexcept
	{
		for (entity sourceEntity : someSourceEntities)
		{
			if (!aScene.EntityIsChild(sourceEntity, aTargetEntity))
				return false;
		}

		return true;
	}

	bool EntityOutlinerPolicies::FolderAlreadyContainsEntity(const Scene& aScene, const String& aFolderPath, Span<const entity> someSourceEntities, entity& aOutRejectedEntity) const noexcept
	{
		const EntityManager& manager = aScene.GetEntityManager();

		for (entity sourceEntity : someSourceEntities)
		{
			if (manager.Has<FolderComponent>(sourceEntity) && manager.Get<FolderComponent>(sourceEntity).Folder.GetPath() == aFolderPath)
			{
				aOutRejectedEntity = sourceEntity;
				return true;
			}
		}

		return false;
	}

	bool EntityOutlinerPolicies::FolderAlreadyContainsFolder(const EntityFolder* apTargetFolder, Span<EntityFolder* const> someSourceFolders, EntityFolder*& paOutRejectedFolder) const noexcept
	{
		for (EntityFolder* pSourceFolder : someSourceFolders)
		{
			if (pSourceFolder->GetParent() == apTargetFolder)
			{
				paOutRejectedFolder = pSourceFolder;
				return true;
			}
		}

		return false;
	}

	EntityOutlinerPolicies::MovePlan EntityOutlinerPolicies::ResolveMoveToEntity(const Context& aContext) const noexcept
	{
		/*
			We know:
			1. No folders are being dragged.
			2. No cycles will be created.

			We need to resolve:
			1. If ALL entities are direct children of the target entity, then resolution is:
				1.1 Detach each entity from the target entity.
				1.2 Reattach them all to target entity's folder (if it has one).
			2. If SOME entities are direct children of the target entity, then resolution is:
				2.1 No-Op for the direct children.
				2.2 (Re-)attach the rest to target entity.
			3. If NO entities are direct children of the target entity, then resolution is:
				3.1 Attach all entities to target entity.
		*/
		const entity targetEntity = std::get<entity>(aContext.TargetPayload);
		const int numDraggedEntities = static_cast<int>(aContext.DraggedEntities.GetSize());

		const EntityManager& manager = aContext.Scene.GetEntityManager();
		const bool targetHasFolder = manager.Has<FolderComponent>(targetEntity);

		MovePlan movePlan;
		movePlan.ResolvedItems.reserve(numDraggedEntities);

		auto&& GetIntersectingEntities = [&aContext, targetEntity]() -> std::unordered_set<entity>
		{
			std::unordered_set<entity> intersectingEntities;
			for (entity draggedEntity : aContext.DraggedEntities)
			{
				if (aContext.Scene.EntityIsParent(draggedEntity, targetEntity))
					intersectingEntities.insert(draggedEntity);
			}

			return intersectingEntities;
		};

		const std::unordered_set<entity> intersectingEntities = GetIntersectingEntities();
		if (intersectingEntities.size() == aContext.DraggedEntities.GetSize())
		{
			//Case 1:
			for (entity draggedEntity : aContext.DraggedEntities)
			{
				ItemResolution resolution;
				resolution.Item = OutlinerPayload(draggedEntity);
				if (targetHasFolder)
					resolution.MoveOperation = EMoveOperation::ReattachToParentOfTarget;
				else
					resolution.MoveOperation = EMoveOperation::DetachFromTarget;
				movePlan.ResolvedItems.push_back(resolution);
			}
		}
		else if (!intersectingEntities.empty())
		{
			//Case 2:
			for (entity draggedEntity : aContext.DraggedEntities)
			{
				ItemResolution resolution;
				resolution.Item = OutlinerPayload(draggedEntity);
				if (intersectingEntities.contains(draggedEntity))
					resolution.MoveOperation = EMoveOperation::NoOp;
				else
					resolution.MoveOperation = EMoveOperation::AttachToTarget;
				movePlan.ResolvedItems.push_back(resolution);
			}
		}
		else
		{
			//Case 3:
			for (entity draggedEntity : aContext.DraggedEntities)
			{
				ItemResolution resolution;
				resolution.Item = OutlinerPayload(draggedEntity);
				resolution.MoveOperation = EMoveOperation::AttachToTarget;
				movePlan.ResolvedItems.push_back(resolution);
			}
		}

		return movePlan;
	}

	EntityOutlinerPolicies::MovePlan EntityOutlinerPolicies::ResolveMoveToFolder(const Context& aContext) const noexcept
	{
		/*
			We know:
			1. Both folders and entities can be part of the dragged items.
			2. No entities dragged currently belong to the target folder.
			3. No cycles will be created.
			4. No name collisions among folders will be created.
			5. No entities or folders dragged currently belong to the target folder.

			We need to resolve:
			1. If an entity is a descendant of another dragged entity, then resolution is:
				1.1 No-Op for the descendant entity.

			Note: Resolution items must be sorted such that entities come before folders, to ensure that entities are moved before folders.
		*/

		const EntityFolder* pTargetFolder = std::get<EntityFolder*>(aContext.TargetPayload);

		const uint32 numDraggedEntities = aContext.DraggedEntities.GetSize();
		const uint32 numDraggedFolders = aContext.DraggedFolders.GetSize();

		MovePlan movePlan;
		movePlan.ResolvedItems.reserve(numDraggedEntities + numDraggedFolders);

		auto&& GetDescendantEntities = [&scene = aContext.Scene, &aContext]() -> std::unordered_set<entity>
		{
			std::unordered_set<entity> descendantEntities;

			for (entity candidateEntityDescendant : aContext.DraggedEntities)
			{
				for (entity candidateEntityAncestor : aContext.DraggedEntities)
				{
					if (candidateEntityDescendant == candidateEntityAncestor)
						continue;

					if (scene.EntityIsDescendant(candidateEntityAncestor, candidateEntityDescendant))
					{
						descendantEntities.insert(candidateEntityDescendant);
						break;
					}
				}
			}

			return descendantEntities;
		};

		std::unordered_set<entity> descendantEntities = GetDescendantEntities();

		for (entity draggedEntity : aContext.DraggedEntities)
		{
			ItemResolution resolution;
			resolution.Item = OutlinerPayload(draggedEntity);
			if (descendantEntities.contains(draggedEntity))
				resolution.MoveOperation = EMoveOperation::NoOp;
			else
				resolution.MoveOperation = EMoveOperation::AttachToTarget;
			movePlan.ResolvedItems.push_back(resolution);
		}

		for (EntityFolder* pFolder : aContext.DraggedFolders)
		{
			ItemResolution resolution;
			resolution.Item = OutlinerPayload(pFolder);
			resolution.MoveOperation = EMoveOperation::AttachToTarget;
			movePlan.ResolvedItems.push_back(resolution);
		}

		return movePlan;
	}

	EntityOutlinerPolicies::MovePlan EntityOutlinerPolicies::ResolveMoveToRoot(const Context& aContext) const noexcept
	{
		return {};
	}

	bool EntityOutlinerPolicies::RootAlreadyContainsEntity(Scene* pScene, Span<const entity> someSourceEntities, entity& aOutRejectedEntity) const noexcept
	{
		for (entity sourceEntity : someSourceEntities)
		{
			if (!pScene->EntityHasAncestors(sourceEntity) && !pScene->GetEntityManager().Has<FolderComponent>(sourceEntity))
			{
				aOutRejectedEntity = sourceEntity;
				return true;
			}
		}

		return false;
	}

	bool EntityOutlinerPolicies::WouldCreateCycle(entity aTargetEntity, Span<const entity> someSourceEntities, const Scene& aScene) const noexcept
	{
		for (entity sourceEntity : someSourceEntities)
		{
			if (sourceEntity == aTargetEntity)
				return true;
			
			if (aScene.EntityIsDescendant(sourceEntity, aTargetEntity))
				return true;
		}

		return false;
	}

	bool EntityOutlinerPolicies::WouldCreateCycle(const EntityFolder* pTargetFolder, Span<EntityFolder* const> someSourceFolders) const noexcept
	{
		const String targetFolderPath = pTargetFolder->GetPath();

		for (const EntityFolder* pSourceFolder : someSourceFolders)
		{
			if (FilePath::IsSubpath(targetFolderPath, pSourceFolder->GetPath()))
				return true;
		}

		return false;
	}

	bool EntityOutlinerPolicies::WouldCreateNameCollision(const EntityFoldersManager& aFoldersManager, const Scene& aScene, const EntityFolder* pTargetFolder, Span<EntityFolder* const> someSourceFolders, EntityFolder*& paOutRejectedFolder) const noexcept
	{
		const String targetFolderPath = pTargetFolder->GetPath();

		for (EntityFolder* pSourceFolder : someSourceFolders)
		{
			if (aFoldersManager.GetFolderName(aScene, targetFolderPath, pSourceFolder->GetLabel()) != pSourceFolder->GetLabel())
			{
				paOutRejectedFolder = pSourceFolder;
				return true;
			}
		}

		return false;
	}

}

