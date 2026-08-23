#include "IDetailCategoryBuilder.h"

#include "IDetailLayoutBuilder.h"

#include <Relentless.h>

namespace Relentless
{
	IDetailCategoryBuilder::IDetailCategoryBuilder(const char* aName, IDetailLayoutBuilder& aParent) noexcept
		: m_Name{ aName }
		, m_DetailLayoutBuilder{ aParent }
	{
	}

	DetailNode& IDetailCategoryBuilder::AddProperty(const char* aPropertyName) noexcept
	{
		RLS_ASSERT(!ExistsProperty(aPropertyName), "[IDetailCategoryBuilder::AddProperty]: Property already exists in category.");

		Ref<DetailNode> pNewNode = new DetailNode(aPropertyName);
		m_Entries.push_back({ .IsGroup = false, .Node = pNewNode });
		return *(m_Entries.back().Node);
	}

	IDetailLayoutBuilder& IDetailCategoryBuilder::GetLayoutBuilder() const noexcept
	{
		return m_DetailLayoutBuilder;
	}

	const std::vector<HeaderAction>& IDetailCategoryBuilder::GetHeaderActions() const noexcept
	{
		return m_HeaderActions;
	}

	IDetailGroupBuilder IDetailCategoryBuilder::EditGroup(const char* aGroupName) noexcept
	{
		RLS_ASSERT(!ExistsProperty(aGroupName), "[IDetailCategoryBuilder::EditGroup]: Group already exists in category.");

		m_Entries.push_back({ .IsGroup = true, .Node = RLS_NEW DetailNode(aGroupName), .GroupName = aGroupName});
		return IDetailGroupBuilder(aGroupName, m_Entries.back().Node, *this);
	}

	DetailPropertyRowBuilder<AssetData> IDetailCategoryBuilder::AddAssetProperty(const char* aPropertyName, const AssetData& aAssetData) noexcept
	{
		RLS_ASSERT(!ExistsProperty(aPropertyName), "[IDetailCategoryBuilder::AddAssetProperty]: Property already exists in category.");

		m_Entries.push_back({ .IsGroup = false, .Node = RLS_NEW AssetDetailNode(aPropertyName, aAssetData) });
		return DetailPropertyRowBuilder<AssetData>(aPropertyName, m_Entries.back().Node);
	}

	bool IDetailCategoryBuilder::ExistsProperty(const char* aPropertyName) const noexcept
	{
		return std::ranges::any_of(m_Entries, [aPropertyName](const DetailEntry& aEntry) { return aEntry.Node && aEntry.Node->GetName() == aPropertyName; });
	}

	const std::vector<DetailEntry>& IDetailCategoryBuilder::GetEntries() const noexcept
	{
		return m_Entries;
	}
}