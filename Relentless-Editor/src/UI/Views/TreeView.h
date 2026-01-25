#pragma once
#include "ListView.h"

namespace Relentless
{
	#define INDEX_NONE -1

	struct ItemInfo
	{
		bool HasChildren = false;
		bool IsExpanded = true;
		int32 ParentIndex = INDEX_NONE;
		uint32 Depth = 0u;
	};

	template<class ItemType>
	class TreeView : public ListView<ItemType>
	{
	public:
		TreeView(std::shared_ptr<HeaderRow> pHeaderRow, const TableViewStyle& style = TableViewStyle()) noexcept
			: ListView<ItemType>(pHeaderRow, style)
		{
			this->m_Mode = ETableViewMode::Tree;
		}

		NO_DISCARD std::vector<ItemType> GetDescendants(const ItemType& pAncestor) noexcept;

		NO_DISCARD bool ExistsItemInfo(const ItemType& pItem) const;
		NO_DISCARD const ItemInfo& GetItemInfo(const ItemType& pItem) const;
		NO_DISCARD const ItemType& GetItemFromIndex(uint32 index) const;

		template<typename InstanceType>
		TreeView<ItemType>* OnExpansionChanged(InstanceType* instance, void(InstanceType::*method)(const ItemType&)) noexcept;

		template<typename InstanceType>
		TreeView<ItemType>* OnGetChildren(InstanceType* instance, void(InstanceType::*method)(const ItemType&, std::vector<ItemType>& outChildren)) noexcept;

		void RequestTreeRefresh() noexcept;

		void RequestScrollToRow(uint32 row, float centerRatio = 0.35f) noexcept { m_PendingScroll = { row, centerRatio }; }
		void RequestScrollToItem(const ItemType& item, float centerRatio = 0.35f) noexcept
		{
			if (auto it = std::find(m_LinearizedItems.begin(), m_LinearizedItems.end(), item);
				it != m_LinearizedItems.end())
			{
				RequestScrollToRow(static_cast<uint32>(std::distance(m_LinearizedItems.begin(), it)), centerRatio);
			}
			else
			{
				m_ShouldRefresh = true;
				m_PendingScrollItem = item;
				// The actual row will be resolved next frame after RefreshTree().
			}
		}

		void SetClippingActive(bool aIsActive) noexcept { m_ClippingActive = aIsActive; }
		void SetItemExpandedState(const ItemType& pItem, bool expandedState) noexcept;

		Broadcaster<void()> OnTreeRefreshed;
	private:
		void OnRender() noexcept override;
		void PopulateLinearizedItems(const std::vector<ItemType>& sourceItems) noexcept;
		void RefreshTree() noexcept;
	private:
		std::vector<ItemType> m_LinearizedItems;
		std::vector<ItemType> m_Scratch;
		std::unordered_map<ItemType, ItemInfo> m_ItemInfos;

		struct PendingScrollTarget { uint32 Row = UINT32_MAX; float CenterRatio = 0.35f; bool IsValid() const { return Row != UINT32_MAX; } };
		PendingScrollTarget m_PendingScroll;                 // [scroll] request by row
		std::optional<ItemType> m_PendingScrollItem;         // [scroll] request by item (resolved after refresh)

		Callback<void(const ItemType&)> m_OnExpansionChanged;
		Callback<void(const ItemType&, std::vector<ItemType>&)> m_OnGetChildren;

		bool m_ShouldRefresh = true;
		bool m_ClippingActive = true;
	};

	template<class ItemType>
	bool TreeView<ItemType>::ExistsItemInfo(const ItemType& pItem) const
	{
		return m_ItemInfos.contains(pItem);
	}

	template<class ItemType>
	std::vector<ItemType> TreeView<ItemType>::GetDescendants(const ItemType& pAncestor) noexcept
	{
		std::vector<ItemType> result;

		auto&& recurse = [&](auto&& self, const ItemType& node) -> void
			{
				m_Scratch.clear();
				m_OnGetChildren(node, m_Scratch);

				std::vector<ItemType> children;
				children.swap(m_Scratch);

				result.insert(result.end(), children.begin(), children.end());
				for (const auto& child : children)
					self(self, child);
			};

		recurse(recurse, pAncestor);
		return result;
	}

