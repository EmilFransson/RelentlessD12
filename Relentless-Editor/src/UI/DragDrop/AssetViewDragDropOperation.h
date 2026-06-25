#pragma once 
#include <Relentless.h>
#include "DragDropOperation.h"

namespace Relentless
{
	class Thumbnail;

	class AssetViewDragDropOperation : public DragDropOperation<AssetViewDragDropOperation>
	{
	public:
		AssetViewDragDropOperation(const std::vector<AssetData>& someAssetDatas, const std::vector<String>& somePaths, Ref<Thumbnail> aThumbnail, const String& aPreviewText) noexcept;
		virtual ~AssetViewDragDropOperation() noexcept override = default;

		void CreatePreview() noexcept override;

		NO_DISCARD const std::vector<AssetData>& GetAssets() const noexcept;
		NO_DISCARD uint32 GetNumDraggedAssets() const noexcept;
		NO_DISCARD const std::vector<String>& GetPaths() const noexcept;
		NO_DISCARD uint32 GetNumDraggedPaths() const noexcept;
	private:
		std::vector<AssetData> m_AssetDatas;
		std::vector<String> m_Paths;

		String m_PreviewText;
		Ref<Thumbnail> m_pThumbnail;
	};
}