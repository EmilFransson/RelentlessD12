#pragma once 
#include <Relentless.h>
#include "DragDropOperation.h"

namespace Relentless
{
	class AssetThumbnailData;

	class AssetDragDropOperation : public DragDropOperation<AssetDragDropOperation>
	{
	public:
		explicit AssetDragDropOperation(const std::vector<AssetData>& someAssetDatas) noexcept;
		virtual ~AssetDragDropOperation() noexcept override = default;

		virtual void CreatePreview() noexcept override;

		NO_DISCARD const std::vector<AssetData>& GetAssets() const noexcept;
		NO_DISCARD uint32 GetNumDraggedAssets() const noexcept;
	private:
		NO_DISCARD String BuildPreviewText() const noexcept;
	private:
		std::vector<AssetData> m_AssetDatas;
		SharedPtr<AssetThumbnailData> m_pAssetThumbnailData;
	};
}