#include "AssetView.h"
#include "Assets/Factory/TextureFactory.h"

#include "Core/Editor.h"
#include "Module/ContentBrowserModule.h"
#include "Subsystem/AssetDefinitionRegistry.h"
#include "Thumbnail/AssetThumbnailPool.h"

#include "UI/Views/TileView.h"
#include "UI/Widgets/Label.h"
#include "UI/Widgets/HorizontalBox.h"
#include "UI/Widgets/Thumbnail.h"

namespace Relentless
{
	AssetView::AssetView() noexcept
	{
		std::shared_ptr<HeaderRow> pHeaderRow = std::make_shared<HeaderRow>();
		pHeaderRow->SetIsVisible(false);

		Column column;
		column.pBox->AddWidget(new Label("First"));
		pHeaderRow->AddColumn(column);

		m_pBox = new HorizontalBox();
		m_pBox->SetPadding(FloatRect(20.0f, 10.0f, 20.0f, 10.0f));

		m_pAssetsTreeView = m_pBox->AddWidget(new TileView<Ref<ContentBrowserItem>>(pHeaderRow));

		m_pAssetsTreeView
			->SetItemWidth(100.0f)
			->SetItemHeight(170.0f)
			->OnRequestSource(this, &AssetView::OnRequestSource)
			->OnGenerateRow(this, &AssetView::OnGenerateRow)
			->OnSelectionChanged(this, &AssetView::OnSelectionChanged)
			->SetHorizontalSizePolicy(ESizePolicy::Stretch)
			->SetVerticalSizePolicy(ESizePolicy::Stretch);

		ContentBrowserModule& contentBrowser = ModuleManager::LoadModuleChecked<ContentBrowserModule>();
		contentBrowser.GetAssetThumbnailPool()->OnThumbnailRegenerated.Connect(this, &AssetView::OnThumbnailRegenerated);
		
		AssetRegistryModule& assetRegistry = ModuleManager::LoadModuleChecked<AssetRegistryModule>();
		if (assetRegistry.IsLoadingAssets())
			InitializeFromAssetRegistry();
		else
		{
			m_AssetRegistryFileScanDoneID = assetRegistry.OnFileScanDone.Connect([this]()
				{
					InitializeFromAssetRegistry();
					ModuleManager::LoadModuleChecked<AssetRegistryModule>().OnFileScanDone.Detach(m_AssetRegistryFileScanDoneID);
				});
		}
	}

	AssetView::~AssetView() noexcept
	{
		AssetRegistryModule& assetRegistry = ModuleManager::LoadModuleChecked<AssetRegistryModule>();
		assetRegistry.OnAssetAdded.Detach(this);

		ContentBrowserModule& contentBrowser = ModuleManager::LoadModuleChecked<ContentBrowserModule>();
		contentBrowser.GetAssetThumbnailPool()->OnThumbnailRegenerated.Detach(this);
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
		if (std::ranges::any_of(m_Items, [&aAssetData](const Ref<ContentBrowserItem>& aItem) {  return aItem->UID == aAssetData.Uuid; }))
			return;

		Ref<ContentBrowserItem> pItem = new ContentBrowserItem();
		pItem->UID = aAssetData.Uuid;

		m_Items.push_back(std::move(pItem));
	}

	Ref<ITableRow> AssetView::OnGenerateRow(const Ref<ContentBrowserItem>& aItem) noexcept
	{
		AssetRegistryModule& assetRegistry = ModuleManager::LoadModuleChecked<AssetRegistryModule>();
		const AssetData* pAsset = assetRegistry.FindAsset(aItem->UID);
		if (!pAsset)
			return nullptr;

		ContentBrowserModule& contentBrowser = ModuleManager::LoadModuleChecked<ContentBrowserModule>();
		const UniquePtr<AssetThumbnailPool>& thumbnailPool = contentBrowser.GetAssetThumbnailPool();

		Ref<Thumbnail> pThumbnail = thumbnailPool->GetAssetThumbnail(*pAsset);
		pThumbnail->SetSize(Vector2(100.0f, 170.0f));

		Ref<AssetRow> pRow = RLS_NEW AssetRow(pThumbnail);
		pRow->OnMouseEnter(this, &AssetView::OnRowBeginHover);
		pRow->OnMouseExit(this, &AssetView::OnRowEndHover);

		return pRow;
	}

	const std::vector<Ref<ContentBrowserItem>>* AssetView::OnRequestSource() noexcept
	{
		return &m_Items;
	}

	void AssetView::OnRender() noexcept
	{
		m_pBox->AssignSize(GetAssignedSize());
		m_pBox->Render();
	}

