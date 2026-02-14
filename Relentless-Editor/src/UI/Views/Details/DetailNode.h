#pragma once
#include "UI/Widgets/ITableRow.h"

#include "Property/PropertyHandle.h"

namespace Relentless
{
	class ITableRow;
	struct ItemInfo;

	class DetailNode : public RefCounted<DetailNode>
	{
	public:
		explicit DetailNode(const char* aName) noexcept;
		explicit DetailNode(const char* aName, Ref<IPropertyHandleBase> aPropertyHandle) noexcept;
		virtual ~DetailNode() noexcept = default;

		NO_DISCARD const std::vector<Ref<DetailNode>>& GetChildren() const noexcept;
		NO_DISCARD const String& GetName() const noexcept;
		NO_DISCARD Ref<IPropertyHandleBase> GetPropertyHandle() const noexcept;

		template<typename InstanceType>
		void OnRequestRow(InstanceType* aInstance, Ref<ITableRow>(InstanceType::*aMethod)(const ItemInfo&)) noexcept;

		template<typename T>
		void OnRequestRow(T&& aCallback) noexcept;

		NO_DISCARD Ref<ITableRow> RequestRowWidget(const ItemInfo& aItemInfo) noexcept;

		void SetChildren(const std::vector<Ref<DetailNode>>& someChildren) noexcept;
	private:
		std::vector<Ref<DetailNode>> m_Children;
		String m_Name;
		Callback<Ref<ITableRow>(const ItemInfo&)> m_OnRequestRowCallback;

		Ref<IPropertyHandleBase> m_pPropertyHandle = nullptr;
	};

	template<typename InstanceType>
	void DetailNode::OnRequestRow(InstanceType* aInstance, Ref<ITableRow>(InstanceType::*aMethod)(const ItemInfo&)) noexcept
	{
		m_OnRequestRowCallback = [aInstance, aMethod](const ItemInfo& aItemInfo) { return (aInstance->*aMethod)(aItemInfo); };
	}

	template<typename T>
	void DetailNode::OnRequestRow(T&& aCallback) noexcept
	{
		m_OnRequestRowCallback = Callback<Ref<ITableRow>(const ItemInfo& aItemInfo)>(std::forward<T>(aCallback));
	}
}