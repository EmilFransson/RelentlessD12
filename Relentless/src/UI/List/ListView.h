#pragma once

#include "Callback/Callback.h"

#include "Input/Keyboard.h"

#include "UI/ContextMenu.h"
#include "UI/IWidget.h"
#include "UI/ITableRow.h"
#include "UI/UIManager.h"
#include "UI/IWidgetContainer.h"

namespace Relentless
{
	struct TableViewStyle
	{
		int Flags = 0;
		bool UseAlternatingRowColors = true;
		Color EvenRowColor = Colors::EvenRowColorDefault;
		Color OddRowColor = Colors::OddRowColorDefault;
		Color RowHoverColor = Colors::RowHoverColorDefault;
	};

	enum class ESelectionType : uint8 { Selected, Deselected };
	enum class ESelectionMode : uint8 { None, Single, SingleToggle, Multi };
	enum class EOrientation   : uint8 { Horizontal, Vertical };
	enum class ETableViewMode : uint8 { List, Tile, Tree };

	class Label;

	struct Column
	{
		Ref<Label> pLabel = nullptr;
		float Weight = 0.0f;
		int Flags = 0;
	};

	class HeaderRow
	{
	public:
		void AddColumn(const Column& newColumn) noexcept;
		NO_DISCARD const Column& GetColumn(uint32 index) const noexcept;
		NO_DISCARD uint32 GetNumColumns() const noexcept;

		void OnRender() noexcept;
		void SetIsPinned(bool isPinned) noexcept;
		void SetIsVisible(bool aIsVisible) noexcept;
	private:
		std::vector<Column> m_Columns;
		bool m_IsPinned = false;
		bool m_IsVisible = true;
	};

	template<class ItemType>
	class ITableViewBase : public IStylableWidget<ITableViewBase<ItemType>>
	{
		static_assert(Is_Pointer_Like<ItemType>::value, "[ITableViewBase]: Item is not of pointer type.");

	public:
		ITableViewBase(std::shared_ptr<HeaderRow> pHeaderRow) noexcept
			: m_pHeaderRow{pHeaderRow}
		{
			Keyboard::OnKeyStateChanged.Connect(this, &ITableViewBase<ItemType>::OnKeyStateChanged);
		}

		virtual ~ITableViewBase() noexcept
		{
			Keyboard::OnKeyStateChanged.Detach(this);
		}

		NO_DISCARD bool IsFocused() const noexcept { return m_IsFocused; }

		virtual void SelectAll() noexcept = 0;
	private:
		void OnKeyStateChanged(RLS_Key key, bool pressed) noexcept
		{
			switch (key)
			{
			case RLS_Key::LCtrl: 
				m_SelectionMode = pressed ? ESelectionMode::SingleToggle : ESelectionMode::Single;
				break;
			case RLS_Key::LShift:
				m_SelectionMode = pressed ? ESelectionMode::Multi : ESelectionMode::Single;
				break;
			case RLS_Key::A:
			{
				if (Keyboard::IsKeyDown(RLS_Key::LCtrl) && m_IsFocused)
					SelectAll();

				break;
			}
			}
		}
	protected:
		std::unordered_map<ItemType, Ref<ITableRow>> m_ItemToRowWidgetMap;
		std::unordered_set<ItemType> m_SelectedItems;
		std::unordered_set<ItemType> m_HighlightedItems;

		TableViewStyle m_Style;
		bool m_IsFocused = false;
		
		ESelectionMode m_SelectionMode			= ESelectionMode::Single;
		EOrientation m_Orientation				= EOrientation::Vertical;
		ETableViewMode m_Mode					= ETableViewMode::List;
		std::shared_ptr<HeaderRow> m_pHeaderRow = nullptr;
		const std::vector<ItemType>* m_pSource = nullptr;
	};

	template<class ItemType>
	class ListView : public ITableViewBase<ItemType>
	{
	public:
		ListView(std::shared_ptr<HeaderRow> pHeaderRow, const TableViewStyle& style = TableViewStyle()) noexcept;
		virtual ~ListView() noexcept override = default;

