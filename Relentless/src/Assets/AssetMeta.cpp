#include "AssetMeta.h"
#include "AssetManager.h"

namespace Relentless
{
	AssetHandle::AssetHandle(AssetType assetType, UUID uuid, uint32_t index) noexcept
		: Type{assetType},
		  Uuid{uuid},
		  Index{index}
	{
		AssetManager::IncreaseReferenceCount(*this);
	}

	AssetHandle::~AssetHandle() noexcept
	{
		AssetManager::DecreaseReferenceCount(*this);
	}

	AssetHandle::AssetHandle(const AssetHandle& otherHandle) noexcept
		: Type{otherHandle.Type},
		  Uuid{otherHandle.Uuid},
		  Index{otherHandle.Index }
	{
		AssetManager::IncreaseReferenceCount(*this);
	}

	AssetHandle& AssetHandle::operator=(const AssetHandle& otherHandle) noexcept
	{
		if (this == &otherHandle)
		{
			return *this;
		}
		
		AssetManager::DecreaseReferenceCount(*this);
			
		Type = otherHandle.Type;
		Uuid = otherHandle.Uuid;
		Index = otherHandle.Index;
	
		AssetManager::IncreaseReferenceCount(*this);

		return *this;
	}
}