	void AssetView::OnRowBeginHover(ITableRow* aRow) noexcept
	{
		const bool isSelected = m_pAssetsTreeView->IsItemSelected(m_pAssetsTreeView->GetItemFromWidget(aRow));

		constexpr Color hoverColor = Color(87.0f, 87.0f, 87.0f, 255.0f);
		constexpr Color hoverSelectedColor = Color(14.0f, 134.0f, 255.0f, 255.0f);

		if (isSelected)
			static_cast<AssetRow*>(aRow)->GetThumbnail()->SetBackgroundColor(hoverSelectedColor);
		else 
			static_cast<AssetRow*>(aRow)->GetThumbnail()->SetBackgroundColor(hoverColor);

		AssetRegistryModule& assetRegistry = ModuleManager::LoadModuleChecked<AssetRegistryModule>();
		const AssetData* pAsset = assetRegistry.FindAsset(m_pAssetsTreeView->GetItemFromWidget(aRow)->UID);

		ContentBrowserModule& contentBrowser = ModuleManager::LoadModuleChecked<ContentBrowserModule>();
		const UniquePtr<AssetThumbnailPool>& thumbnailPool = contentBrowser.GetAssetThumbnailPool();
		thumbnailPool->RegenerateAssetThumbnail(*pAsset);
	}

	void AssetView::OnRowEndHover(ITableRow* aRow) noexcept
	{
		const bool isSelected = m_pAssetsTreeView->IsItemSelected(m_pAssetsTreeView->GetItemFromWidget(aRow));

		constexpr Color selectedColor = Color(0.0f, 112.0f, 224.0f, 255.0f);
		
		if (isSelected)
			static_cast<AssetRow*>(aRow)->GetThumbnail()->SetBackgroundColor(selectedColor);
		else
			static_cast<AssetRow*>(aRow)->GetThumbnail()->SetBackgroundColor(Thumbnail::DefaultBackgroundColor);
	}

	void AssetView::OnSelectionChanged(const Ref<ContentBrowserItem>& aItem, ESelectionType aSelectionType) noexcept
	{
		AssetRow* pRow = static_cast<AssetRow*>(m_pAssetsTreeView->GetRowWidget(aItem).Get());
		Thumbnail* pThumbnail = pRow->GetThumbnail();

		if (aSelectionType == ESelectionType::Selected)
		{
			constexpr Color selectedColor = Color(14.0f, 134.0f, 255.0f, 255.0f);
			pThumbnail->SetBackgroundColor(selectedColor);
		}
		else
		{
			if (pThumbnail->IsHovered())
			{
				constexpr Color hoverColor = Color(87.0f, 87.0f, 87.0f, 255.0f);
				pThumbnail->SetBackgroundColor(hoverColor);
			}
			else 
				pThumbnail->SetBackgroundColor(Thumbnail::DefaultBackgroundColor);
		}
	}

	void AssetView::OnThumbnailRegenerated(const AssetData& aAssetData, const Ref<Thumbnail>& aThumbnail) noexcept
	{
		//This will be improved instead of a linear search:
		AssetDefinitionRegistry* pAssetDefinitionRegistry = Editor::Get()->GetSubsystem<AssetDefinitionRegistry>();

		for (size_t i = 0; i < m_Items.size(); ++i)
		{
			if (m_Items[i]->UID == aAssetData.Uuid)
			{
				AssetRow* pAssetRow = static_cast<AssetRow*>(m_pAssetsTreeView->GetRowWidget(m_Items[i]).Get());

				const Ref<IAssetDefinition>& pAssetDefinition = pAssetDefinitionRegistry->GetDefinitionForAsset(aAssetData);

				pAssetRow->GetThumbnail()->SetResource(aThumbnail->GetResource());
				aThumbnail->SetInfo(pAssetDefinition->GetThumbnailInfo(aAssetData));
				break;
			}
		}
	}

	AssetRow::AssetRow(const Ref<Thumbnail>& aThumbnail) noexcept
	{
		m_CustomHoverLogic = true;
		m_Tiled = true;

		aThumbnail->OnClicked(this, &AssetRow::OnThumbnailClicked);
		aThumbnail->OnMouseEnter(this, &AssetRow::OnThumbnailBeginHover);
		aThumbnail->OnMouseExit(this, &AssetRow::OnThumbnailEndHover);

		m_Thumbnails.push_back(aThumbnail);
	}

	Thumbnail* AssetRow::GetThumbnail() const noexcept
	{
		return m_Thumbnails.front();
	}

	void AssetRow::OnRenderColumn(uint32 aColumn) noexcept
	{
		m_Thumbnails[aColumn]->Render();
	}

	void AssetRow::SetThumbnail(const Ref<Thumbnail>& aThumbnail) noexcept
	{
		m_Thumbnails[0] = aThumbnail;
		m_Thumbnails[0]->OnClicked(this, &AssetRow::OnThumbnailClicked);
		m_Thumbnails[0]->OnMouseEnter(this, &AssetRow::OnThumbnailBeginHover);
		m_Thumbnails[0]->OnMouseExit(this, &AssetRow::OnThumbnailEndHover);
	}

	void AssetRow::OnThumbnailBeginHover(MAYBE_UNUSED Thumbnail* aThumbnail) noexcept
	{
		m_OnMouseEnterCallback.ExecuteIfSet(this);
	}

	void AssetRow::OnThumbnailClicked(MAYBE_UNUSED Thumbnail* aThumbnail, const PointerInfo& aPointerInfo) noexcept
	{
		m_OnClickedCallback.ExecuteIfSet(aPointerInfo);
	}

	void AssetRow::OnThumbnailEndHover(MAYBE_UNUSED Thumbnail* aThumbnail) noexcept
	{
		m_OnMouseExitCallback.ExecuteIfSet(this);
	}
}
