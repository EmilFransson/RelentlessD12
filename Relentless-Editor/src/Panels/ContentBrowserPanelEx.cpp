#include "ContentBrowserPanelEx.h"

#include "../UI/Views/Assets/AssetView.h"

namespace Relentless
{
	ContentBrowserPanelEx::ContentBrowserPanelEx() noexcept
		: PanelBase("Content Browser", ImGuiWindowFlags_None)
	{
		m_pAssetsView = new AssetView();
		SetRoot(m_pAssetsView);
	}
}
