#include "EditorLayer.h"
namespace Relentless
{
	EditorLayer::EditorLayer(const std::string& layerName) noexcept
		: Layer{layerName}, m_ViewportPanelSize{100.0f, 100.0f}
	{}

	void EditorLayer::OnEvent(IEvent&) noexcept
	{
		
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

		ImGui::End();
		ImGui::PopStyleVar();
	}

	void EditorLayer::OnAttach() noexcept
	{
		m_pTriangle = std::make_shared<Triangle>();
	}

	void EditorLayer::OnUpdate(const float) noexcept
	{
		if (m_SceneViewportChanged && !Mouse::IsButtonPressed(RLS_MOUSE::Left))
			OnSceneViewportChanged();

		Renderer3D::Begin();
		Renderer3D::Submit(m_pTriangle);
		Renderer3D::End();
	}

	void EditorLayer::OnSceneViewportChanged() noexcept
	{
		if (m_ViewportPanelSize.x < 1u)
			m_ViewportPanelSize.x = 1u;
		if (m_ViewportPanelSize.y < 1u)
			m_ViewportPanelSize.y = 1u;

		Renderer3D::OnSceneViewportChanged(static_cast<uint32_t>(m_ViewportPanelSize.x), static_cast<uint32_t>(m_ViewportPanelSize.y));
		ImguiLayer::OnSceneViewportChanged(static_cast<uint32_t>(m_ViewportPanelSize.x), static_cast<uint32_t>(m_ViewportPanelSize.y));
		m_SceneViewportChanged = false;
	}
}
