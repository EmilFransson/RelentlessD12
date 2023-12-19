#include "TextureManager.h"

namespace Relentless
{
	//inline static std::mutex g_CreateMutex;
	//
	//TextureHandle TextureManager::LoadTextureFromFile(const std::string& fullPath, const UUID& uuid) noexcept
	//{
	//	if (Exists(fullPath))
	//	{
	//		return m_TextureNameToHandleMap[fullPath];
	//	}
	//	
	//	TextureHandle textureHandle;
	//	textureHandle.UUID = uuid;
	//
	//	{
	//		const std::lock_guard<std::mutex> lock(g_CreateMutex);
	//
	//		if (!m_FreeList.empty())
	//		{
	//			textureHandle.Index = m_FreeList.front();
	//			m_FreeList.pop();
	//			m_2DTextures[textureHandle.Index] = std::move(*Texture2D::Create(fullPath));
	//		}
	//		else
	//		{
	//			textureHandle.Index = static_cast<uint16_t>(m_2DTextures.size());
	//			m_2DTextures.emplace_back(std::move(*Texture2D::Create(fullPath)));
	//		}
	//
	//		m_TextureNameToHandleMap[fullPath] = textureHandle;
	//	}
	//	return textureHandle;
	//}
	//
	//Texture2D& TextureManager::GetTexture(const TextureHandle& textureHandle) noexcept
	//{
	//	RLS_ASSERT(textureHandle != NULL_HANDLE, "Texture handle is invalid.");
	//	RLS_ASSERT(m_2DTextures.size() > textureHandle.Index, "Texture does not exist");
	//
	//	return m_2DTextures[textureHandle.Index];
	//}
	//
	//Texture2D& TextureManager::GetTextureByString(const std::string& textureName) noexcept
	//{
	//	RLS_ASSERT(m_TextureNameToHandleMap.contains(textureName), "Texture '" + textureName + "' does not exist.");
	//	TextureHandle& textureHandle = m_TextureNameToHandleMap[textureName];
	//	RLS_ASSERT(m_2DTextures.size() > textureHandle.Index, "Texture '" + textureName + "' does not exist.");
	//	return m_2DTextures[textureHandle.Index];
	//}
	//
	//TextureHandle& TextureManager::GetTextureHandleByString(const std::string& textureName) noexcept
	//{
	//	const std::lock_guard<std::mutex> lock(g_CreateMutex);
	//
	//	RLS_ASSERT(m_TextureNameToHandleMap.contains(textureName), "Texture '" + textureName + "' does not exist.");
	//	return m_TextureNameToHandleMap[textureName];
	//}
	//
	//bool TextureManager::Exists(const std::string& textureName) noexcept
	//{
	//	const std::lock_guard<std::mutex> lock(g_CreateMutex);
	//
	//	return m_TextureNameToHandleMap.contains(textureName);
	//}
	//
	//TextureHandle TextureManager::PromoteToHandle(const UUID& uuid) noexcept
	//{
	//	const std::lock_guard<std::mutex> lock(g_CreateMutex);
	//
	//	for (auto& [name, handle] : m_TextureNameToHandleMap)
	//	{
	//		if (handle.UUID == uuid)
	//		{
	//			return handle;
	//		}
	//	}
	//
	//	RLS_ASSERT(false, "UUID does not exist in unordered_map.");
	//	return {};
	//}
}