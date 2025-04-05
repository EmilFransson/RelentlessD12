#pragma once
#include "Panel.h"

namespace Relentless
{
	class Editor;

	class ViewportPanel : public PanelBase
	{
	public:
		ViewportPanel(const char* pName, ImGuiWindowFlags flags, Editor* pEditor, uint32 renderViewIndex) noexcept;
		virtual ~ViewportPanel() = default;

		[[nodiscard]] const Vector2u& GetViewportSize() const noexcept;
		[[nodiscard]] Vector2i GetClientHoverCoordinates() const noexcept;
		[[nodiscard]] const Vector2u& GetClientScreenPosition() const noexcept;
		[[nodiscard]] bool IsClientAreaHovered() const noexcept;
	protected:
		virtual void PreRender() noexcept override;
		void OnRender() noexcept override;
		virtual void PostRender() noexcept override;

	private:
		Vector2u m_ScreenPosition;
		Vector2u m_ViewportSize;

		Editor* m_pEditor = nullptr;
		uint32 m_RenderViewIndex = std::numeric_limits<uint32>::max();
		bool m_ClientAreaHovered = false;
	};
}