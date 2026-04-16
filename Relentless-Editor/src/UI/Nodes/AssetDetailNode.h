#pragma once
#include <Relentless.h>

#include "DetailNode.h"

namespace Relentless
{
	class AssetPropertyHandle;
	class AssetThumbnailData;

	class AssetDetailNode : public DetailNode
	{
	public:
		AssetDetailNode(const char* aName, const AssetData& aAssetData) noexcept;
		virtual ~AssetDetailNode() noexcept override;

		NO_DISCARD const SharedPtr<AssetThumbnailData>& GetThumbnailData() const noexcept;
	private:
		AssetData m_AssetData;
		SharedPtr<AssetThumbnailData> m_pThumbnailData;
	};
}