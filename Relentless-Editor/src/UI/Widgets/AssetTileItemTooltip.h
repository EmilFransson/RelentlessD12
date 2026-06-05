#pragma once
#include "Tooltip.h"

namespace Relentless
{
	class VerticalBox;

	class AssetTileItemTooltip : public Tooltip
	{
	public:
		AssetTileItemTooltip(const AssetData& aAssetData) noexcept;

	protected:
		void OnRender() noexcept override;
	private:
		Ref<VerticalBox> m_pSlot;
	};
}