#pragma once
#include "Panel.h"

namespace Relentless
{
	class Editor;

	class ViewportPanel : public PanelBase
	{
	public:
		ViewportPanel(const char* pName, ImGuiWindowFlags flags, Editor* pEditor, uint32 renderViewIndex) noexcept;
	protected:
		virtual void PreRender() noexcept override;
		void OnRender() noexcept override;
		virtual void PostRender() noexcept override;

	private:
		Editor* m_pEditor = nullptr;
		uint32 m_RenderViewIndex = std::numeric_limits<uint32>::max();
	};
}