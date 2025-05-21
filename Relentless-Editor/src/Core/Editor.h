#pragma once
#include <Relentless.h>
#include "../Panels/DetailsPanel.h"
#include "../Panels/OutlinerPanel.h"
#include "../Panels/PropertiesPanel.h"
#include "../Panels/ContentBrowserPanel.h"
#include "../Panels/MetricsPanel.h" 
#include "../Panels/SceneRendererPanel.h"
#include "../Panels/InspectorPanel.h"
#include "../Panels/ViewportPanel.h"

#include "EntityFilters.h"
#include "Selection.h"

namespace Relentless
{
	enum class ESceneState : uint8 { Edit = 0, Play, Simulate };

	class Editor
	{
	public:
		Editor() noexcept = default;
		virtual ~Editor() noexcept;
		virtual void OnEvent(IEvent& event) noexcept;
		virtual void OnImGuiRender() noexcept;
		virtual void OnCreate() noexcept;
		virtual void OnDestroy() noexcept;
		virtual void OnUpdate(const float deltaTime) noexcept;
		virtual void OnRender() noexcept;

		[[nodiscard]] Scene* GetActiveScene() const noexcept;
		[[nodiscard]] const UniquePtr<Selection>& GetSelection() noexcept;
		[[nodiscard]] const UniquePtr<EntityFiltersManager>& GetEntityFiltersManager() noexcept;
		[[nodiscard]] ViewportRenderView& GetRenderView(uint32 renderViewIndex) noexcept;
		[[nodiscard]] std::vector<ViewportRenderView>& GetRenderViews() noexcept;

		[[nodiscard]] std::vector<entity> GetTransformSelection() const noexcept;

		void OnViewportEntityDuplicationRequest() noexcept;

		Broadcaster<void(Scene*)> OnSceneChanged;
	private:
		[[nodiscard]] bool IsHoveringAnyFocusedViewport() const noexcept;
		[[nodiscard]] bool IsNavigatingAnyViewport() const noexcept;

		[[nodiscard]] ViewportPanel* GetHoveredViewport() const noexcept;

		void SetActiveScene(const std::shared_ptr<Scene>& pScene) noexcept;
		void LoadStarterMeshes() noexcept;

		void CreateDetailsPanel() noexcept;
		void CreateStartScene() noexcept;

		void UI_DrawStatisticsPanel() noexcept;
		void UI_DrawMainMenuBar() noexcept;

		void CreateEntityFromDroppedMesh(const AssetHandle& meshHandle) noexcept;
		void OnEntitySelectionChanged(entity e, ESelectionState selectionState);
		void OnEntityPreDestroyed(entity e) noexcept;
		void OnEntityAttached(entity child, entity parent) noexcept;
		void OnEntityReadbackDone(uint32 entityID) noexcept;

		void OnPanelCreated(PanelBase* pPanel) noexcept;
		void OnPanelGainedFocus(PanelBase* pPanel) noexcept;

		void OnViewportHotkeyPressed(ViewportPanel* pPanel, RLS_Key key) noexcept;
		void OnViewportClicked(ViewportPanel* pPanel, Vector2u relativeMouseCoords) noexcept;

		void OnUnhandledEvent(IEvent& event);

		void SpawnViewport() noexcept;
	private:
		std::vector<ViewportRenderView> m_RenderViews;
		std::vector<UniquePtr<ViewportPanel>> m_EditorViewports;

		std::vector<PanelBase*> m_PanelStack;

		entity m_HoveredEntity = NULL_ENTITY;

		UniquePtr<Selection> m_pSelection = nullptr;
		UniquePtr<EntityFiltersManager> m_pEntityFiltersManager = nullptr;

		//PropertiesPanel m_PropertiesPanel;
		//ContentBrowserPanel m_ContentBrowserPanel;
		//MetricsPanel m_MetricsPanel;
		//SceneRendererPanel m_SceneRendererPanel;
		//InspectorPanel m_InspectorPanel;

		std::shared_ptr<Scene> m_pActiveScene = nullptr;
		std::shared_ptr<Scene> m_pEditorScene = nullptr;

		std::shared_ptr<UtilityRenderer> m_pUtilityRenderer = nullptr;
		
		UniquePtr<OutlinerPanel> m_pOutlinerPanel = nullptr;
		UniquePtr<DetailsPanel> m_pDetailsPanel;

		bool m_DisplayOutlinerPanel = true;
		bool m_DisplayContentBrowserPanel = true;
		bool m_DisplayPropertiesPanel = true;
		bool m_DisplayInspectorPanel = false;
		bool m_DisplayMetricsPanel = true;
		bool m_DisplaySceneRendererPanel = true;
		bool m_DisplayStatisticsPanel = true;

		bool m_ImmersiveModeEnabled = false;
		bool m_AllowMouseConfinement = true;

		AssetHandle m_PlayButtonTextureHandle = NULL_HANDLE;
		AssetHandle m_StopButtonTextureHandle = NULL_HANDLE;
		AssetHandle m_PauseButtonTextureHandle = NULL_HANDLE;
		AssetHandle m_SimulateButtonTextureHandle = NULL_HANDLE;
		AssetHandle m_StepButtonTextureHandle = NULL_HANDLE;

		ESceneState m_SceneState = ESceneState::Edit;

		std::shared_ptr<TextureCube> m_SkyBox = nullptr;
		
		Ref<HorizontalBox> m_pHorizontalBox = nullptr;

		Ref<CollapsibleSection> m_pSection = nullptr;
		Ref<FloatDrag> m_pDragger = nullptr;
	};
}
