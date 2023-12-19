#pragma once
#include "TextureManager.h"
namespace Relentless
{
	struct AssetHandle;

	class TextureSerializer
	{
	public:
		static void Serialize(AssetHandle& textureHandle, const std::string& path, const std::string& toReplace = "") noexcept;
		static UUID SerializeDefault(const std::string& path) noexcept;
		static AssetHandle Deserialize(const std::string& fullPath) noexcept;
	};
}