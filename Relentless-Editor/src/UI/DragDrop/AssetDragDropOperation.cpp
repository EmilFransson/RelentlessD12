#include "AssetDragDropOperation.h"

#include "Module/ContentBrowserModule.h"

#include "UI/Views/Assets/Items/AssetThumbnailData.h"

#include "UI/Widgets/HorizontalBox.h"
#include "UI/Widgets/Label.h"
#include "UI/Widgets/AssetThumbnail.h"

namespace Relentless
{
	AssetDragDropOperation::AssetDragDropOperation(const std::vector<AssetData>& someAssetDatas, const String& aPreviewText) noexcept
		:m_AssetDatas{ someAssetDatas },
		 m_PreviewText{ aPreviewText }
	{
	}

	void AssetDragDropOperation::CreatePreview() noexcept
	{
		ContentBrowserModule& contentBrowserModule = ModuleManager::LoadModuleChecked<ContentBrowserModule>();
		m_pAssetThumbnailData = MakeShared<AssetThumbnailData>(m_AssetDatas.back(), contentBrowserModule.GetAssetThumbnailPool());

		Ref<HorizontalBox> pBox = RLS_NEW HorizontalBox();
		pBox->SetPadding({5.0f, 5.0f, 5.0f, 5.0f});
		pBox->SetSpacing(5.0f);

		pBox->AddWidget(RLS_NEW AssetThumbnail(m_pAssetThumbnailData->GetWeakPtr(), Vector2(50.0f, 50.0f)))
			->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);
		pBox->AddWidget(RLS_NEW Label(m_PreviewText))
			->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);

		m_pPreviewWidget = pBox;
	}

	const std::vector<AssetData>& AssetDragDropOperation::GetAssets() const noexcept
	{
		return m_AssetDatas;
	}

	uint32 AssetDragDropOperation::GetNumDraggedAssets() const noexcept
	{
		return static_cast<uint32>(m_AssetDatas.size());
	}
}