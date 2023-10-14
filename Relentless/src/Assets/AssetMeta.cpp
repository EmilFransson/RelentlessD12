#include "AssetMeta.h"
#include "AssetManagerEx.h"

namespace Relentless
{
	AssetHandle_EX::AssetHandle_EX(AssetType assetType, UUID uuid, uint32_t index) noexcept
		: Type{assetType},
		  Uuid{uuid},
		  Index{index}
	{
		AssetManagerEx::IncreaseReferenceCount(*this);
	}

	AssetHandle_EX::~AssetHandle_EX() noexcept
	{
		AssetManagerEx::DecreaseReferenceCount(*this);
	}

	AssetHandle_EX::AssetHandle_EX(const AssetHandle_EX& otherHandle) noexcept
		: Type{otherHandle.Type},
		  Uuid{otherHandle.Uuid},
		  Index{otherHandle.Index }
	{
		AssetManagerEx::IncreaseReferenceCount(*this);
	}

	AssetHandle_EX& AssetHandle_EX::operator=(const AssetHandle_EX& otherHandle) noexcept
	{
		if (this == &otherHandle)
		{
			return *this;
		}
		
		AssetManagerEx::DecreaseReferenceCount(*this);
			
		Type = otherHandle.Type;
		Uuid = otherHandle.Uuid;
		Index = otherHandle.Index;
	
		AssetManagerEx::IncreaseReferenceCount(*this);

		return *this;
	}
}