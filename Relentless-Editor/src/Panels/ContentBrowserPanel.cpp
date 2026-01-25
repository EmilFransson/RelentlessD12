#include "ContentBrowserPanel.h"

#include <UI/Views/Assets/AssetView.h>

namespace Relentless
{
	ContentBrowserPanel::ContentBrowserPanel() noexcept
		: PanelBase("Content Browser", ImGuiWindowFlags_None)
	{
		m_pAssetsView = new AssetView();
		
		SetRoot(m_pAssetsView);
		SetPadding(Vector2(2.0f, 0.0f));
	}

	ContentBrowserPanel::~ContentBrowserPanel() noexcept = default;
}
