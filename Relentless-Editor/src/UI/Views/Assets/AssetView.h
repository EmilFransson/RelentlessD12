#pragma once
#include <Relentless.h>

namespace Relentless
{
	struct ContentBrowserItem : public RefCounted<ContentBrowserItem>
	{
		AssetType Type = AssetType::Undefined;
	};

	class AssetRow : public ITableRow
	{
	public:
		AssetRow() noexcept;
		NO_DISCARD Thumbnail* GetThumbnail() const noexcept;
		NO_DISCARD Vector2 ReportSize() const noexcept override { return m_ColumnWidgets[0]->ReportSize(); }
	protected:
		NO_DISCARD float CalcDesiredWidth() const noexcept override { return 0.0f; }
		const Color& GetBackgroundColor() const noexcept override { return Colors::Transparent; }
		uint32 GetNumColumns() noexcept override { return 1u; }
		bool IsDragDropEligible() noexcept override { return false; }
		void OnRenderColumn(uint32 aColumn) noexcept override;
	private:
		void OnThumbnailBeginHover(Thumbnail* aThumbnail) noexcept;
		void OnThumbnailClicked(Thumbnail* aThumbnail, const PointerInfo& aPointerInfo) noexcept;
		void OnThumbnailEndHover(Thumbnail* aThumbnail) noexcept;
	private:
		std::vector<Ref<Thumbnail>> m_Thumbnails;
	};

	class AssetView : public IWidget<AssetView>
	{
	public:
		AssetView() noexcept;

		NO_DISCARD float CalcDesiredWidth() const noexcept override;

	private:
		NO_DISCARD uint64 GetFolderIconID() const noexcept;
		NO_DISCARD uint64 GetMeshIconID() const noexcept;

		NO_DISCARD Ref<ITableRow> OnGenerateRow(const Ref<ContentBrowserItem>& aItem) noexcept;
		NO_DISCARD const std::vector<Ref<ContentBrowserItem>>* OnRequestSource() noexcept;
		void OnRender() noexcept override;

		void OnRowBeginHover(ITableRow* aRow) noexcept;
		void OnRowEndHover(ITableRow* aRow) noexcept;

		void OnSelectionChanged(const Ref<ContentBrowserItem>& aItem, ESelectionType aSelectionType) noexcept;
	private:
		std::vector<Ref<ContentBrowserItem>> m_Items;
		Ref<TileView<Ref<ContentBrowserItem>>> m_pAssetsTreeView = nullptr;

		Ref<Texture> m_pMeshIconTexture = nullptr;
		Ref<Texture> m_pFolderIconTexture = nullptr;

		//AssetHandle m_MeshIconHandle = NULL_HANDLE;
		//AssetHandle m_FolderIconHandle = NULL_HANDLE;

		Ref<HorizontalBoxEx> m_pBox = nullptr;
	};
}