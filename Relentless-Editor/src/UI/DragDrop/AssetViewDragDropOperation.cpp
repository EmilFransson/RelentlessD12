#include "AssetViewDragDropOperation.h"

#include "UI/Widgets/HorizontalBox.h"
#include "UI/Widgets/Label.h"
#include "UI/Widgets/Thumbnail.h"

namespace Relentless
{
	AssetViewDragDropOperation::AssetViewDragDropOperation(const std::vector<AssetData>& someAssetDatas, const std::vector<String>& somePaths, Ref<Thumbnail> aThumbnail, const String& aPreviewText) noexcept
		: m_AssetDatas{ someAssetDatas }
		, m_Paths{ somePaths }
		, m_PreviewText{ aPreviewText }
		, m_pThumbnail{ aThumbnail }
	{
	}

	void AssetViewDragDropOperation::CreatePreview() noexcept
	{
		Ref<HorizontalBox> pBox = RLS_NEW HorizontalBox();
		pBox->SetPadding({ 5.0f, 5.0f, 5.0f, 5.0f });
		pBox->SetSpacing(5.0f);

		pBox->AddWidget(m_pThumbnail)
			->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);
		pBox->AddWidget(RLS_NEW Label(m_PreviewText))
			->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);

		m_pPreviewWidget = pBox;
	}

	const std::vector<AssetData>& AssetViewDragDropOperation::GetAssets() const noexcept
	{
		return m_AssetDatas;
	}

	uint32 AssetViewDragDropOperation::GetNumDraggedAssets() const noexcept
	{
		return static_cast<uint32>(m_AssetDatas.size());
	}

	const std::vector<String>& AssetViewDragDropOperation::GetPaths() const noexcept
	{
		return m_Paths;
	}

	uint32 AssetViewDragDropOperation::GetNumDraggedPaths() const noexcept
	{
		return static_cast<uint32>(m_Paths.size());
	}

}