#include "AssetDragDropOperation.h"

#include "Module/ContentBrowserModule.h"

#include "Thumbnail/AssetThumbnailData.h"

#include "UI/Widgets/HorizontalBox.h"
#include "UI/Widgets/Label.h"
#include "UI/Widgets/AssetThumbnail.h"

namespace Relentless
{
	AssetDragDropOperation::AssetDragDropOperation(const std::vector<AssetData>& someAssetDatas) noexcept
		: m_AssetDatas{ someAssetDatas }
	{
	}

	void AssetDragDropOperation::CreatePreview() noexcept
	{
		ContentBrowserModule& contentBrowserModule = ModuleManager::LoadModuleChecked<ContentBrowserModule>();
		m_pAssetThumbnailData = MakeShared<AssetThumbnailData>(m_AssetDatas.front(), contentBrowserModule.GetAssetThumbnailPool());

		Ref<HorizontalBox> pBox = RLS_NEW HorizontalBox();
		pBox->SetPadding({5.0f, 5.0f, 5.0f, 5.0f});
		pBox->SetSpacing(5.0f);

		pBox->AddWidget(RLS_NEW AssetThumbnail(m_pAssetThumbnailData->GetWeakPtr(), Vector2(50.0f, 50.0f)))
			->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);
		pBox->AddWidget(RLS_NEW Label(BuildPreviewText()))
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

	String AssetDragDropOperation::BuildPreviewText() const noexcept
	{
		const size_t numAssets = m_AssetDatas.size();
		const String& name = m_AssetDatas.front().Name;

		if (numAssets == 1u)
			return name;

		return std::format("{} (+ {} {})", name, numAssets - 1, numAssets > 2 ? "others" : "other");
	}

}