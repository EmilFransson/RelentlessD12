#include "IDetailGroupBuilder.h"

#include "IDetailCategoryBuilder.h"

namespace Relentless
{
	IDetailGroupBuilder::IDetailGroupBuilder(const char* aName, Ref<DetailNode> aNode, IDetailCategoryBuilder& aParent) noexcept
		:m_Name{ aName }, 
		 m_pGroupNode{ aNode },
		 m_DetailCategoryBuilder{ aParent }
	{
	}

	DetailPropertyRowBuilder<AssetData> IDetailGroupBuilder::AddAssetProperty(const char* aPropertyName, const AssetData& aAssetData) noexcept
	{
		//RLS_ASSERT(!ExistsProperty(aPropertyName), "[IDetailCategoryBuilder::AddAssetProperty]: Property already exists in category.");

		m_pGroupNode->AddChild(RLS_NEW AssetDetailNode(aPropertyName, aAssetData));
		return DetailPropertyRowBuilder<AssetData>(aPropertyName, m_pGroupNode->GetChildren().back());
	}

	IDetailCategoryBuilder& IDetailGroupBuilder::GetCategoryBuilder() const noexcept
	{
		return m_DetailCategoryBuilder;
	}

}