		virtual NO_DISCARD float CalcDesiredWidth() const noexcept override { return 0.0f; }

		void ClearHightlightedItems() noexcept;
		void ClearItemsSource() noexcept;
		void ClearSelection() noexcept;
		ListView<ItemType>* ClearSelectionOnClick(bool shouldClear) noexcept;

		NO_DISCARD const ItemType& GetItemFromWidget(const ITableRow* pWidgetToFind) const noexcept;
		NO_DISCARD uint32 GetNumGeneratedWidgets() const noexcept;
		NO_DISCARD uint32 GetNumItemsSelected() const noexcept;
		virtual NO_DISCARD String GetReferencerName() const noexcept;
		uint32 GetSelectedItems(std::vector<ItemType>& outSelectedItems) const noexcept;
		NO_DISCARD Ref<ITableRow> GetRowWidget(const ItemType& item) const noexcept;

		NO_DISCARD bool HasValidItemSource() const noexcept;

		NO_DISCARD bool IsItemHighlighted(const ItemType& item) const noexcept;
		NO_DISCARD bool IsItemSelected(const ItemType& item) const noexcept;
		NO_DISCARD bool IsItemVisible(const ItemType& item) const noexcept;

		template<typename InstanceType>
		ListView<ItemType>* OnClick(InstanceType* instance, void(InstanceType::* method)(const PointerInfo& pointerInfo, const ItemType& item)) noexcept;

		template<typename InstanceType>
		ListView<ItemType>* OnContextMenuOpening(InstanceType* instance, Ref<ContextMenu>(InstanceType::* method)(const ItemType& item)) noexcept;

		template<typename InstanceType>
		ListView<ItemType>* OnDebugItemToString(const InstanceType* instance, String(InstanceType::*method)(const ItemType&) const) noexcept;

		template<typename InstanceType>
		ListView<ItemType>* OnDoubleClick(InstanceType* instance, void(InstanceType::* method)(const ItemType& item)) noexcept;

		template<typename InstanceType>
		ListView<ItemType>* OnEntryInitialized(InstanceType* instance, void(InstanceType::* method)(const ItemType&, const Ref<ITableRow>&)) noexcept;

		template<typename InstanceType>
		ListView<ItemType>* OnIsSelectableOrNavigable(InstanceType* instance, bool(InstanceType::*method)(const ItemType&)) noexcept;

		template<typename InstanceType>
		ListView<ItemType>* OnItemScrolledIntoView(InstanceType* instance, void(InstanceType::* method)(const ItemType&)) noexcept;

		template<typename InstanceType>
		ListView<ItemType>* OnGenerateRow(InstanceType* instance, Ref<ITableRow>(InstanceType::* method)(const ItemType&)) noexcept;

		template<typename InstanceType>
		ListView<ItemType>* OnRequestSource(InstanceType* instance, const std::vector<ItemType>*(InstanceType::* method)()) noexcept;

		template<typename InstanceType>
		ListView<ItemType>* OnRowReleased(InstanceType* instance, void(InstanceType::* method)(const Ref<ITableRow>&)) noexcept;

		template<typename InstanceType>
		ListView<ItemType>* OnSelectionChanged(InstanceType* instance, void(InstanceType::*method)(const ItemType&, ESelectionType)) noexcept;

		Broadcaster<void(bool)> OnFocusChanged;

		void SelectAll() noexcept override;
		void SetItemHighlighted(const ItemType& item, bool highlight) noexcept;
		void SetItemSelection(const ItemType& item, ESelectionType selectionType) noexcept;
		void SetSelectionMode(ESelectionMode mode) noexcept;
		void SetStyle(const TableViewStyle& style) noexcept;

		void TriggerFocusChange(bool focused) noexcept;
	protected:
		virtual NO_DISCARD Ref<ITableRow> GenerateNewWidget(const ItemType& item) noexcept;
		NO_DISCARD uint32 GetItemIndex(const ItemType& item) const noexcept;
		virtual void OnRowClicked(const PointerInfo& pointerInfo, const ItemType& pItem) noexcept;
		void ReleaseInvisibleWidgets() noexcept;
	private:
		void OnRender() noexcept override;
		void OnRowDoubleClicked(const ItemType& pItem) noexcept;

