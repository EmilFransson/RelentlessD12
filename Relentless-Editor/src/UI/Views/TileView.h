#pragma once
#include "ListView.h"

namespace Relentless
{
	template<class ItemType>
	class TileView : public ListView<ItemType>
	{
	public:
		TileView(const TableViewStyle& style = TableViewStyle()) noexcept
			: ListView<ItemType>(nullptr, style)
		{
			this->m_Mode = ETableViewMode::Tile;
		}

		NO_DISCARD const ItemType& GetItemFromIndex(uint32 index) const;

		NO_DISCARD virtual bool IsContainer() const noexcept override
		{ 
			return false; 
		};

		void RequestRefresh() noexcept;

		void SetClippingActive(bool aIsActive) noexcept { m_ClippingActive = aIsActive; }
		TileView<ItemType>* SetItemHeight(float aItemHeight) noexcept;
		TileView<ItemType>* SetItemWidth(float aItemWidth) noexcept;

		Broadcaster<void()> OnRefreshed;
	private:
		void OnRender() noexcept override;
		void PopulateLinearizedItems(const std::vector<ItemType>& sourceItems) noexcept;
		void Refresh() noexcept;
	private:
		bool m_ShouldRefresh = true;
		bool m_ClippingActive = true;
		Vector2 m_TileSize = Vector2::Zero;
	};

	template<class ItemType>
	TileView<ItemType>* TileView<ItemType>::SetItemWidth(float aItemWidth) noexcept
	{
		m_TileSize.x = aItemWidth;
		return this;
	}

	template<class ItemType>
	TileView<ItemType>* TileView<ItemType>::SetItemHeight(float aItemHeight) noexcept
	{
		m_TileSize.y = aItemHeight;
		return this;
	}

	template<class ItemType>
	void TileView<ItemType>::Refresh() noexcept
	{
		//Todo: Perhaps move to ITreeViewBase! Also check source vs linearized items.
		this->m_pSource = this->m_OnRequestSource();
		RLS_ASSERT(this->HasValidItemSource(), "[TileView::Refresh]: Items source is invalid.");

		this->m_ItemToRowWidgetMap.clear();

		m_ShouldRefresh = false;

		OnRefreshed();
	}

	template<class ItemType>
	void TileView<ItemType>::RequestRefresh() noexcept
	{
		m_ShouldRefresh = true;
	}

	template<class ItemType>
	void TileView<ItemType>::OnRender() noexcept
	{
		if (m_ShouldRefresh)
			Refresh();

		RLS_ASSERT(this->HasValidItemSource(), "[TileView::OnRender]: Items source is invalid.");

		const std::vector<ItemType>& source = *(this->m_pSource);

		if (source.empty())
			return;

		const ImVec2 availableSpace = this->HasAssignedSize() ? ImVec2(this->GetAssignedSize().x, this->GetAssignedSize().y) : ImGui::GetContentRegionAvail();
		const float availableWidth = availableSpace.x;
		const float spacing = 10.0f;

		float tileWidth = 128.0f;
		float tileHeight = 128.0f;

		if (!Math::AreValuesClose(m_TileSize.x, 0.0f) && !Math::AreValuesClose(m_TileSize.y, 0.0f))
		{
			tileWidth = m_TileSize.x;
			tileHeight = m_TileSize.y;
		}
		else if (!(this->m_ItemToRowWidgetMap.empty()))
		{
			const Vector2 size = this->m_ItemToRowWidgetMap.begin()->second->ReportSize();
			tileWidth = size.x;
			tileHeight = size.y;
		}

		const int numItems = static_cast<int>(source.size());
		const int numItemsPerRow = Math::Max(1, static_cast<int>((availableWidth + spacing) / (tileWidth + spacing)));
		const int numRows = (numItems + numItemsPerRow - 1) / numItemsPerRow;

		ImGuiListClipper clipper;
		clipper.Begin(numRows, tileHeight + spacing);

		bool clipRangeDirty = false;

		struct ItemRectInfo
		{
			ImVec2 Min;
			ImVec2 Max;
		};

		std::vector<ItemRectInfo> itemRectInfos;

		while (clipper.Step())
		{
			for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row)
			{
				const int rowStartIndex = row * numItemsPerRow;
				const int rowEndIndex = std::min(rowStartIndex + numItemsPerRow, numItems);

				bool firstInRow = true;

				for (int i = rowStartIndex; i < rowEndIndex; ++i)
				{
					const ItemType& item = source[i];

					if (!firstInRow)
						ImGui::SameLine(0.0f, spacing);

					firstInRow = false;

					ImGui::BeginGroup();

					if (!this->m_ItemToRowWidgetMap.contains(item))
					{
						this->m_ItemToRowWidgetMap[item] = std::move(this->GenerateNewWidget(item));
						this->m_OnItemScrolledIntoView.ExecuteIfSet(item);
					}

					this->m_ItemToRowWidgetMap[item]->Render();

					ImGui::EndGroup();

					ItemRectInfo& info = itemRectInfos.emplace_back();
					info.Min = ImGui::GetItemRectMin();
					info.Max = ImGui::GetItemRectMax();
				}
			}

			if (static_cast<int>(this->m_VisibleListStart) != clipper.DisplayStart * numItemsPerRow || static_cast<int>(this->m_VisibleListEnd) != Math::Min((clipper.DisplayEnd * numItemsPerRow), numItems))
			{
				this->m_VisibleListStart = clipper.DisplayStart * numItemsPerRow;
				this->m_VisibleListEnd = Math::Min((clipper.DisplayEnd * numItemsPerRow), numItems);
				clipRangeDirty = true;
			}
		}

		if (this->m_ClearSelectionOnEmptySpaceClick && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && std::ranges::none_of(itemRectInfos, [](const ItemRectInfo& aItemRectInfo){ return ImGui::IsMouseHoveringRect(aItemRectInfo.Min, aItemRectInfo.Max); }))
			this->ClearSelection();

		if (clipRangeDirty)
			this->ReleaseInvisibleWidgets();
	}
}