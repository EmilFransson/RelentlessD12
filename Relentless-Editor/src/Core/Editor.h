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

#include "EntityFolders.h"
#include "Selection.h"

namespace Relentless
{
	enum class ESceneState : uint8 { Edit = 0, Play, Simulate };

	class Editor : public std::enable_shared_from_this<Editor>
	{
	public:
		Editor() noexcept = default;
		virtual ~Editor() noexcept;

		NO_DISCARD const Ref<EntityOutlinerView> GetEntityOutlinerView() const noexcept;

		virtual void OnEvent(IEvent& event) noexcept;
		virtual void OnImGuiRender() noexcept;
		virtual void OnCreate() noexcept;
		virtual void OnDestroy() noexcept;
		virtual void OnUpdate(const float deltaTime) noexcept;
		virtual void OnRender() noexcept;

		NO_DISCARD Scene* GetActiveScene() const noexcept;
		NO_DISCARD EntityFolder* GetFolderContainingEntity(entity aEntity) const noexcept;
		NO_DISCARD const UniquePtr<Selection>& GetSelection() noexcept;
		NO_DISCARD const UniquePtr<EntityFoldersManager>& GetEntityFoldersManager() noexcept;
		NO_DISCARD ViewportRenderView& GetRenderView(uint32 renderViewIndex) noexcept;
		NO_DISCARD std::vector<ViewportRenderView>& GetRenderViews() noexcept;

		NO_DISCARD std::vector<entity> GetTransformSelection() const noexcept;

		void OnViewportEntityDuplicationRequest() noexcept;

		void SetVisibilityForSelectedEntities(bool aVisibilityState) noexcept;

		inline static Broadcaster<void(entity aEntity)> OnEntityTransformed;
		inline static Broadcaster<void()> OnShutDown;
		Broadcaster<void(Scene*)> OnPreSceneChanged;
		Broadcaster<void(Scene*)> OnSceneChanged;

	private:
		void SetActiveScene(const std::shared_ptr<Scene>& pScene) noexcept;
		void LoadStarterMeshes() noexcept;

		void CreateStartScene() noexcept;

		void UI_DrawMainMenuBar() noexcept;

		void CreateEntityFromDroppedMesh(const AssetHandle& meshHandle) noexcept;
		void OnEntityFolderDeleted(EntityFolder* apFolder) noexcept;
		void OnEntityFolderMoved(EntityFolder* pMovedFolder, EntityFolder* pMovedFolderParent, const String& aOldPath, const String& aNewPath) noexcept;
		void OnEntitySelectionChanged(entity e, ESelectionState selectionState);
		void OnEntityPreDestroyed(entity e) noexcept;
		void OnEntityAttached(entity child, entity parent) noexcept;
		void OnEntityReadbackDone(uint32 entityID) noexcept;

		void OnViewportHotkeyPressed(ViewportPanel* pPanel, RLS_Key key) noexcept;
		void OnViewportClicked(ViewportPanel* pPanel, Vector2u relativeMouseCoords) noexcept;

		void SpawnViewport() noexcept;
	private:
		std::vector<ViewportRenderView> m_RenderViews;
		std::vector<ViewportPanel*> m_EditorViewports;

		entity m_HoveredEntity = NULL_ENTITY;

		UniquePtr<Selection> m_pSelection = nullptr;
		UniquePtr<EntityFoldersManager> m_pEntityFoldersManager = nullptr;

		std::shared_ptr<Scene> m_pActiveScene = nullptr;
		std::shared_ptr<Scene> m_pEditorScene = nullptr;

		std::shared_ptr<UtilityRenderer> m_pUtilityRenderer = nullptr;
		
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
		
		DetailsPanel* m_pDetailsPanel = nullptr;
		OutlinerPanel* m_pOutlinerPanel = nullptr;
		float m_MinLogLuminance = -4.0f;
		float m_MinEV100 = -10.0f;
		float m_MaxEV100 = 20.0f;
		float m_ExposureCompensation = 1.0f;
	};
}