	protected:
		Callback<void(const PointerInfo& pointerInfo, const ItemType& item)> m_OnClick;
		Callback<Ref<ContextMenu>(const ItemType& item)> m_OnContextMenuOpening;
		Callback<String(const ItemType&)> m_OnDebugItemToString;
		Callback<void(const ItemType& item)> m_OnDoubleClick;
		Callback<void(const ItemType&, const Ref<ITableRow>&)> m_OnEntryInitialized;
		Callback<void(const Ref<ITableRow>& item)> m_OnMouseEnterRow;
		Callback<void(const Ref<ITableRow>& item)> m_OnMouseExitRow;
		Callback<Ref<ITableRow>(const ItemType&)> m_OnGenerateRow;
		Callback<bool(const ItemType&)> m_OnIsSelectableOrNavigable;
		Callback<void(const ItemType&)> m_OnItemScrolledIntoView;
		Callback<const std::vector<ItemType>*()> m_OnRequestSource;
		Callback<void(const Ref<ITableRow>&)> m_OnRowReleased;
		Callback<void(const ItemType&, ESelectionType)> m_OnSelectionChanged;
	
		std::optional<ItemType> m_ItemToScrollIntoView;
		std::optional<ItemType> m_RangeSelectionStart;

		uint32 m_VisibleListStart = 0u;
		uint32 m_VisibleListEnd = 0u;

		bool m_ClearSelectionOnEmptySpaceClick = true;
		bool m_ContextMenuOpen = false;
	};

	template<class ItemType>
	String ListView<ItemType>::GetReferencerName() const noexcept
	{
		return "Unnamed";
	}

	template<class ItemType>
	Ref<ITableRow> ListView<ItemType>::GetRowWidget(const ItemType& item) const noexcept
	{
		RLS_ASSERT(this->m_ItemToRowWidgetMap.contains(item), "[ListView::GetRowWidget]: Widget does not exist for item '{}'", m_OnDebugItemToString.IsSet() ? m_OnDebugItemToString(item) : "Unknown");
		return this->m_ItemToRowWidgetMap.at(item);
	}

	template<class ItemType>
	bool ListView<ItemType>::IsItemVisible(const ItemType& item) const noexcept
	{
		return this->m_ItemToRowWidgetMap.contains(item);
	}

	template<class ItemType>
	void ListView<ItemType>::ReleaseInvisibleWidgets() noexcept
	{
		if (!this->m_pSource)
			return;

		const std::vector<ItemType>& source = *this->m_pSource;
		if (source.empty())
			return;

		std::unordered_set<ItemType> visibleItems;
		visibleItems.reserve(m_VisibleListEnd - m_VisibleListStart);
		
		for (uint32 row = m_VisibleListStart; row < m_VisibleListEnd; ++row)
			visibleItems.insert(source[row]);

		std::erase_if(this->m_ItemToRowWidgetMap, [&](auto& keyValuePair)
			{
				if (visibleItems.find(keyValuePair.first) == visibleItems.end())
				{
					m_OnRowReleased.ExecuteIfSet(keyValuePair.second);
					return true;
				}

				return false;
			});
	}

	template<class ItemType>
	uint32 ListView<ItemType>::GetNumGeneratedWidgets() const noexcept
	{
		return static_cast<uint32>(this->m_ItemToRowWidgetMap.size());
	}

	template<class ItemType>
	void ListView<ItemType>::SelectAll() noexcept
	{
		if (!this->m_pSource)
			return;

		for (const auto& item : *this->m_pSource)
		{
			if (!IsItemSelected(item))
				SetItemSelection(item, ESelectionType::Selected);
		}
	}

	template<class ItemType>
	void ListView<ItemType>::TriggerFocusChange(bool focused) noexcept
	{
		if (this->m_IsFocused != focused)
		{
			this->m_IsFocused = focused;
			OnFocusChanged(this->m_IsFocused);
		}
	}