	template<class ItemType>
	void TreeView<ItemType>::SetItemExpandedState(const ItemType& pItem, bool expandedState) noexcept
	{
		m_ItemInfos.at(pItem).IsExpanded = expandedState;
		m_ShouldRefresh = true;
	}

	template<class ItemType>
	const ItemType& TreeView<ItemType>::GetItemFromIndex(uint32 index) const
	{
		return m_LinearizedItems[index];
	}

	template<class ItemType>
	void TreeView<ItemType>::PopulateLinearizedItems(const std::vector<ItemType>& sourceItems) noexcept
	{
		m_LinearizedItems.clear();

		auto&& RecursivelyAddChildren = [this](auto&& self, const std::vector<ItemType>& items, int32 parentIndex, uint32 depth) -> void
			{
				for (const ItemType& item : items)
				{
					m_Scratch.clear();

					m_OnGetChildren(item, m_Scratch);
					const bool hasChildren = !m_Scratch.empty();

					ItemInfo& info = m_ItemInfos[item];
					info.HasChildren = hasChildren;
					info.ParentIndex = parentIndex;
					info.Depth = depth;

					const int32 myIndex = static_cast<int32>(m_LinearizedItems.size());
					m_LinearizedItems.push_back(item);

					if (hasChildren && info.IsExpanded)
					{
						std::vector<ItemType> localChildren;
						localChildren.swap(m_Scratch);

						self(self, localChildren, myIndex, depth + 1);

						m_Scratch.swap(localChildren);
					}
				}
			};

		RecursivelyAddChildren(RecursivelyAddChildren, sourceItems, INDEX_NONE, 0u);
	}

	template<class ItemType>
	void TreeView<ItemType>::RefreshTree() noexcept
	{
		//Todo: Perhaps move to ITreeViewBase! Also check source vs linearized items.
		this->m_pSource = this->m_OnRequestSource();
		RLS_ASSERT(this->HasValidItemSource(), "[TreeView::RefreshTree]: Items source is invalid.");

		this->m_ItemToRowWidgetMap.clear();

		PopulateLinearizedItems(*(this->m_pSource));
		this->m_pSource = &m_LinearizedItems;
		
		m_ShouldRefresh = false;

		OnTreeRefreshed();
	}

	template<class ItemType>
	void TreeView<ItemType>::RequestTreeRefresh() noexcept
	{
		m_ShouldRefresh = true;
	}

	template<class ItemType>
	const ItemInfo& TreeView<ItemType>::GetItemInfo(const ItemType& pItem) const
	{
		RLS_ASSERT(m_ItemInfos.contains(pItem), "[TreeView::GetItemInfo]: Item has no info assigned!");
		return m_ItemInfos.at(pItem);
	}

