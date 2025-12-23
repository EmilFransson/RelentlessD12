#include "AssetView.h"

namespace Relentless
{
	AssetView::AssetView() noexcept
	{
		std::shared_ptr<HeaderRow> pHeaderRow = std::make_shared<HeaderRow>();
		pHeaderRow->SetIsVisible(false);

		Column column;
		column.pLabel = new Label("First");
		pHeaderRow->AddColumn(column);

		m_pBox = new HorizontalBoxEx();

		m_pAssetsTreeView = m_pBox->AddWidget(new TileView<Ref<ContentBrowserItem>>(pHeaderRow));
		m_pAssetsTreeView->SetMargin(FloatRect(20.0f, 10.0f, 0.0f, 0.0f));

		m_pAssetsTreeView
			->SetItemWidth(100.0f)
			->SetItemHeight(170.0f)
			->OnRequestSource(this, &AssetView::OnRequestSource)
			->OnGenerateRow(this, &AssetView::OnGenerateRow)
			->OnSelectionChanged(this, &AssetView::OnSelectionChanged);

		{
			for (int i = 0; i < 30; ++i)
			{
				Ref<ContentBrowserItem> pItem = new ContentBrowserItem();
				pItem->Type = AssetType::Mesh;
				m_Items.push_back(pItem);
			}
		}
		
		{
			Ref<ContentBrowserItem> pItem = new ContentBrowserItem();
			pItem->Type = AssetType::Undefined;
			m_Items.push_back(pItem);
		}

		//std::vector<AssetImportTask> tasks;

		//auto&& CreateTextureImportTask = [&tasks, this](const Path& relativePath, bool srgb, auto SetTextureFunc)
		//	{
		//		Ref<TextureFactory> pFactory = RLS_NEW TextureFactory();
		//		pFactory->SetImportAsSRGB(srgb);
		//
		//		pFactory->OnDone.Connect(SetTextureFunc);
		//
		//		AssetImportTask& task = tasks.emplace_back();
		//		task.FilePath = FilepathUtils::Combine(EDITOR_ASSET_DIRECTORY, relativePath);
		//		task.pFactory = pFactory;
		//	};
		//
		//CreateTextureImportTask("Textures/cube_256x256.png", true, [this](const ImportedAsset& asset, bool success)
		//	{
		//		RLS_VERIFY(success, "[AssetView::AssetView] Asset import failed.");
		//		m_MeshIconHandle = asset.Handle;
		//	});
		//
		//CreateTextureImportTask("Textures/folder_256x256.png", true, [this](const ImportedAsset& asset, bool success)
		//	{
		//		RLS_VERIFY(success, "[AssetView::AssetView] Asset import failed.");
		//		m_FolderIconHandle = asset.Handle;
		//	});

		std::vector<AssetImportTask> tasks;
		{
			AssetImportTask& task = tasks.emplace_back();
			task.FilePath = FilepathUtils::Combine(EDITOR_ASSET_DIRECTORY, "Textures/cube_256x256.png");

			Ref<TextureFactory> pFactory = RLS_NEW TextureFactory();
			pFactory->SetImportAsSRGB(true);
			task.pFactory = pFactory;
		}
		{
			AssetImportTask& task = tasks.emplace_back();
			task.FilePath = FilepathUtils::Combine(EDITOR_ASSET_DIRECTORY, "Textures/folder_256x256.png");

			Ref<TextureFactory> pFactory = RLS_NEW TextureFactory();
			pFactory->SetImportAsSRGB(true);
			task.pFactory = pFactory;
		}

		AssetToolsModule& assetToolsModule = ModuleManager::LoadModuleChecked<AssetToolsModule>();
		std::vector<AssetImportResult> importResults = assetToolsModule.Import(tasks);

		Ref<Texture2D> pMeshIconTexture2D = AssetManager::Get<Texture2D>(importResults[0].Handle);
		Ref<Texture2D> pFolderIconTexture2D = AssetManager::Get<Texture2D>(importResults[1].Handle);

		GraphicsDevice* pDevice = Application::Get().GetGraphicsDevice();

		m_pMeshIconTexture = pDevice->CreateTexture(pMeshIconTexture2D->GetDesc(), pMeshIconTexture2D->GetName().c_str(), pMeshIconTexture2D->GetImage());
		m_pFolderIconTexture = pDevice->CreateTexture(pFolderIconTexture2D->GetDesc(), pFolderIconTexture2D->GetName().c_str(), pFolderIconTexture2D->GetImage());

		//Importer::RequestAsyncLoad(tasks).Wait();
	}

	float AssetView::CalcDesiredWidth() const noexcept
	{
		return 0.0f;
	}

	uint64 AssetView::GetFolderIconID() const noexcept
	{
		return m_pFolderIconTexture->GetSRV()->GetGPUHandle().ptr; //AssetManager::Get<Texture>(m_FolderIconHandle)->GetSRV()->GetGPUHandle().ptr;
	}

	uint64 AssetView::GetMeshIconID() const noexcept
	{
		return m_pMeshIconTexture->GetSRV()->GetGPUHandle().ptr; //AssetManager::Get<Texture>(m_MeshIconHandle)->GetSRV()->GetGPUHandle().ptr;
	}

	Ref<ITableRow> AssetView::OnGenerateRow(const Ref<ContentBrowserItem>& aItem) noexcept
	{
		Ref<AssetRow> pRow = new AssetRow();
		pRow->OnMouseEnter(this, &AssetView::OnRowBeginHover);
		pRow->OnMouseExit(this, &AssetView::OnRowEndHover);

		Thumbnail* pThumbnail = pRow->GetThumbnail();
		switch (aItem->Type)
		{
			case AssetType::Mesh:
			{
				pThumbnail->ID(this, &AssetView::GetMeshIconID);
				break;
			}
			default:
			{
				pThumbnail->ID(this, &AssetView::GetFolderIconID);
				break;
			}
		}

		return pRow;
	}

	const std::vector<Ref<ContentBrowserItem>>* AssetView::OnRequestSource() noexcept
	{
		return &m_Items;
	}

	void AssetView::OnRender() noexcept
	{
		//m_pAssetsTreeView->Render();
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

	AssetRow::AssetRow() noexcept
	{
		m_CustomHoverLogic = true;
		m_Tiled = true;

		Ref<Thumbnail> pThumbnail = new Thumbnail(Vector2(100.0f, 170.0f));
		pThumbnail->OnClicked(this, &AssetRow::OnThumbnailClicked);
		pThumbnail->OnMouseEnter(this, &AssetRow::OnThumbnailBeginHover);
		pThumbnail->OnMouseExit(this, &AssetRow::OnThumbnailEndHover);

		m_Thumbnails.push_back(pThumbnail);
	}

	Thumbnail* AssetRow::GetThumbnail() const noexcept
	{
		return m_Thumbnails.front();
	}

	void AssetRow::OnRenderColumn(uint32 aColumn) noexcept
	{
		m_Thumbnails[aColumn]->Render();
	}

	void AssetRow::OnThumbnailBeginHover(Thumbnail* aThumbnail) noexcept
	{
		m_OnMouseEnterCallback.ExecuteIfSet(this);
	}

	void AssetRow::OnThumbnailClicked(Thumbnail* aThumbnail, const PointerInfo& aPointerInfo) noexcept
	{
		m_OnClickedCallback.ExecuteIfSet(aPointerInfo);
	}

	void AssetRow::OnThumbnailEndHover(Thumbnail* aThumbnail) noexcept
	{
		m_OnMouseExitCallback.ExecuteIfSet(this);
	}
}
