#pragma once
#include "TextureManager.h"
namespace Relentless
{
	class TextureSerializer
	{
	public:
		static void Serialize(TextureHandle& textureHandle, const std::string& path, const std::string& toReplace = "") noexcept;
		static UUID SerializeDefault(const std::string& path) noexcept;
		static TextureHandle Deserialize(const std::string& fullPath) noexcept;
	};
}