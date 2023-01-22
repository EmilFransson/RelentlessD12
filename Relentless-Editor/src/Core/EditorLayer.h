#pragma once
#include <Relentless.h>
#include "../Panels/SceneHierarchyPanel.h"
#include "../Panels/PropertiesPanel.h"
#include "../Panels/ContentBrowserPanel.h"
#include "../Panels/MetricsPanel.h"

namespace Relentless
{
	enum class GizmoType : int8_t { NONE = -1, TRANSLATE = 0, ROTATE = 1, SCALE = 2 };

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
	private:
		void OnSceneViewportChanged() noexcept;
	private:
		ImVec2 m_ViewportPanelSize;
		bool m_SceneViewportChanged;
		bool m_HoveringSceneViewport;
		bool m_ClickedSceneViewPort;
		std::shared_ptr<PerspectiveCamera> m_pEditorCamera;
		ImVec2 vMin;
		ImVec2 vMax;

		ImVec2 sceneViewPortStartPosition;
		ImVec2 sceneViewPortWindowPosition;

		entity m_HoveredEntity;
		entity m_SelectedEntity;
		GizmoType m_CurrentGizmoType;

		SceneHierarchyPanel m_SceneHierarchyPanel;
		PropertiesPanel m_PropertiesPanel;
		ContentBrowserPanel m_ContentBrowserPanel;
		MetricsPanel m_MetricsPanel;

		Scene m_Scene;

		MeshFactory m_MeshFactory;
	};
}
