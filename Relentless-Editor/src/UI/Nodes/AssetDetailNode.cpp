#include "AssetDetailNode.h"

#include "Module/ContentBrowserModule.h"
#include "Property/AssetPropertyHandle.h"

#include "Thumbnail/AssetThumbnailData.h"

namespace Relentless
{
	AssetDetailNode::AssetDetailNode(const char* aName, const AssetData& aAssetData) noexcept
		:DetailNode{aName, nullptr},
		 m_AssetData{aAssetData}
	{
		m_pThumbnailData = MakeShared<AssetThumbnailData>(m_AssetData, ModuleManager::LoadModuleChecked<ContentBrowserModule>().GetAssetThumbnailPool());
	}

	AssetDetailNode::~AssetDetailNode() noexcept = default;
	
	const SharedPtr<AssetThumbnailData>& AssetDetailNode::GetThumbnailData() const noexcept
	{
		return m_pThumbnailData;
	}

}