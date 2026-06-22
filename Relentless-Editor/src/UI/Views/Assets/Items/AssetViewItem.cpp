#include "AssetViewItem.h"

namespace Relentless
{
	AssetViewItem::AssetViewItem(EAssetViewItemType aType) noexcept
		: m_Type{aType}
	{
	}

	EAssetViewItemType AssetViewItem::GetType() const noexcept
	{
		return m_Type;
	}
}