	template<class ItemType>
	uint32 ListView<ItemType>::GetSelectedItems(std::vector<ItemType>& outSelectedItems) const noexcept
	{
		std::vector<ItemType> selectedItems(this->m_SelectedItems.begin(), this->m_SelectedItems.end());
		outSelectedItems = std::move(selectedItems);

		return static_cast<uint32>(outSelectedItems.size());
	}

	template<class ItemType>
	uint32 ListView<ItemType>::GetItemIndex(const ItemType& item) const noexcept
	{
		for (size_t i = 0u; i < this->m_pSource->size(); ++i)
		{
			if (item == (*this->m_pSource)[i])
				return static_cast<uint32>(i);
		}
	}

	template<class ItemType>
	Ref<ITableRow> ListView<ItemType>::GenerateNewWidget(const ItemType& item) noexcept
	{
		Ref<ITableRow> pNewRow = m_OnGenerateRow(item);

		pNewRow->OnClicked([&](const PointerInfo& info) { OnRowClicked(info, item); });
		pNewRow->OnDoubleClicked(std::bind(&ListView<ItemType>::OnRowDoubleClicked, this, item));

		if (m_OnEntryInitialized.IsSet())
			m_OnEntryInitialized(item, pNewRow);

		return std::move(pNewRow);
	}

	template<class ItemType>
	bool ListView<ItemType>::IsItemHighlighted(const ItemType& item) const noexcept
	{
		return this->m_HighlightedItems.contains(item);
	}

	template<class ItemType>
	bool ListView<ItemType>::IsItemSelected(const ItemType& item) const noexcept
	{
		return this->m_SelectedItems.contains(item);
	}

	template<class ItemType>
	void ListView<ItemType>::OnRowClicked(const PointerInfo& pointerInfo, const ItemType& pItem) noexcept
	{
#if defined RLS_DEBUG || defined RLS_RELWITHDEBINFO
		String message = "[ListView]: Clicked on item: ";
		if (m_OnDebugItemToString.IsSet())
			message += m_OnDebugItemToString(pItem);
		else
			message += "Unknown";

		RLS_CORE_INFO(message.c_str());
#endif

		if (pointerInfo.EffectingButton == RLS_Button::Left)
		{
			//For now, as it conflicts with row clicks, return when context menu is open!
			if (m_ContextMenuOpen)
				return;

			switch (this->m_SelectionMode)
			{
			case ESelectionMode::Single:
			{
				if (!IsItemSelected(pItem))
				{
					ClearSelection();
					SetItemSelection(pItem, ESelectionType::Selected);
				}
				else
				{
					const std::vector<ItemType> selectedItems(this->m_SelectedItems.begin(), this->m_SelectedItems.end());
					for (const auto& selectedItem : selectedItems)
					{
						if (selectedItem != pItem)
							SetItemSelection(selectedItem, ESelectionType::Deselected);
					}

					m_RangeSelectionStart = pItem;
				}

				break;
			}
			case ESelectionMode::SingleToggle:
			{
				if (!IsItemSelected(pItem))
					SetItemSelection(pItem, ESelectionType::Selected);
				else
				{
					SetItemSelection(pItem, ESelectionType::Deselected);
					if (this->m_SelectedItems.empty())
						m_RangeSelectionStart = std::nullopt;
					else
						m_RangeSelectionStart = *(this->m_SelectedItems.begin());
				}

				break;
			}
			case ESelectionMode::Multi:
			{
				if (m_RangeSelectionStart == std::nullopt && !IsItemSelected(pItem))
					SetItemSelection(pItem, ESelectionType::Selected);
				else
				{
					uint32 rangeBegin = GetItemIndex(m_RangeSelectionStart.value());
					uint32 rangeEnd = GetItemIndex(pItem);

					if (rangeBegin > rangeEnd)
						std::swap(rangeBegin, rangeEnd);

					for (uint32 i = rangeBegin; i <= rangeEnd; ++i)
					{
						if (!IsItemSelected((*this->m_pSource)[i]))
							SetItemSelection((*this->m_pSource)[i], ESelectionType::Selected);
					}
				}

				break;
			}
			}
		}
		else if (pointerInfo.EffectingButton == RLS_Button::Right)
		{
			if (!IsItemSelected(pItem))
			{
				ClearSelection();
				SetItemSelection(pItem, ESelectionType::Selected);
			}

			if (m_OnContextMenuOpening.IsSet())
			{
				Ref<ContextMenu> pContextMenu = m_OnContextMenuOpening(pItem);
				pContextMenu->OnClosed.Connect([this, clearOnEmptyClick = m_ClearSelectionOnEmptySpaceClick]() 
					{ 
						m_ClearSelectionOnEmptySpaceClick = clearOnEmptyClick; 
						m_ContextMenuOpen = false;
					});
				m_ClearSelectionOnEmptySpaceClick = false;
				UIManager::Get().SetActiveContextMenu(pContextMenu);
				m_ContextMenuOpen = true;
			}
		}

		m_OnClick.ExecuteIfSet(pointerInfo, pItem);
	}