	template<class ItemType>
	void TreeView<ItemType>::OnRender() noexcept
	{
		if (m_ShouldRefresh)
			RefreshTree();

		if (m_LinearizedItems.empty())
			return;

		const uint32 numColumns = this->m_pHeaderRow->GetNumColumns();

		if (this->m_Style.UseAlternatingRowColors)
		{
			ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_TableRowBg, ImVec4(this->m_Style.EvenRowColor.R(), this->m_Style.EvenRowColor.G(), this->m_Style.EvenRowColor.B(), this->m_Style.EvenRowColor.A()));
			ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_TableRowBgAlt, ImVec4(this->m_Style.OddRowColor.R(), this->m_Style.OddRowColor.G(), this->m_Style.OddRowColor.B(), this->m_Style.OddRowColor.A()));
		}

		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(ImGui::GetStyle().CellPadding.x, 0.0f));

		if (ImGui::BeginTable("##Table", numColumns, this->GetFlags()))
		{
			this->m_pHeaderRow->OnRender();

			ImGuiListClipper clipper;
			clipper.Begin(m_LinearizedItems.size());

			bool warmUp = true;
			bool clipRangeDirty = false;
			float lastRowY = 0.0f;

			float clipStartPosY = FLT_MAX;
			float clipItemsHeight = 0.0f;

			if (this->m_ClippingActive)
			{
				while (clipper.Step())
				{
					if (clipStartPosY == FLT_MAX)
						clipStartPosY = clipper.StartPosY;
					if (!warmUp)
						clipItemsHeight = clipper.ItemsHeight;

					for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row)
					{
						const ItemType& item = m_LinearizedItems[row];

						if (!this->m_ItemToRowWidgetMap.contains(item))
						{
							this->m_ItemToRowWidgetMap[item] = std::move(this->GenerateNewWidget(item));
							this->m_OnItemScrolledIntoView.ExecuteIfSet(item);
						}

						this->m_ItemToRowWidgetMap[item]->Render();
						lastRowY = ImGui::GetCursorScreenPos().y;
					}

					if (!warmUp)
					{
						if (static_cast<int>(this->m_VisibleListStart) != clipper.DisplayStart || static_cast<int>(this->m_VisibleListEnd) != clipper.DisplayEnd)
						{
							this->m_VisibleListStart = static_cast<uint32>(clipper.DisplayStart);
							this->m_VisibleListEnd = static_cast<uint32>(clipper.DisplayEnd);
							clipRangeDirty = true;
						}
					}

					warmUp = !warmUp; // ping pong: warmup1 -> no warmup -> warmup2
				}
			}
			else
			{
				for (uint32 row = 0u; row < m_LinearizedItems.size(); ++row)
				{
					const ItemType& item = m_LinearizedItems[row];

					if (!this->m_ItemToRowWidgetMap.contains(item))
					{
						this->m_ItemToRowWidgetMap[item] = std::move(this->GenerateNewWidget(item));
						this->m_OnItemScrolledIntoView.ExecuteIfSet(item);
					}

					this->m_ItemToRowWidgetMap[item]->Render();
					lastRowY = ImGui::GetCursorScreenPos().y;
				}
			}

			if (m_PendingScrollItem.has_value() && !m_PendingScroll.IsValid())
			{
				const ItemType& want = *m_PendingScrollItem;
				auto it = std::find(m_LinearizedItems.begin(), m_LinearizedItems.end(), want);
				if (it != m_LinearizedItems.end())
				{
					m_PendingScroll.Row = static_cast<uint32>(std::distance(m_LinearizedItems.begin(), it));
					// keep CenterRatio default (0.35f) unless you want to carry one with the item request
				}
				m_PendingScrollItem.reset();
			}

			if (m_PendingScroll.IsValid() && clipStartPosY != FLT_MAX && !m_LinearizedItems.empty())
			{
				const uint32 maxRow = static_cast<uint32>(m_LinearizedItems.size() - 1);
				const uint32 clampedRow = (m_PendingScroll.Row > maxRow) ? maxRow : m_PendingScroll.Row;

				// Convert the desired row to local Y (relative to current window)
				const float localY = clipStartPosY + static_cast<float>(clampedRow) * clipItemsHeight;

				// Scroll so that 'localY' appears at the chosen spot in view (0=top, .5=center, 1=bottom)
				ImGui::SetScrollFromPosY(localY, m_PendingScroll.CenterRatio);

				m_PendingScroll = {}; // clear request
			}

			ImGui::EndTable();

			const ImVec2 rowMax = ImGui::GetItemRectMax();
			const ImVec2 rowMin = ImVec2(ImGui::GetItemRectMin().x, lastRowY);

			const float remainingHeight = rowMax.y - rowMin.y;
			if (remainingHeight > 0.0f && this->m_ClearSelectionOnEmptySpaceClick && ImGui::IsMouseHoveringRect(rowMin, rowMax) && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
				this->ClearSelection();

			if (clipRangeDirty)
				this->ReleaseInvisibleWidgets();
		}

		ImGui::PopStyleVar();

		if (this->m_Style.UseAlternatingRowColors)
			ImGui::PopStyleColor(2);
	}

	template<class ItemType>
	template<typename InstanceType>
	TreeView<ItemType>* TreeView<ItemType>::OnExpansionChanged(InstanceType* instance, void(InstanceType::* method)(const ItemType&)) noexcept
	{
		m_OnExpansionChanged = [instance, method](const ItemType& item) { return (instance->*method)(item); };
		return this;
	}

	template<class ItemType>
	template<typename InstanceType>
	TreeView<ItemType>* TreeView<ItemType>::OnGetChildren(InstanceType* instance, void(InstanceType::*method)(const ItemType&, std::vector<ItemType>&)) noexcept
	{
		m_OnGetChildren = [instance, method](const ItemType& item, std::vector<ItemType>& outChildren) { return (instance->*method)(item, outChildren); };
		return this;
	}

}