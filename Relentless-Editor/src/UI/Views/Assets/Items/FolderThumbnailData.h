#pragma once

#include "AssetViewItem.h"

namespace Relentless
{
	class FolderThumbnailData : public AssetViewItem
	{
	public:
		FolderThumbnailData(const String& aVirtualPath, const String& aName) noexcept;

		NO_DISCARD const String& GetName() const noexcept override;
		NO_DISCARD const String& GetVirtualPath() const noexcept;
	private:
		String m_VirtualPath;
		String m_Name;
	};
}