	template<class ItemType>
	void ListView<ItemType>::OnRowDoubleClicked(const ItemType& pItem) noexcept
	{
#if defined RLS_DEBUG || defined RLS_RELWITHDEBINFO
		String message = "[ListView]: Double clicked on item: ";
		if (m_OnDebugItemToString.IsSet())
			message += m_OnDebugItemToString(pItem);
		else
			message += "Unknown";

		RLS_CORE_INFO(message.c_str());
#endif

		m_OnDoubleClick.ExecuteIfSet(pItem);
	}

	template<class ItemType>
	void ListView<ItemType>::ClearSelection() noexcept
	{
		if (this->m_SelectedItems.empty())
			return;

		std::vector<ItemType> selectedItemsCopy(this->m_SelectedItems.begin(), this->m_SelectedItems.end());
		
		for (const ItemType& item : selectedItemsCopy)
			SetItemSelection(item, ESelectionType::Deselected);
	
#if defined RLS_DEBUG || defined RLS_RELWITHDEBINFO
		RLS_CORE_INFO("[ListView]: Cleared selection");
#endif
	}

	template<class ItemType>
	ListView<ItemType>* ListView<ItemType>::ClearSelectionOnClick(bool shouldClear) noexcept
	{
		m_ClearSelectionOnEmptySpaceClick = shouldClear;
		return this;
	}

	template<class ItemType>
	ListView<ItemType>::ListView(std::shared_ptr<HeaderRow> pHeaderRow, const TableViewStyle& style) noexcept
		: ITableViewBase<ItemType>(pHeaderRow)
	{
		this->m_Mode = ETableViewMode::List;
		this->m_Orientation = EOrientation::Vertical;
		this->m_Style = style;
	}

	template<class ItemType>
	void ListView<ItemType>::ClearHightlightedItems() noexcept
	{
		this->m_HighlightedItems.clear();
	}

	template<class ItemType>
	void ListView<ItemType>::ClearItemsSource() noexcept
	{
		this->m_pSource = nullptr;
		this->m_ItemToRowWidgetMap.clear();
	}

	template<class ItemType>
	const ItemType& ListView<ItemType>::GetItemFromWidget(const ITableRow* pWidgetToFind) const noexcept
	{
		auto it = std::find_if(this->m_ItemToRowWidgetMap.begin(), this->m_ItemToRowWidgetMap.end(), [&](const auto& pair)
{
				return pair.second.Get() == pWidgetToFind;
			}
		);

		return it->first;
	}

	template<class ItemType>
	bool ListView<ItemType>::HasValidItemSource() const noexcept
	{
		return this->m_pSource != nullptr;
	}

	template<class ItemType>
	uint32 ListView<ItemType>::GetNumItemsSelected() const noexcept
	{
		return this->m_SelectedItems.size();
	}

	template<class ItemType>
	template<typename InstanceType>
	ListView<ItemType>* ListView<ItemType>::OnClick(InstanceType* instance, void(InstanceType::*method)(const PointerInfo& pointerInfo, const ItemType& item)) noexcept
	{
		m_OnClick = [instance, method](const PointerInfo& pointerInfo, const ItemType& item) { return (instance->*method)(pointerInfo, item); };
		return this;
	}

