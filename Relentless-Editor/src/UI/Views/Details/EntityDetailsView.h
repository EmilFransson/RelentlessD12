#pragma once
#include "IDetailsView.h"

#include "UI/Views/Details/Context/EntityDetailsContext.h"

namespace Relentless
{
	enum class ESelectionState : uint8;

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
		void OnEntityDestroyed(entity aDestroyedEntity) noexcept;
		void OnEntityTransformed(entity aTransformedEntity) noexcept;
		void OnSceneChanged(Scene* aScene) noexcept;
		void OnSelectionChanged(entity aEntity, ESelectionState aSelectionState) noexcept;
	private:
		EntityDetailsContext m_Context;
		Scene* m_pInspectedScene = nullptr;
		bool m_IsLocked = false;
		bool m_RequestedRefresh = true;
	};
}