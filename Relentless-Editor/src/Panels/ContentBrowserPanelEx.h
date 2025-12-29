#pragma once
#include <Relentless.h>

namespace Relentless
{
	class AssetView;

	class ContentBrowserPanelEx : public PanelBase
	{
	public:
		ContentBrowserPanelEx() noexcept;
	private:
		Ref<AssetView> m_pAssetsView = nullptr;
	};
}