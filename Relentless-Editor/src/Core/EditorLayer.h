#pragma once
#include <Relentless.h>
#include "../Panels/SceneHierarchyPanel.h"
#include "../Panels/PropertiesPanel.h"
#include "../Panels/ContentBrowserPanel.h"
#include "../Panels/MetricsPanel.h" 
#include "../Panels/SceneRendererPanel.h"
#include "../Panels/InspectorPanel.h"

namespace Relentless
{
	enum class GizmoType : int8_t { NONE = -1, TRANSLATE = 0, ROTATE = 1, SCALE = 2 };
	enum class GizmoMode : uint8_t { LOCAL = 0, WORLD };

	class EditorLayer : public Layer
	{
	public:
		EditorLayer(const std::string& layerName = "EditorLayer") noexcept;
		virtual ~EditorLayer() noexcept override final = default;
		virtual void OnEvent(IEvent& event) noexcept override final;
		virtual void OnImGuiRender() noexcept override final;
		virtual void OnAttach() noexcept override final;
		virtual void OnDetach() noexcept override final{};
		virtual void OnUpdate(const float deltaTime) noexcept override final;
		virtual void OnRender() noexcept override final;
		virtual void OnPostRender() noexcept override final;
	private:
		void LoadStarterMeshes() noexcept;
		void CreateStartScene() noexcept;
		void OnSceneViewportChanged() noexcept;
		void DestroySelectedEntity() noexcept;
		void CopySelectedEntity() noexcept;
		void ManipulateTransformGizmo() noexcept;
		void LoadScene(const std::filesystem::path& filepath) noexcept;
		void SaveScene(const std::filesystem::path& filepath) noexcept;
	private:
		bool m_SceneViewportChanged;
		bool m_HoveringSceneViewport;
		ImVec2 m_ViewportPanelSize;
		ImVec2 vMin;
		ImVec2 vMax;

		entity m_HoveredEntity;
		entity m_SelectedEntity;
		GizmoType m_CurrentGizmoType;
		GizmoMode m_CurrentGizmoMode;

		SceneHierarchyPanel m_SceneHierarchyPanel;
		PropertiesPanel m_PropertiesPanel;
		ContentBrowserPanel m_ContentBrowserPanel;
		MetricsPanel m_MetricsPanel;
		SceneRendererPanel m_SceneRendererPanel;
		InspectorPanel m_InspectorPanel;

		std::shared_ptr<Scene> m_pScene;
		std::shared_ptr<SceneRenderer> m_pSceneRenderer;
	
		std::queue<std::function<void()>> m_DeferredFunctions;
		
		bool m_DisplaySceneHierarchyPanel;
		bool m_DisplayContentBrowserPanel;
		bool m_DisplayPropertiesPanel;
		bool m_DisplayInspectorPanel;
		bool m_DisplayMetricsPanel;
		bool m_DisplaySceneRendererPanel;
		bool m_DisplayStatisticsPanel;

		bool m_ImmersiveModeEnabled;

		std::string m_Path{};
		bool m_CreateNewScene{ false };
	};
}
