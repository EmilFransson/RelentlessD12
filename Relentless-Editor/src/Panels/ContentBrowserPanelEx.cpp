#include "ContentBrowserPanelEx.h"

#include "../UI/Views/Assets/AssetView.h"

namespace Relentless
{
	ContentBrowserPanelEx::ContentBrowserPanelEx(std::weak_ptr<Editor> aEditor) noexcept
		: IEditorPanel("Content Browser", ImGuiWindowFlags_None, aEditor)
	{
		m_pAssetsView = new AssetView();
		SetRoot(m_pAssetsView);
	}
}
