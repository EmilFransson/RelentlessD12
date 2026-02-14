#pragma once
#include "UI/Views/Details/DetailNode.h"

#include "DetailPropertyRowBuilder.h"

#include "Property/PropertyHandle.h"
#include "Property/MultiPropertyHandle.h"
#include "Property/EntityPropertyHandle.h"

namespace Relentless
{
	class IDetailCategoryBuilder
	{
	public:
		explicit IDetailCategoryBuilder(const char* aName) noexcept;
		virtual ~IDetailCategoryBuilder() noexcept = default;

		NO_DISCARD DetailNode& AddProperty(const char* aPropertyName) noexcept;

		template<typename DataType>
		DetailPropertyRowBuilder<DataType> AddProperty(const char* aPropertyName, Ref<IPropertyHandleBase> aPropertyHandle) noexcept;

		template<typename DataType>
		DetailPropertyRowBuilder<DataType> AddProperty(const char* aPropertyName, Callback<DataType()> aGetter, Callback<void(const DataType&)> aSetter) noexcept;

		template<typename DataType>
		DetailPropertyRowBuilder<DataType> AddProperty(const char* aPropertyName, Callback<DataType()> aGetter, Callback<void(const DataType&)> aSetter, const DataType& aDefaultValue) noexcept;

		template<typename DataType>
		DetailPropertyRowBuilder<DataType> AddProperty(const char* aPropertyName, Callback<DataType()> aGetter, Callback<void(const DataType&)> aSetter, Callback<DataType()> aDefaultGetter) noexcept;

		NO_DISCARD bool ExistsProperty(const char* aPropertyName) const noexcept;
		
		NO_DISCARD const std::vector<Ref<DetailNode>>& GetNodes() const noexcept;
		
		bool m_IsExpanded = true;
	private:
		std::vector<Ref<DetailNode>> m_Nodes;
		String m_Name;
	};

	template<typename DataType>
	DetailPropertyRowBuilder<DataType> IDetailCategoryBuilder::AddProperty(const char* aPropertyName, Callback<DataType()> aGetter, Callback<void(const DataType&)> aSetter) noexcept
	{
		RLS_ASSERT(!ExistsProperty(aPropertyName), "[IDetailCategoryBuilder::AddProperty]: Property already exists in category.");
		m_Nodes.push_back(RLS_NEW DetailNode(aPropertyName, RLS_NEW PropertyHandle<DataType>(std::move(aGetter), std::move(aSetter))));

		return DetailPropertyRowBuilder<DataType>(aPropertyName, m_Nodes.back());
	}

	template<typename DataType>
	DetailPropertyRowBuilder<DataType> IDetailCategoryBuilder::AddProperty(const char* aPropertyName, Callback<DataType()> aGetter, Callback<void(const DataType&)> aSetter, const DataType& aDefaultValue) noexcept
	{
		RLS_ASSERT(!ExistsProperty(aPropertyName), "[IDetailCategoryBuilder::AddProperty]: Property already exists in category.");
		m_Nodes.push_back(RLS_NEW DetailNode(aPropertyName, RLS_NEW PropertyHandle<DataType>(std::move(aGetter), std::move(aSetter), aDefaultValue)));

		return DetailPropertyRowBuilder<DataType>(aPropertyName, m_Nodes.back());
	}

	template<typename DataType>
	DetailPropertyRowBuilder<DataType> IDetailCategoryBuilder::AddProperty(const char* aPropertyName, Callback<DataType()> aGetter, Callback<void(const DataType&)> aSetter, Callback<DataType()> aDefaultGetter) noexcept
	{
		RLS_ASSERT(!ExistsProperty(aPropertyName), "[IDetailCategoryBuilder::AddProperty]: Property already exists in category.");
		m_Nodes.push_back(RLS_NEW DetailNode(aPropertyName, RLS_NEW PropertyHandle<DataType>(std::move(aGetter), std::move(aSetter), std::move(aDefaultGetter))));

		return DetailPropertyRowBuilder<DataType>(aPropertyName, m_Nodes.back());
	}

	template<typename DataType>
	DetailPropertyRowBuilder<DataType>
	IDetailCategoryBuilder::AddProperty(const char* aPropertyName, Ref<IPropertyHandleBase> aPropertyHandle) noexcept
	{
		RLS_ASSERT(!ExistsProperty(aPropertyName), "[IDetailCategoryBuilder::AddProperty]: Property already exists in category.");
		m_Nodes.push_back(RLS_NEW DetailNode(aPropertyName, std::move(aPropertyHandle)));

		return DetailPropertyRowBuilder<DataType>(aPropertyName, m_Nodes.back());
	}

}