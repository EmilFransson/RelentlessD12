#include "FolderThumbnailData.h"

namespace Relentless
{
	FolderThumbnailData::FolderThumbnailData(const String& aVirtualPath, const String& aName) noexcept
		: AssetViewItem{ EAssetViewItemType::Folder }
		, m_VirtualPath{ aVirtualPath }
		, m_Name{ aName }
	{
	}

	const String& FolderThumbnailData::GetName() const noexcept
	{
		return m_Name;
	}

	const String& FolderThumbnailData::GetVirtualPath() const noexcept
	{
		return m_VirtualPath;
	}
}