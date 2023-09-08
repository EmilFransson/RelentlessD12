#pragma once
#include "Texture.h"
#include "Helper.h"
#include "../../Utility/ManagerUtilities.h"
namespace Relentless
{
	class TextureManager
	{
	public:
		TextureManager() noexcept = default;
		~TextureManager() noexcept = default;
		TextureHandle LoadTextureFromFile(const std::string& fullPath, const UUID& uuid = CreateUUID()) noexcept;
		[[nodiscard]] Texture2D& GetTexture(const TextureHandle& textureHandle) noexcept;
		[[nodiscard]] Texture2D& GetTextureByString(const std::string& textureName) noexcept;
		[[nodiscard]] TextureHandle& GetTextureHandleByString(const std::string& textureName) noexcept;
		[[nodiscard]] bool Exists(const std::string& textureName) noexcept;
		[[nodiscard]] TextureHandle PromoteToHandle(const UUID& uuid) noexcept;
	private:
		std::queue<uint16_t> m_FreeList;
		std::vector<Texture2D> m_2DTextures;
		std::unordered_map<std::string, TextureHandle> m_TextureNameToHandleMap;
	};
}