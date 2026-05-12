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
		
		NO_DISCARD virtual String GetDisplayName() const noexcept override;
		NO_DISCARD virtual String GetPersistKey() const noexcept override;
	private:
		Ref<AssetView> m_pAssetsView = nullptr;
	};
}