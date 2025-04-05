#include "ViewportPanel.h"
#include "../Core/Editor.h"

namespace Relentless
{
	ViewportPanel::ViewportPanel(const char* pName, ImGuiWindowFlags flags, Editor* pEditor, uint32 renderViewIndex) noexcept
		: PanelBase(pName, flags), m_pEditor{pEditor}, m_RenderViewIndex{renderViewIndex}
	{}

	const Vector2u& ViewportPanel::GetViewportSize() const noexcept
	{
		return m_ViewportSize;
	}

	Vector2i ViewportPanel::GetClientHoverCoordinates() const noexcept
	{
		if (!IsClientAreaHovered())
			return Vector2i(-1, -1);

		const Vector2u& windowPos = GetClientScreenPosition();
		const Vector2u mouseScreenPosition = Mouse::GetCursorScreenPosition();

		// Compute relative coordinates by subtracting window top-left position
		const Vector2i clientPosition = Vector2i(mouseScreenPosition.x - windowPos.x, mouseScreenPosition.y - windowPos.y);
		if (clientPosition.x < 0 || clientPosition.y < 0)
			return Vector2i(-1, -1);

		return clientPosition;
	}

	const Vector2u& ViewportPanel::GetClientScreenPosition() const noexcept
	{
		return m_ScreenPosition;
	}

	bool ViewportPanel::IsClientAreaHovered() const noexcept
	{
		return m_ClientAreaHovered;
	}

	void ViewportPanel::PreRender() noexcept
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	}

	void ViewportPanel::OnRender() noexcept
	{
		const Vector2u& region = GetContentRegionAvail();
		if (region.x == 0u || region.y == 0u)
			return;

		ViewportRenderView& renderView = m_pEditor->GetRenderView(m_RenderViewIndex);

		// Toolbar section with a fixed height (e.g., 30 pixels)
		ImGui::BeginChild("Toolbar", ImVec2(0, 30), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);

		// Place toolbar buttons horizontally using SameLine()
		if (ImGui::Button("Wireframe"))
		{
			// Switch to wireframe mode
			renderView.RenderMode = renderView.RenderMode == RenderModeEx::Solid ? RenderModeEx::Wireframe : RenderModeEx::Solid;
		}
		ImGui::SameLine();
		if (ImGui::Button("Solid"))
		{
			renderView.DrawGrid = !renderView.DrawGrid;
			// Switch to solid mode
		}
		// Add more controls as needed...

		ImGui::EndChild();

		// Main viewport section takes up the remaining space
		ImGui::BeginChild("Viewport", ImVec2(0, 0), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);

		// Render your viewport content here
		// e.g., displaying the rendered image, handling input, etc.
		
		m_ViewportSize = Vector2u(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y);
		ImGui::Image((ImTextureID)renderView.pTarget->GetSRV()->GetGPUHandle().ptr, ImVec2(m_ViewportSize.x, m_ViewportSize.y));
		
		m_ClientAreaHovered = ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenOverlapped);

		// Retrieve the client area (in absolute screen coordinates)
		m_ScreenPosition = Vector2u(ImGui::GetItemRectMin().x, ImGui::GetItemRectMin().y);

		ImGui::EndChild();
	}

	void ViewportPanel::PostRender() noexcept
	{
		ImGui::PopStyleVar();
	}
}