#pragma once
#include "ListView.h"

namespace Relentless
{
	#define INDEX_NONE -1

	struct ItemInfo
	{
		bool HasChildren = false;
		int32 ParentIndex = INDEX_NONE;
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

		NO_DISCARD const ItemInfo& GetItemInfo(const ItemType& pItem) const;

		template<typename InstanceType>
		TreeView<ItemType>* OnExpansionChanged(InstanceType* instance, void(InstanceType::*method)(const ItemType&)) noexcept;

		template<typename InstanceType>
		TreeView<ItemType>* OnGetChildren(InstanceType* instance, std::vector<ItemType>(InstanceType::*method)(const ItemType&)) noexcept;
	private:
		void OnRender() noexcept override;
	private:
		std::vector<ItemType> m_LinearizedItems;
		std::unordered_map<ItemType, ItemInfo> m_ItemInfos;

		Callback<void(const ItemType&)> m_OnExpansionChanged;
		Callback<std::vector<ItemType>(const ItemType&)> m_OnGetChildren;
	};

	template<class ItemType>
	const ItemInfo& TreeView<ItemType>::GetItemInfo(const ItemType& pItem) const
	{
		return m_ItemInfos[pItem];
	}

	template<class ItemType>
	void TreeView<ItemType>::OnRender() noexcept
	{
		ListView<ItemType>::OnRender();
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
	TreeView<ItemType>* TreeView<ItemType>::OnGetChildren(InstanceType* instance, std::vector<ItemType>(InstanceType::* method)(const ItemType&)) noexcept
	{
		m_OnGetChildren = [instance, method](const ItemType& item) { return (instance->*method)(item); };
		return this;
	}

}