	template<class ItemType>
	template<typename InstanceType>
	ListView<ItemType>* ListView<ItemType>::OnContextMenuOpening(InstanceType* instance, Ref<ContextMenu>(InstanceType::*method)(const ItemType& item)) noexcept
	{
		m_OnContextMenuOpening = [instance, method](const ItemType& item) { return (instance->*method)(item); };
		return this;
	}

	template<class ItemType>
	template<typename InstanceType>
	ListView<ItemType>* ListView<ItemType>::OnDebugItemToString(const InstanceType* instance, String(InstanceType::*method)(const ItemType&) const) noexcept
	{
		m_OnDebugItemToString = [instance, method](const ItemType& item) { return (instance->*method)(item); };
		return this;
	}

	template<class ItemType>
	template<typename InstanceType>
	ListView<ItemType>* ListView<ItemType>::OnDoubleClick(InstanceType* instance, void(InstanceType::*method)(const ItemType& item)) noexcept
	{
		m_OnDoubleClick = [instance, method](const ItemType& item) { return (instance->*method)(item); };
		return this;
	}

	template<class ItemType>
	template<typename InstanceType>
	ListView<ItemType>* ListView<ItemType>::OnEntryInitialized(InstanceType* instance, void(InstanceType::*method)(const ItemType&, const Ref<ITableRow>&)) noexcept
	{
		m_OnEntryInitialized = [this, instance, method](const ItemType& item, const Ref<ITableRow>& pTableRow) { return (instance->*method)(item, pTableRow); };
		return this;
	}

	template<class ItemType>
	template<typename InstanceType>
	ListView<ItemType>* ListView<ItemType>::OnGenerateRow(InstanceType* instance, Ref<ITableRow>(InstanceType::*method)(const ItemType&)) noexcept
	{
		m_OnGenerateRow = [instance, method](const ItemType& item) { return (instance->*method)(item); };
		return this;
	}

	template<class ItemType>
	template<typename InstanceType>
	ListView<ItemType>* ListView<ItemType>::OnIsSelectableOrNavigable(InstanceType* instance, bool(InstanceType::*method)(const ItemType&)) noexcept
	{
		m_OnIsSelectableOrNavigable = [instance, method](const ItemType& item) { return (instance->*method)(item); };
		return this;
	}

	template<class ItemType>
	template<typename InstanceType>
	ListView<ItemType>* ListView<ItemType>::OnItemScrolledIntoView(InstanceType* instance, void(InstanceType::*method)(const ItemType&)) noexcept
	{
		m_OnItemScrolledIntoView = [instance, method](const ItemType& item) { return (instance->*method)(item); };
		return this;
	}

	template<class ItemType>
	template<typename InstanceType>
	ListView<ItemType>* ListView<ItemType>::OnRequestSource(InstanceType* instance, const std::vector<ItemType>*(InstanceType::*method)()) noexcept
	{
		m_OnRequestSource = [instance, method]() { return (instance->*method)(); };
		return this;
	}

	template<class ItemType>
	template<typename InstanceType>
	ListView<ItemType>* ListView<ItemType>::OnRowReleased(InstanceType* instance, void(InstanceType::*method)(const Ref<ITableRow>&)) noexcept
	{
		m_OnRowReleased = [instance, method](const Ref<ITableRow>& pRow) { return (instance->*method)(pRow); };
		return this;
	}

	template<class ItemType>
	template<typename InstanceType>
	ListView<ItemType>* ListView<ItemType>::OnSelectionChanged(InstanceType* instance, void(InstanceType::*method)(const ItemType&, ESelectionType)) noexcept
	{
		m_OnSelectionChanged = [instance, method](const ItemType& item, ESelectionType selectionType) { return (instance->*method)(item, selectionType); };
		return this;
	}

	template<class ItemType>
	void ListView<ItemType>::SetItemHighlighted(const ItemType& item, bool highlight) noexcept
	{
		if (highlight)
			this->m_HighlightedItems.insert(item);
		else
			this->m_HighlightedItems.erase(item);
	}

