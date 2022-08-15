#include "EditorLayer.h"
namespace Relentless
{
	EditorLayer::EditorLayer(const std::string& layerName) noexcept
		: Layer{layerName}, 
		  m_ViewportPanelSize{100.0f, 100.0f}, 
		  m_SceneViewportChanged{ false },
		  m_HoveringSceneViewport{ false }
	{}

	void EditorLayer::OnEvent(IEvent& event) noexcept
	{
		switch (event.GetEventType())
		{
		case EventType::RawMouseMoveEvent:
		{
			bool isNavigatingScene = m_HoveringSceneViewport && Mouse::IsButtonPressed(RLS_MOUSE::Right);
			if (isNavigatingScene)
				m_pSceneCamera->OnMouseMove();
			event.MarkAsHandled();
			break;
		}
		case EventType::RightMouseButtonPressedEvent:
		{
			//TODO: Mouse should have the functions stated below, not Window.
			bool isNavigatingScene = m_HoveringSceneViewport && Mouse::IsButtonPressed(RLS_MOUSE::Right);
			if (isNavigatingScene)
			{
				Window::ConfineMouseCursor(vMin.x, vMax.x, vMax.y, vMin.y);
				Window::HideMouseCursor();
				event.MarkAsHandled();
			}
			break;
		}
		case EventType::RightMouseButtonReleasedEvent: //TRY TO CHANGE!
		{
			bool isNavigatingScene = m_HoveringSceneViewport && Mouse::IsButtonPressed(RLS_MOUSE::Right);
			if (!isNavigatingScene)
			{
				Window::FreeMouseCursor();
				Window::ShowMouseCursor();
				event.MarkAsHandled();
			}
			break;
		}
		case EventType::KeyDownEvent: //TODO: Change to KeyPressedEvent (since it fires only once until released and pressed again)
		{
			//if (!NAVIGATING_SCENE)
			//{
			//	RLS_KEY key = static_cast<KeyDownEvent&>(event).GetKeyCode();
			//	if (key == RLS_KEY::Q)
			//		m_GizmoType = -1;
			//	else if (key == RLS_KEY::W)
			//		m_GizmoType = ImGuizmo::TRANSLATE;
			//	else if (key == RLS_KEY::E)
			//		m_GizmoType = ImGuizmo::ROTATE;
			//	else if (key == RLS_KEY::R)
			//		m_GizmoType = ImGuizmo::SCALE;
			//}
			//
			//event.MarkAsHandled();
			break;
		}
		}
	}

	void EditorLayer::OnImGuiRender() noexcept
	{
		auto pUITextureHandle = ImguiLayer::GetUITextureGPUHandle();
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Viewport", NULL);
		
		if (m_ViewportPanelSize.x != ImGui::GetContentRegionAvail().x || m_ViewportPanelSize.y != ImGui::GetContentRegionAvail().y)
		{
			m_ViewportPanelSize.x = ImGui::GetContentRegionAvail().x;
			m_ViewportPanelSize.y = ImGui::GetContentRegionAvail().y;
			m_SceneViewportChanged = true;
		}
		
		ImGui::Image
		(
			(ImTextureID)pUITextureHandle.ptr,
			ImVec2(m_ViewportPanelSize.x, m_ViewportPanelSize.y)
		);

		if (ImGui::IsWindowHovered())
		{
			m_HoveringSceneViewport = true;
			vMin = ImGui::GetWindowContentRegionMin();
			vMax = ImGui::GetWindowContentRegionMax();

			vMin.x += ImGui::GetWindowPos().x;
			vMin.y += ImGui::GetWindowPos().y;
			vMax.x += ImGui::GetWindowPos().x;
			vMax.y += ImGui::GetWindowPos().y;
			ImGui::GetForegroundDrawList()->AddRect(vMin, vMax, IM_COL32(255, 255, 0, 255), 0.0f, 0, 0.1f);
		}
		else
		{
			m_HoveringSceneViewport = false;
		}
		ImGui::End();
		ImGui::PopStyleVar();
	}

	void EditorLayer::OnAttach() noexcept
	{
		m_pSceneCamera = std::move(PerspectiveCamera::Create(DirectX::XMVECTORF32{ 0.0f, 0.0f, -10.0f }, 1360u, 750));
		m_pTriangle = std::make_shared<Triangle>();
	}

	void EditorLayer::OnUpdate(const float deltaTime) noexcept
	{
		if (m_SceneViewportChanged && !Mouse::IsButtonPressed(RLS_MOUSE::Left))
			OnSceneViewportChanged();

		if (m_HoveringSceneViewport)
			m_pSceneCamera->Update(deltaTime);

		Renderer3D::Begin(m_pSceneCamera);
		Renderer3D::Submit(m_pTriangle);
		Renderer3D::End();
	}

	void EditorLayer::OnSceneViewportChanged() noexcept
	{
		if (m_ViewportPanelSize.x < 1u)
			m_ViewportPanelSize.x = 1u;
		if (m_ViewportPanelSize.y < 1u)
			m_ViewportPanelSize.y = 1u;

		m_pSceneCamera->RecalculateProjectionMatrix(static_cast<uint32_t>(m_ViewportPanelSize.x), static_cast<uint32_t>(m_ViewportPanelSize.y));
		Renderer3D::OnSceneViewportChanged(static_cast<uint32_t>(m_ViewportPanelSize.x), static_cast<uint32_t>(m_ViewportPanelSize.y));
		ImguiLayer::OnSceneViewportChanged(static_cast<uint32_t>(m_ViewportPanelSize.x), static_cast<uint32_t>(m_ViewportPanelSize.y));
		m_SceneViewportChanged = false;
	}
}
