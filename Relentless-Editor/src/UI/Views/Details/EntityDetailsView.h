#pragma once
#include "IDetailsView.h"

#include "UI/Views/Details/Context/EntityDetailsContext.h"

namespace Relentless
{
	enum class ESelectionState : uint8;

	struct TwoSlotLayout
	{
		HorizontalBox* pLeftBox = nullptr;
		HorizontalBox* pRightBox = nullptr;
	};

	struct ThreeSlotLayout
	{
		HorizontalBox* pLeftBox = nullptr;
		HorizontalBox* pMiddleBox = nullptr;
		HorizontalBox* pRightBox = nullptr;
	};

	class EntityDetailsView : public IDetailsView
	{
	public:
		EntityDetailsView() noexcept;
		virtual ~EntityDetailsView() noexcept override;

		NO_DISCARD bool IsLocked() const noexcept;

		void SetLocked(bool aLock) noexcept;
	protected:
		virtual void OnPreRequestSource(bool aFromManualTrigger) noexcept override;
	private:
		void BuildEmptyHeader(HorizontalBox* aRow) noexcept;
		void BuildHeader() noexcept;
		void BuildSingleEntityHeader(HorizontalBox* aRow, entity aEntity) noexcept;
		void BuildMultiEntityHeader(HorizontalBox* aRow, uint32 aEntityCount) noexcept;
		NO_DISCARD Ref<Button> BuildLockButton() noexcept;
		NO_DISCARD ThreeSlotLayout BuildThreeSlotLayout(HorizontalBox* aRow) noexcept;
		NO_DISCARD TwoSlotLayout BuildTwoSlotLayout(HorizontalBox* aRow) noexcept;

		void OnEntityDestroyed(entity aDestroyedEntity) noexcept;
		void OnEntityTransformed(entity aTransformedEntity) noexcept;
		void OnEntityComponentPropertyChanged(entity aEntity, TypeIndex aComponentType, MAYBE_UNUSED IComponent* aComponent, uint64 aProperty) noexcept;
		void OnSceneChange(Scene* aCurrentScene) noexcept;
		void OnSceneChanged(Scene* aScene) noexcept;
		void OnSelectionChanged(MAYBE_UNUSED entity aEntity, MAYBE_UNUSED ESelectionState aSelectionState) noexcept;

		void RequestRebuildHeader() noexcept;
	private:
		EntityDetailsContext m_Context;
		Scene* m_pInspectedScene = nullptr;
		bool m_IsLocked = false;
		bool m_SuspendEditCallback = false;
		bool m_HeaderRebuildPending = false;
	};
}