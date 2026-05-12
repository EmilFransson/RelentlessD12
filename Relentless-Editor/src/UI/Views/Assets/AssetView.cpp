#include "AssetView.h"
#include "Assets/Factory/TextureFactory.h"

#include "Core/Editor.h"

#include "Module/ContentBrowserModule.h"

#include "Subsystem/AssetDefinitionRegistry.h"

#include "Thumbnail/AssetThumbnailPool.h"

#include "UI/DragDrop/AssetDragDropOperation.h"
#include "UI/Views/TileView.h"
#include "UI/Widgets/AssetTileItem.h"
#include "UI/Widgets/ITableRow.h"
#include "UI/Widgets/Label.h"
#include "UI/Widgets/HorizontalBox.h"

namespace Relentless
{
	AssetView::AssetView() noexcept
	{
		m_pBox = RLS_NEW HorizontalBox();
		m_pBox->SetPadding(FloatRect(20.0f, 10.0f, 20.0f, 10.0f));

		m_pAssetsTreeView = m_pBox->AddWidget(RLS_NEW TileView<SharedPtr<AssetThumbnailData>>());

		m_pAssetsTreeView
			->SetItemWidth(100.0f)
			->SetItemHeight(170.0f)
			->OnRequestSource(this, &AssetView::OnRequestSource)
			->OnGenerateRow(this, &AssetView::OnGenerateItem)
			->OnDoubleClick(this, &AssetView::OnAssetTileItemDoubleClicked)
			->SetHorizontalSizePolicy(ESizePolicy::Stretch)
			->SetVerticalSizePolicy(ESizePolicy::Stretch);

		AssetRegistryModule& assetRegistry = ModuleManager::LoadModuleChecked<AssetRegistryModule>();
		if (!assetRegistry.IsLoadingAssets())
			InitializeFromAssetRegistry();
		else
		{
			m_AssetRegistryFileScanDoneID = assetRegistry.OnFileScanDone.Connect([this]()
				{
					InitializeFromAssetRegistry();
					ModuleManager::LoadModuleChecked<AssetRegistryModule>().OnFileScanDone.Detach(m_AssetRegistryFileScanDoneID);
					m_AssetRegistryFileScanDoneID = -1;
				});
		}
	}

	AssetView::~AssetView() noexcept
	{
		AssetRegistryModule& assetRegistry = ModuleManager::LoadModuleChecked<AssetRegistryModule>();
		assetRegistry.OnAssetAdded.Detach(this);
	}

	void AssetView::InitializeFromAssetRegistry() noexcept
	{
		AssetRegistryModule& assetRegistry = ModuleManager::LoadModuleChecked<AssetRegistryModule>();

		assetRegistry.ForEachAssetWithPath("Assets/", [this](const AssetData& aData)
			{
				OnAssetAdded(aData);
				return true;
			}, 
			true);

		assetRegistry.OnAssetAdded.Connect(this, &AssetView::OnAssetAdded);
	}

	void AssetView::OnAssetAdded(const AssetData& aAssetData) noexcept
	{
		ContentBrowserModule& contentBrowser = ModuleManager::LoadModuleChecked<ContentBrowserModule>();
		const SharedPtr<AssetThumbnailPool>& pThumbnailPool = contentBrowser.GetAssetThumbnailPool();

		m_Items.push_back(MakeShared<AssetThumbnailData>(aAssetData, pThumbnailPool));
		m_pAssetsTreeView->RequestRefresh();
	}

	Reply AssetView::OnAssetTileItemDragDetected(MAYBE_UNUSED const WidgetGeometry& aGeometry, MAYBE_UNUSED const PointerInfo& aPointerInfo) noexcept
	{
		std::vector<SharedPtr<AssetThumbnailData>> selectedItems;
		if (m_pAssetsTreeView->GetSelectedItems(selectedItems) == 0u)
			return Reply::Unhandled();

		std::vector<AssetData> assetDatas;
		assetDatas.reserve(selectedItems.size());

		for (const auto& thumbnailData : selectedItems)
			assetDatas.push_back(thumbnailData->GetAssetData());

		return Reply::Handled().BeginDragDrop(RLS_NEW AssetDragDropOperation(assetDatas));
	}

	void AssetView::OnAssetTileItemDoubleClicked(const SharedPtr<AssetThumbnailData>& aThumbnailData) noexcept
	{
		if (const Ref<IAssetDefinition>& pAssetDefinition = Editor::Get()->GetSubsystem<AssetDefinitionRegistry>()->GetDefinitionForAsset(aThumbnailData->GetAssetData()))
		{
			const AssetHandle assetHandle = AssetManager::LoadAsset(aThumbnailData->GetAssetData());
			if (!assetHandle.IsValid())
				return;

			pAssetDefinition->OpenAssets({ assetHandle });
		}
	}

	Ref<ITableRow> AssetView::OnGenerateItem(const SharedPtr<AssetThumbnailData>& aItem) noexcept
	{
		Ref<AssetTileItem> pAssetTileItem = RLS_NEW AssetTileItem(*aItem, Vector2(100.0f, 170.0f), m_pAssetsTreeView);
		pAssetTileItem->OnDragDetected(this, &AssetView::OnAssetTileItemDragDetected);

		return pAssetTileItem;
	}

	const std::vector<SharedPtr<AssetThumbnailData>>* AssetView::OnRequestSource() noexcept
	{
		return &m_Items;
	}

	void AssetView::OnRender() noexcept
	{
		m_pBox->AssignSize(GetAssignedSize());
		m_pBox->Render();
	}
}