	template<class ItemType>
	void ListView<ItemType>::SetItemSelection(const ItemType& item, ESelectionType selectionType) noexcept
	{
		if (selectionType == ESelectionType::Selected)
		{
			bool selectable = true;
			if (m_OnIsSelectableOrNavigable.IsSet())
				selectable = m_OnIsSelectableOrNavigable(item);

			if (!selectable)
				return;

			RLS_ASSERT(!this->m_SelectedItems.contains(item), "[ListView::SetItemSelection]: Item {} is already selected.", m_OnDebugItemToString.IsSet() ? m_OnDebugItemToString(item) : "Unknown");
			this->m_SelectedItems.insert(item);
			m_OnSelectionChanged.ExecuteIfSet(item, selectionType);

			//if (m_RangeSelectionStart == std::nullopt)
				m_RangeSelectionStart = item;

			RLS_CORE_INFO("[ListView::SetItemSelection]: Selected item: '{}'", m_OnDebugItemToString.IsSet() ? m_OnDebugItemToString(item) : "Unknown");
		}
		else
		{
			RLS_ASSERT(this->m_SelectedItems.contains(item), "[ListView::SetItemSelection]: Item '{}' is not selected.", m_OnDebugItemToString.IsSet() ? m_OnDebugItemToString(item) : "Unknown");
			this->m_SelectedItems.erase(item);
			m_OnSelectionChanged.ExecuteIfSet(item, selectionType);

			if (m_RangeSelectionStart == item)
				m_RangeSelectionStart = std::nullopt;

			RLS_CORE_INFO("[ListView::SetItemSelection]: Deselected item: '{}'", m_OnDebugItemToString.IsSet() ? m_OnDebugItemToString(item) : "Unknown");
		}
	}

	template<class ItemType>
	void ListView<ItemType>::SetSelectionMode(ESelectionMode mode) noexcept
	{
		ITableViewBase<ItemType>::m_SelectionMode = mode;
	}

	template<class ItemType>
	void ListView<ItemType>::SetStyle(const TableViewStyle& style) noexcept
	{
		ITableViewBase<ItemType>::m_Style = style;
	}

	//---------- PRIVATE FUNCTIONS --------------

	template<class ItemType>
	void ListView<ItemType>::OnRender() noexcept
	{
		this->m_pSource = m_OnRequestSource();

		RLS_ASSERT(HasValidItemSource(), "[ListView::OnRender]: Items source is invalid.");

		const std::vector<ItemType>& source = *(this->m_pSource);
		if (source.empty())
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
			clipper.Begin(source.size());

			bool warmUp = true;
			bool clipRangeDirty = false;
			float lastRowY = 0.0f;

			while (clipper.Step())
			{
				for (uint32 row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row)
				{
					const ItemType& item = source[row];
					
					if (!this->m_ItemToRowWidgetMap.contains(item))
					{
						this->m_ItemToRowWidgetMap[item] = std::move(GenerateNewWidget(item));
						m_OnItemScrolledIntoView.ExecuteIfSet(item);
					}

					this->m_ItemToRowWidgetMap[item]->Render();
					lastRowY = ImGui::GetCursorScreenPos().y;
				}

				if (!warmUp)
				{
					if (m_VisibleListStart != clipper.DisplayStart || m_VisibleListEnd != clipper.DisplayEnd)
					{
						m_VisibleListStart = clipper.DisplayStart;
						m_VisibleListEnd = clipper.DisplayEnd;
						clipRangeDirty = true;
					}
				}
				
				warmUp = !warmUp; // ping pong: warmup1 -> no warmup -> warmup2
			}

			ImGui::EndTable();

			const ImVec2 rowMax = ImGui::GetItemRectMax();
			const ImVec2 rowMin = ImVec2(ImGui::GetItemRectMin().x, lastRowY);

			const float remainingHeight = rowMax.y - rowMin.y;
			if (remainingHeight > 0.0f && m_ClearSelectionOnEmptySpaceClick && ImGui::IsMouseHoveringRect(rowMin, rowMax) && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
				ClearSelection();

			if (clipRangeDirty)
				ReleaseInvisibleWidgets();
		}

		ImGui::PopStyleVar();

		if (this->m_Style.UseAlternatingRowColors)
			ImGui::PopStyleColor(2);
	}
}