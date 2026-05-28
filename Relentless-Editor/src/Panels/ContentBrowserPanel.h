#pragma once
#include "Panel.h"

namespace Relentless
{
	class AssetView;
	class Button;
	class HorizontalBox;

	class ContentBrowserPanel : public PanelBase
	{
	public:
		ContentBrowserPanel() noexcept;
		virtual ~ContentBrowserPanel() noexcept;
		
		NO_DISCARD virtual String GetDisplayName() const noexcept override;
		NO_DISCARD virtual String GetPersistKey() const noexcept override;
	private:
		NO_DISCARD Ref<Button> BuildAddAssetButton() noexcept;
		NO_DISCARD Ref<HorizontalBox> BuildToolbar() noexcept;

		void OnAddAssetButtonClicked() noexcept;
	private:
		Ref<AssetView> m_pAssetsView = nullptr;
	};
}