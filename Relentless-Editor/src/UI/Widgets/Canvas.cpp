#include "Canvas.h"

namespace Relentless
{
	const IntRect& Canvas::GetScreenRect() const noexcept
	{
		return m_ScreenRect;
	}

	Vector2 Canvas::ReportSize() const noexcept
	{
		return { (float)m_Size.x, (float)m_Size.y };
	}

	bool Canvas::RequiresAssignedSize() const noexcept
	{
		return true;
	}

	void Canvas::OnRender() noexcept
	{
		PROFILE_FUNC;

		Vector2 sizeFloat = GetAssignedSize();
		Vector2i size = Vector2i(sizeFloat.x, sizeFloat.y); //GetAssignedSize(); //Vector2i(static_cast<int32>(ImGui::GetContentRegionAvail().x), static_cast<int32>(ImGui::GetContentRegionAvail().y));

		if (size.x > 0 && size.y > 0)
		{
			Texture* pTarget = m_TargetCallback();
			ImGui::Image((ImTextureID)pTarget->GetSRV()->GetGPUHandle().ptr, ImVec2(static_cast<float>(size.x), static_cast<float>(size.y)));
		}
		
		const ImVec2 rectMin = ImGui::GetItemRectMin();
		const ImVec2 rectMax = ImGui::GetItemRectMax();
		
		m_ScreenRect = IntRect((int)rectMin.x, (int)rectMin.y, (int)rectMax.x, (int)rectMax.y);
		size.x = m_ScreenRect.GetWidth();
		size.y = m_ScreenRect.GetHeight();

		if (m_Size != size)
			Resize(size);

		const bool isHovering = ImGui::IsMouseHoveringRect(rectMin, rectMax);

		if ((isHovering && !m_IsHovered) || (!isHovering && m_IsHovered))
		{
			m_IsHovered = isHovering;

			if (m_OnHoverStateChanged.IsSet())
				m_OnHoverStateChanged(m_IsHovered);
		}
	}

	void Canvas::Resize(const Vector2i& newSize) noexcept
	{
		m_Size = Vector2i((int32)newSize.x, (int32)newSize.y);

		if (m_OnResizeCallback.IsSet())
			m_OnResizeCallback(m_Size);
	}
}