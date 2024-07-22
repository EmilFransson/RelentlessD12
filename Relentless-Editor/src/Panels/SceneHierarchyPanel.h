#pragma once
#include <Relentless.h>
namespace Relentless
{
	class SceneHierarchyPanel
	{
	public:
		explicit SceneHierarchyPanel() noexcept;
		~SceneHierarchyPanel() noexcept = default;
		void OnEvent(IEvent& event);
		void OnImGuiRender(const bool show) noexcept;
		void SetActiveScene(Scene* const pScene) noexcept;
		void DrawEntityNode(const entity entityID) noexcept;
		void SetSelectedEntity(const entity entityID) noexcept;
		void SetOnEntityDestroyFunction(std::function<void(entity id)> callBackFunction) noexcept;
		void SetOnEntityCreatedFunction(std::function<void(entity id)> callBackFunction) noexcept;
		void SetOnEntitySelectedFunction(std::function<void(entity id)> callBackFunction) noexcept;
		[[nodiscard]] constexpr const entity GetSelectedEntity() const noexcept { return m_SelectedEntity; }
		[[nodiscard]] bool IsEntitySelected(entity entity) const noexcept;
		void DeselectEntity(entity entity) noexcept;
		void SelectEntity(entity entity) noexcept;
	private:
		void OnTableEntryClicked() noexcept;
		void OnMouseReleasedOverTableEntry() noexcept;
		void OnSelectAll() noexcept;
		[[nodiscard]] bool DrawTableEntry(entity currentEntity) noexcept;
		[[nodiscard]] bool DrawTableRootEntry(bool& outIsOpen) noexcept;
		void DrawDraggingTooltip() noexcept;
	private:
		Scene* m_pScene;
		entity m_SelectedEntity;
		std::function<void(entity)> m_OnEntityDestroyedCallBack;
		std::function<void(entity)> m_OnEntityCreatedCallBack;
		std::function<void(entity)> m_OnEntitySelectedCallBack;
		entity m_EntityScheduledForDestruction;

		AssetHandle m_ShowEntityTextureIconHandle = NULL_HANDLE;
		AssetHandle m_HideEntityTextureIconHandle = NULL_HANDLE;

		std::vector<entity> m_SelectedEntities;
		entity m_HoveredEntity = NULL_ENTITY;
		entity m_DraggedEntity = NULL_ENTITY;

		bool m_IsTableFocused = false;
		bool m_IsTableHovered = false;

		std::string m_ContentFilter{};

		bool m_SceneTableEntrySelected = false;
		bool m_SceneTableEntryIsHovered = false;
		bool m_SceneIsHiddenInGame = false;
	};
}