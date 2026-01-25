#pragma once
#include "UI/OutlinerTableRow.h"	

namespace Relentless
{
	class EntityFolder;
	class EntityFoldersSubsystem;

	enum class EMoveOperation			: uint8 { NoOp = 0u, AttachToTarget, DetachFromTarget, ReattachToParentOfTarget };
	enum class EPostDuplicateOperation	: uint8 { NoOp = 0u, AttachToDuplicatedParentItem };

	class EntityOutlinerPolicies
	{
	public:
		struct DuplicateContext
		{
			const Scene& Scene;
			const EntityFoldersSubsystem& FoldersSubsystem;
			Span<const entity> Entities;
			Span<EntityFolder* const> Folders;
		};

		struct MoveContext
		{
			const Scene& Scene;
			const EntityFoldersSubsystem& FoldersSubsystem;
			const OutlinerPayload& TargetPayload;
			Span<const entity> Entities;
			Span<EntityFolder* const> Folders;
		};	

		struct ItemMoveResolution
		{
			OutlinerPayload Item;
			EMoveOperation MoveOperation = EMoveOperation::NoOp;
		};

		struct ItemDuplicateResolution
		{
			OutlinerPayload Item;
			EPostDuplicateOperation DuplicateOperation = EPostDuplicateOperation::NoOp;
		};

		struct DuplicatePlan
		{
			std::vector<ItemDuplicateResolution> ResolvedItems;
		};

		struct MovePlan
		{
			std::vector<ItemMoveResolution> ResolvedItems;
		};

		struct ValidationResponse
		{
			String Message;
			bool IsValid = false;
		};

		NO_DISCARD DuplicatePlan ResolveDuplicateRequest(const DuplicateContext& aContext) const noexcept;
		NO_DISCARD MovePlan ResolveMoveRequest(const MoveContext& aContext) const noexcept;
		NO_DISCARD ValidationResponse ValidateMoveRequest(const MoveContext& aContext) const noexcept;
	private:
		NO_DISCARD bool AllEntitiesAreChildrenToTarget(entity aTargetEntity, Span<const entity> someSourceEntities, const Scene& aScene) const noexcept;

		NO_DISCARD bool FolderAlreadyContainsEntity(const Scene& aScene, const String& aFolderPath, Span<const entity> someSourceEntities, entity& aOutRejectedEntity) const noexcept;
		NO_DISCARD bool FolderAlreadyContainsFolder(const EntityFolder* apTargetFolder, Span<EntityFolder* const> someSourceFolders, EntityFolder*& paOutRejectedFolder) const noexcept;

		NO_DISCARD MovePlan ResolveMoveToEntity(const MoveContext& aContext) const noexcept;
		NO_DISCARD MovePlan ResolveMoveToFolder(const MoveContext& aContext) const noexcept;
		NO_DISCARD MovePlan ResolveMoveToRoot(const MoveContext& aContext) const noexcept;

		NO_DISCARD bool RootAlreadyContainsEntity(Scene* pScene, Span<const entity> someSourceEntities, entity& aOutRejectedEntity) const noexcept;

		NO_DISCARD ValidationResponse ValidateMoveToEntity(const MoveContext& aContext) const;
		NO_DISCARD ValidationResponse ValidateMoveToFolder(const MoveContext& aContext) const;
		NO_DISCARD ValidationResponse ValidateMoveToScene(const MoveContext& aContext) const;

		NO_DISCARD bool WouldCreateCycle(entity aTargetEntity, Span<const entity> someSourceEntities, const Scene& aScene) const noexcept;
		NO_DISCARD bool WouldCreateCycle(const EntityFolder* pTargetFolder, Span<EntityFolder* const> someSourceFolders) const noexcept;
		NO_DISCARD bool WouldCreateNameCollision(const EntityFoldersSubsystem& aFoldersSubsystem, const Scene& aScene, const EntityFolder* pTargetFolder, Span<EntityFolder* const> someSourceFolders, EntityFolder*& paOutRejectedFolder) const noexcept;
	private:
	};
}