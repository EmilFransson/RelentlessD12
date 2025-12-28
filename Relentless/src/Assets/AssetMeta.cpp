#include "AssetMeta.h"
#include "AssetManager.h"

namespace Relentless
{
	AssetHandle::AssetHandle(const TypeIndex& aTypeIndex, const UUID& aUUID, uint32_t aSparseIndex) noexcept
		: Type{ aTypeIndex },
		  Uuid{ aUUID },
		  Index{ aSparseIndex }
	{
	}

	AssetHandle::~AssetHandle() noexcept
	{
	}

	AssetHandle::AssetHandle(const AssetHandle& otherHandle) noexcept
		: Type{otherHandle.Type},
		  Uuid{otherHandle.Uuid},
		  Index{otherHandle.Index }
	{
	}

	AssetHandle& AssetHandle::operator=(const AssetHandle& otherHandle) noexcept
	{
		if (this == &otherHandle)
		{
			return *this;
		}
			
		Type = otherHandle.Type;
		Uuid = otherHandle.Uuid;
		Index = otherHandle.Index;

		return *this;
	}
}