#pragma once
#include "AssetDetailPropertyRowBuilder.h"
#include "DetailPropertyRowBuilder.h"
#include "IDetailGroupBuilder.h"

#include "Property/AssetPropertyHandle.h"
#include "Property/PropertyHandle.h"
#include "Property/MultiPropertyHandle.h"
#include "Property/EntityPropertyHandle.h"

#include "UI/Nodes/AssetDetailNode.h"
#include "UI/Nodes/DetailNode.h"

namespace Relentless
{
	struct AssetData;
	class IDetailLayoutBuilder;

	struct DetailEntry
	{
		bool IsGroup			= false;
		Ref<DetailNode> Node	= nullptr;
		String GroupName		= "";
	};

	struct HeaderAction
	{
		String Label;
		Callback<void()> OnClicked;
		bool CloseOnSelection = true;
	};

	class IDetailCategoryBuilder
	{
	public:
		IDetailCategoryBuilder(const char* aName, IDetailLayoutBuilder& aParent) noexcept;
		virtual ~IDetailCategoryBuilder() noexcept = default;

		NO_DISCARD DetailNode& AddProperty(const char* aPropertyName) noexcept;

		DetailPropertyRowBuilder<AssetData> AddAssetProperty(const char* aPropertyName, const AssetData& aAssetData) noexcept;

		template<typename DataType>
		DetailPropertyRowBuilder<DataType> AddProperty(const char* aPropertyName, Ref<IPropertyHandleBase> aPropertyHandle) noexcept;

		template<typename DataType>
		DetailPropertyRowBuilder<DataType> AddProperty(const char* aPropertyName, Callback<DataType()> aGetter, Callback<void(const DataType&)> aSetter) noexcept;

		template<typename DataType>
		DetailPropertyRowBuilder<DataType> AddProperty(const char* aPropertyName, Callback<DataType()> aGetter, Callback<void(const DataType&)> aSetter, const DataType& aDefaultValue) noexcept;

		template<typename DataType>
		DetailPropertyRowBuilder<DataType> AddProperty(const char* aPropertyName, Callback<DataType()> aGetter, Callback<void(const DataType&)> aSetter, Callback<DataType()> aDefaultGetter) noexcept;

		NO_DISCARD IDetailLayoutBuilder& GetLayoutBuilder() const noexcept;
		NO_DISCARD const std::vector<HeaderAction>& GetHeaderActions() const noexcept;

		NO_DISCARD IDetailGroupBuilder EditGroup(const char* aGroupName) noexcept;
		NO_DISCARD bool ExistsProperty(const char* aPropertyName) const noexcept;
		
		NO_DISCARD const std::vector<DetailEntry>& GetEntries() const noexcept;

		template<typename InstanceType>
		IDetailCategoryBuilder& AddHeaderAction(StringView aLabel, InstanceType* aInstanceType, void(InstanceType::*aMethod)())
		{
			HeaderAction& headerAction = m_HeaderActions.emplace_back();
			headerAction.Label = aLabel;
			headerAction.OnClicked = [aInstanceType, aMethod]() { return (aInstanceType->*aMethod)(); };

			return *this;
		}

		template<typename Func>
		IDetailCategoryBuilder& AddHeaderAction(StringView aLabel, Func&& aCallback)
		{
			HeaderAction& headerAction = m_HeaderActions.emplace_back();
			headerAction.Label = aLabel;
			headerAction.OnClicked = Callback<void()>(std::forward<Func>(aCallback));

			return *this;
		}
		
		bool m_IsExpanded = true;
	private:
		std::vector<DetailEntry> m_Entries;
		std::vector<HeaderAction> m_HeaderActions;
		String m_Name;
		IDetailLayoutBuilder& m_DetailLayoutBuilder;
	};

	template<typename DataType>
	DetailPropertyRowBuilder<DataType> IDetailCategoryBuilder::AddProperty(const char* aPropertyName, Callback<DataType()> aGetter, Callback<void(const DataType&)> aSetter) noexcept
	{
		RLS_ASSERT(!ExistsProperty(aPropertyName), "[IDetailCategoryBuilder::AddProperty]: Property already exists in category.");
		m_Entries.push_back({ .IsGroup = false, .Node = RLS_NEW DetailNode(aPropertyName, RLS_NEW PropertyHandle<DataType>(std::move(aGetter), std::move(aSetter))) });

		return DetailPropertyRowBuilder<DataType>(aPropertyName, m_Entries.back().Node);
	}

	template<typename DataType>
	DetailPropertyRowBuilder<DataType> IDetailCategoryBuilder::AddProperty(const char* aPropertyName, Callback<DataType()> aGetter, Callback<void(const DataType&)> aSetter, const DataType& aDefaultValue) noexcept
	{
		RLS_ASSERT(!ExistsProperty(aPropertyName), "[IDetailCategoryBuilder::AddProperty]: Property already exists in category.");
		m_Entries.push_back({ .IsGroup = false, .Node = RLS_NEW DetailNode(aPropertyName, RLS_NEW PropertyHandle<DataType>(std::move(aGetter), std::move(aSetter), aDefaultValue)) });

		return DetailPropertyRowBuilder<DataType>(aPropertyName, m_Entries.back().Node);
	}

	template<typename DataType>
	DetailPropertyRowBuilder<DataType> IDetailCategoryBuilder::AddProperty(const char* aPropertyName, Callback<DataType()> aGetter, Callback<void(const DataType&)> aSetter, Callback<DataType()> aDefaultGetter) noexcept
	{
		RLS_ASSERT(!ExistsProperty(aPropertyName), "[IDetailCategoryBuilder::AddProperty]: Property already exists in category.");
		m_Entries.push_back({ .IsGroup = false, .Node = RLS_NEW DetailNode(aPropertyName, RLS_NEW PropertyHandle<DataType>(std::move(aGetter), std::move(aSetter), std::move(aDefaultGetter))) });

		return DetailPropertyRowBuilder<DataType>(aPropertyName, m_Entries.back().Node);
	}

	template<typename DataType>
	DetailPropertyRowBuilder<DataType>
	IDetailCategoryBuilder::AddProperty(const char* aPropertyName, Ref<IPropertyHandleBase> aPropertyHandle) noexcept
	{
		RLS_ASSERT(!ExistsProperty(aPropertyName), "[IDetailCategoryBuilder::AddProperty]: Property already exists in category.");
		m_Entries.push_back({ .IsGroup = false, .Node = RLS_NEW DetailNode(aPropertyName, std::move(aPropertyHandle)) });

		return DetailPropertyRowBuilder<DataType>(aPropertyName, m_Entries.back().Node);
	}

}