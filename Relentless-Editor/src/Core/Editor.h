#pragma once
#include <Relentless.h>
#include "../Panels/OutlinerPanel.h"
#include "../Panels/PropertiesPanel.h"
#include "../Panels/ContentBrowserPanel.h"
#include "../Panels/MetricsPanel.h" 
#include "../Panels/SceneRendererPanel.h"
#include "../Panels/InspectorPanel.h"

#include "EntityFilters.h"
#include "Selection.h"

namespace Relentless
{
	enum class EGizmoType : int8_t { None = -1, Translate = 0, Rotate = 1, Scale = 2 };
	enum class EGizmoMode : uint8_t { Local = 0, World };

	enum class ESceneState : uint8_t { Edit = 0, Play, Simulate };

	class Editor
	{
	public:
		Editor() noexcept = default;
		virtual ~Editor() noexcept = default;

		virtual void OnEvent(IEvent& event) noexcept;
		virtual void OnImGuiRender() noexcept;
		virtual void OnCreate() noexcept;
		virtual void OnDestroy() noexcept;
		virtual void OnUpdate(const float deltaTime) noexcept;
		virtual void OnRender() noexcept;
		virtual void OnPostRender() noexcept;

		[[nodiscard]] Selection& GetSelection() noexcept;
		[[nodiscard]] EntityFiltersManager& GetEntityFiltersManager() noexcept;

		Broadcaster<void(Scene*)> OnSceneChanged;

	private:
		void SetActiveScene(const std::shared_ptr<Scene>& pScene) noexcept;
		void LoadStarterMeshes() noexcept;
		void CreateStartScene() noexcept;
		void OnSceneViewportChanged() noexcept;
		void ManipulateTransformGizmo() noexcept;

		void SetSceneContext(std::shared_ptr<Scene> pScene) noexcept;

		void UI_DrawStatisticsPanel() noexcept;
		void UI_DrawMainMenuBar() noexcept;

		void CreateEntityFromDroppedMesh(const AssetHandle& meshHandle) noexcept;
		void OnEntitySelectionChanged(entity e, ESelectionState selectionState);
		void OnEntityPreDestroyed(entity e) noexcept;
	private:
		bool m_SceneViewportChanged = false;
		bool m_HoveringSceneViewport = false;
		ImVec2 m_ViewportPanelSize = ImVec2(800.0f, 600.0f);
		ImVec2 vMin;
		ImVec2 vMax;

		entity m_HoveredEntity = NULL_ENTITY;
		EGizmoType m_CurrentGizmoType = EGizmoType::None;
		EGizmoMode m_CurrentGizmoMode = EGizmoMode::World;

		std::unique_ptr<OutlinerPanel> m_pOutlinerPanel = nullptr;
		PropertiesPanel m_PropertiesPanel;
		ContentBrowserPanel m_ContentBrowserPanel;
		MetricsPanel m_MetricsPanel;
		SceneRendererPanel m_SceneRendererPanel;
		InspectorPanel m_InspectorPanel;

		std::shared_ptr<Scene> m_pActiveScene = nullptr;
		std::shared_ptr<Scene> m_pEditorScene = nullptr;

		std::shared_ptr<SceneRenderer> m_pSceneRenderer = nullptr;
		std::shared_ptr<UtilityRenderer> m_pUtilityRenderer = nullptr;

		bool m_DisplayOutlinerPanel = true;
		bool m_DisplayContentBrowserPanel = true;
		bool m_DisplayPropertiesPanel = true;
		bool m_DisplayInspectorPanel = false;
		bool m_DisplayMetricsPanel = true;
		bool m_DisplaySceneRendererPanel = true;
		bool m_DisplayStatisticsPanel = true;

		bool m_ImmersiveModeEnabled = false;

		AssetHandle m_PlayButtonTextureHandle = NULL_HANDLE;
		AssetHandle m_StopButtonTextureHandle = NULL_HANDLE;
		AssetHandle m_PauseButtonTextureHandle = NULL_HANDLE;
		AssetHandle m_SimulateButtonTextureHandle = NULL_HANDLE;
		AssetHandle m_StepButtonTextureHandle = NULL_HANDLE;

		ESceneState m_SceneState = ESceneState::Edit;

		std::shared_ptr<TextureCube> m_SkyBox = nullptr;

		Selection m_Selection;
		EntityFiltersManager m_EntityFiltersManager;

		bool m_IsPanningMouse = false;
		bool m_ViewportIsFocused = false;
	};
}
