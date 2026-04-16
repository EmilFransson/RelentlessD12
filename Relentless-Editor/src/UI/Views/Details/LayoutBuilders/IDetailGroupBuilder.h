#pragma once

#include "DetailPropertyRowBuilder.h"
#include "AssetDetailPropertyRowBuilder.h"

#include "Property/AssetPropertyHandle.h"
#include "Property/PropertyHandle.h"
#include "Property/MultiPropertyHandle.h"
#include "Property/EntityPropertyHandle.h"

#include "UI/Nodes/AssetDetailNode.h"
#include "UI/Nodes/DetailNode.h"

namespace Relentless
{
	struct AssetData;

	class IDetailGroupBuilder
	{
	public:
		explicit IDetailGroupBuilder(const char* aName, Ref<DetailNode> aNode) noexcept;
		virtual ~IDetailGroupBuilder() noexcept = default;

		DetailPropertyRowBuilder<AssetData> AddAssetProperty(const char* aPropertyName, const AssetData& aAssetData) noexcept;
		
		template<typename DataType>
		DetailPropertyRowBuilder<DataType> AddProperty(const char* aPropertyName, Ref<IPropertyHandleBase> aPropertyHandle) noexcept;

		template<typename DataType>
		DetailPropertyRowBuilder<DataType> AddProperty(const char* aPropertyName, Callback<DataType()> aGetter, Callback<void(const DataType&)> aSetter) noexcept;

		template<typename DataType>
		DetailPropertyRowBuilder<DataType> AddProperty(const char* aPropertyName, Callback<DataType()> aGetter, Callback<void(const DataType&)> aSetter, const DataType& aDefaultValue) noexcept;

		template<typename DataType>
		DetailPropertyRowBuilder<DataType> AddProperty(const char* aPropertyName, Callback<DataType()> aGetter, Callback<void(const DataType&)> aSetter, Callback<DataType()> aDefaultGetter) noexcept;

		//NO_DISCARD bool ExistsProperty(const char* aPropertyName) const noexcept;
		//
		//NO_DISCARD const std::vector<Ref<DetailNode>>& GetNodes() const noexcept;
		bool m_IsExpanded = true;
	private:
		Ref<DetailNode> m_pGroupNode;
		String m_Name;
	};

	template<typename DataType>
	DetailPropertyRowBuilder<DataType> IDetailGroupBuilder::AddProperty(const char* aPropertyName, Ref<IPropertyHandleBase> aPropertyHandle) noexcept
	{
		//RLS_ASSERT(!ExistsProperty(aPropertyName), "[IDetailGroupBuilder::AddProperty]: Property already exists in category.");
		m_pGroupNode->AddChild(RLS_NEW DetailNode(aPropertyName, std::move(aPropertyHandle)));

		return DetailPropertyRowBuilder<DataType>(aPropertyName, m_pGroupNode->GetChildren().back());
	}

	template<typename DataType>
	DetailPropertyRowBuilder<DataType> IDetailGroupBuilder::AddProperty(const char* aPropertyName, Callback<DataType()> aGetter, Callback<void(const DataType&)> aSetter) noexcept
	{
		//RLS_ASSERT(!ExistsProperty(aPropertyName), "[IDetailGroupBuilder::AddProperty]: Property already exists in category.");
		
		m_pGroupNode->AddChild(RLS_NEW DetailNode(aPropertyName, RLS_NEW PropertyHandle<DataType>(std::move(aGetter), std::move(aSetter))));

		return DetailPropertyRowBuilder<DataType>(aPropertyName, m_pGroupNode->GetChildren().back());
	}

	template<typename DataType>
	DetailPropertyRowBuilder<DataType> IDetailGroupBuilder::AddProperty(const char* aPropertyName, Callback<DataType()> aGetter, Callback<void(const DataType&)> aSetter, const DataType& aDefaultValue) noexcept
	{
		//RLS_ASSERT(!ExistsProperty(aPropertyName), "[IDetailGroupBuilder::AddProperty]: Property already exists in category.");
		m_pGroupNode->AddChild(RLS_NEW DetailNode(aPropertyName, RLS_NEW PropertyHandle<DataType>(std::move(aGetter), std::move(aSetter), aDefaultValue)));

		return DetailPropertyRowBuilder<DataType>(aPropertyName, m_pGroupNode->GetChildren().back());
	}

	template<typename DataType>
	DetailPropertyRowBuilder<DataType> IDetailGroupBuilder::AddProperty(const char* aPropertyName, Callback<DataType()> aGetter, Callback<void(const DataType&)> aSetter, Callback<DataType()> aDefaultGetter) noexcept
	{
		//RLS_ASSERT(!ExistsProperty(aPropertyName), "[IDetailGroupBuilder::AddProperty]: Property already exists in category.");
		m_pGroupNode->AddChild(RLS_NEW DetailNode(aPropertyName, RLS_NEW PropertyHandle<DataType>(std::move(aGetter), std::move(aSetter), std::move(aDefaultGetter))));

		return DetailPropertyRowBuilder<DataType>(aPropertyName, m_pGroupNode->GetChildren().back());
	}

}