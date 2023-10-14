#pragma once
#include "AssetHandleTemp.h"
namespace Relentless
{
	#define NULL_INDEX std::numeric_limits<uint16_t>::max()
	#define NULL_UUID UUID{0}

	struct AssetHandle
	{
		UUID UUID{ 0 };
		uint16_t Index{ std::numeric_limits<uint16_t>::max() };

		friend bool operator==(const AssetHandle& lhs, const AssetHandle& rhs)
		{
			return lhs.UUID == rhs.UUID && lhs.Index == rhs.Index;
		}

		friend bool operator!=(const AssetHandle& lhs, const AssetHandle& rhs)
		{
			return !(lhs == rhs);
		}
	};

	typedef AssetHandle TextureHandle;
	typedef AssetHandle MaterialHandle;
	typedef AssetHandle MeshHandle;
	typedef AssetHandle SceneHandle;

	#define NULL_HANDLE AssetHandle(UUID{0}, NULL_INDEX)

	[[nodiscard]] std::string GetAssetHandleAsString(const AssetHandle& assetHandle) noexcept;
}