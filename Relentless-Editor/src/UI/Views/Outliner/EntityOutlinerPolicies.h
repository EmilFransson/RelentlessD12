#pragma once

#include <Relentless.h>
#include "../../OutlinerTableRow.h"	

namespace Relentless
{
	class EntityFolder;
	class EntityFoldersManager;

	enum class EMoveOperation : uint8 { NoOp = 0u, AttachToTarget, DetachFromTarget, ReattachToParentOfTarget };

	class EntityOutlinerPolicies
	{
	public:
		struct Context
		{
			const Scene& Scene;
			const EntityFoldersManager& FoldersManager;
			const OutlinerPayload& TargetPayload;
			Span<const entity> DraggedEntities;
			Span<EntityFolder* const> DraggedFolders;
		};	

		struct ItemResolution
		{
			OutlinerPayload Item;

			EMoveOperation MoveOperation = EMoveOperation::NoOp;
		};

		struct MovePlan
		{
			std::vector<ItemResolution> ResolvedItems;
		};

		struct ValidationResponse
		{
			String Message;
			bool IsValid = false;
		};

		NO_DISCARD MovePlan ResolveMoveRequest(const Context& aContext) const noexcept;
		NO_DISCARD ValidationResponse ValidateMoveRequest(const Context& aContext) const noexcept;
	private:
		NO_DISCARD bool AllEntitiesAreChildrenToTarget(entity aTargetEntity, Span<const entity> someSourceEntities, const Scene& aScene) const noexcept;

		NO_DISCARD bool FolderAlreadyContainsEntity(const Scene& aScene, const String& aFolderPath, Span<const entity> someSourceEntities, entity& aOutRejectedEntity) const noexcept;
		NO_DISCARD bool FolderAlreadyContainsFolder(const EntityFolder* apTargetFolder, Span<EntityFolder* const> someSourceFolders, EntityFolder*& paOutRejectedFolder) const noexcept;

		NO_DISCARD MovePlan ResolveMoveToEntity(const Context& aContext) const noexcept;
		NO_DISCARD MovePlan ResolveMoveToFolder(const Context& aContext) const noexcept;
		NO_DISCARD MovePlan ResolveMoveToRoot(const Context& aContext) const noexcept;

		NO_DISCARD bool RootAlreadyContainsEntity(Scene* pScene, Span<const entity> someSourceEntities, entity& aOutRejectedEntity) const noexcept;

		NO_DISCARD ValidationResponse ValidateMoveToEntity(const Context& aContext) const;
		NO_DISCARD ValidationResponse ValidateMoveToFolder(const Context& aContext) const;
		NO_DISCARD ValidationResponse ValidateMoveToScene(const Context& aContext) const;

		NO_DISCARD bool WouldCreateCycle(entity aTargetEntity, Span<const entity> someSourceEntities, const Scene& aScene) const noexcept;
		NO_DISCARD bool WouldCreateCycle(const EntityFolder* pTargetFolder, Span<EntityFolder* const> someSourceFolders) const noexcept;
		NO_DISCARD bool WouldCreateNameCollision(const EntityFoldersManager& aFoldersManager, const Scene& aScene, const EntityFolder* pTargetFolder, Span<EntityFolder* const> someSourceFolders, EntityFolder*& paOutRejectedFolder) const noexcept;
	private:
	};
}