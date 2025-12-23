#pragma once

#include <Relentless.h>
#include "IEditorPanel.h"

namespace Relentless
{
	class AssetView;

	class ContentBrowserPanelEx : public IEditorPanel
	{
	public:
		ContentBrowserPanelEx(std::weak_ptr<Editor> aEditor) noexcept;
	private:
		Ref<AssetView> m_pAssetsView = nullptr;
	};
}