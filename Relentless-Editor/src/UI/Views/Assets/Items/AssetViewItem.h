#pragma once
#include <Relentless.h>

namespace Relentless
{
	enum class EAssetViewItemType : uint8 { None = 0u, Asset, Folder };

	class AssetViewItem
	{
	public:
		explicit AssetViewItem(EAssetViewItemType aType) noexcept;
		virtual ~AssetViewItem() noexcept = default;

		NO_DISCARD virtual const String& GetName() const noexcept = 0;
		NO_DISCARD EAssetViewItemType GetType() const noexcept;
	private:
		EAssetViewItemType m_Type = EAssetViewItemType::None;
	};
}