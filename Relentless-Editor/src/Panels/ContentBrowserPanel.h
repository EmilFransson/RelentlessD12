#pragma once
#include <UI/Widgets/Panel.h>

namespace Relentless
{
	class AssetView;

	class ContentBrowserPanel : public PanelBase
	{
	public:
		ContentBrowserPanel() noexcept;
		virtual ~ContentBrowserPanel() noexcept;
	private:
		Ref<AssetView> m_pAssetsView = nullptr;
	};
}