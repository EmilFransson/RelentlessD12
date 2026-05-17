#pragma once
#include <Relentless.h>
#include "IEditorSubsystem.h"

namespace Relentless
{
	enum class ESelectionState : uint8 { Selected = 0, Deselected };

	class SelectionSubsystem : public IEditorSubsystem
	{
	public:
		SelectionSubsystem() noexcept = default;
		~SelectionSubsystem() noexcept override = default;

		void DeselectEntity(entity aEntityToDeselect) noexcept;
		void DeselectEntities(std::span<entity> someEntitiesToDeselect) noexcept;
		void DeselectAllEntities() noexcept;

		NO_DISCARD const std::vector<entity>& GetSelectedEntities() const noexcept;
		NO_DISCARD uint32 GetSelectedEntityCount() const noexcept;
		NO_DISCARD entity GetFirstSelected() const noexcept;

		NO_DISCARD bool IsEntitySelected(entity aEntityToQuery) const noexcept;

		NO_DISCARD bool OnLoad(ISystemManager* aSystemManager) noexcept override;
		void OnUnload(ISystemManager* aSystemManager) noexcept override;
		void OnSceneChange(MAYBE_UNUSED Scene* aScene) noexcept;

		void SelectEntity(entity aEntityToSelect) noexcept;
		void SelectEntities(std::span<entity> someEntitiesToSelect) noexcept;
		static bool ShouldCreateSubsystem(ISystemManager* aSystemManager) noexcept;

		Broadcaster<void()> OnPreDeselectAll;
		Broadcaster<void(entity, ESelectionState)> OnSelectionChanged;
	private:
		/*
			Element 0 - selected first.
			Last element - selected last.
		*/
		std::vector<entity> m_SelectedEntities;
	};
}