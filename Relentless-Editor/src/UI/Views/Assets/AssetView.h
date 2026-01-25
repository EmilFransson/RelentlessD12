#pragma once
#include "UI/Widgets/ITableRow.h"

namespace Relentless
{
	enum class ESelectionType : uint8;
	class HorizontalBox;
	class Thumbnail;
	template<typename T>
	class TileView;

	struct ContentBrowserItem : public RefCounted<ContentBrowserItem>
	{
		UUID UID = NULL_UUID;
	};

	class AssetRow : public ITableRow
	{
	public:
		AssetRow(const Ref<Thumbnail>& aThumbnail) noexcept;
		NO_DISCARD Thumbnail* GetThumbnail() const noexcept;
		NO_DISCARD Vector2 ReportSize() const noexcept override { return m_ColumnWidgets[0]->ReportSize(); }
		void SetThumbnail(const Ref<Thumbnail>& aThumbnail) noexcept;
	protected:
		NO_DISCARD float CalcDesiredWidth() const noexcept override { return 0.0f; }
		NO_DISCARD const Color& GetBackgroundColor() const noexcept override { return Colors::Transparent; }
		NO_DISCARD uint32 GetNumColumns() noexcept override { return 1u; }
		NO_DISCARD bool IsDragDropEligible() noexcept override { return false; }
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
		virtual ~AssetView() noexcept;

		NO_DISCARD float CalcDesiredWidth() const noexcept override;

	private:
		void InitializeFromAssetRegistry() noexcept;

		void OnAssetAdded(const AssetData& aAssetData) noexcept;
		NO_DISCARD Ref<ITableRow> OnGenerateRow(const Ref<ContentBrowserItem>& aItem) noexcept;
		NO_DISCARD const std::vector<Ref<ContentBrowserItem>>* OnRequestSource() noexcept;
		void OnRender() noexcept override;
		void OnRowBeginHover(ITableRow* aRow) noexcept;
		void OnRowEndHover(ITableRow* aRow) noexcept;
		void OnSelectionChanged(const Ref<ContentBrowserItem>& aItem, ESelectionType aSelectionType) noexcept;
		void OnThumbnailRegenerated(const AssetData& aAssetData, const Ref<Thumbnail>& aThumbnail) noexcept;
	private:
		std::vector<Ref<ContentBrowserItem>> m_Items;
		Ref<TileView<Ref<ContentBrowserItem>>> m_pAssetsTreeView = nullptr;

		Ref<HorizontalBox> m_pBox = nullptr;
		CallbackID m_AssetRegistryFileScanDoneID = -1;
	};
}