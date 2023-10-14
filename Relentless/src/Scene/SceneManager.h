#pragma once

#include "Scene.h"
#include "../Graphics/Resources/Helper.h"
#include "../Utility/ManagerUtilities.h"
namespace Relentless
{
	class SceneManager
	{
	public:
		SceneManager() noexcept = default;
		~SceneManager() noexcept = default;
		SceneHandle LoadSceneFromFile(const std::string& fullPath, const UUID& uuid = CreateUUID()) noexcept;
		SceneHandle CreateNewScene(const std::string& filepath) noexcept;
		[[nodiscard]] Scene& GetScene(const SceneHandle& sceneHandle) noexcept;
		[[nodiscard]] Scene& GetSceneByString(const std::string& sceneName) noexcept;
		[[nodiscard]] SceneHandle& GetSceneHandleByString(const std::string& sceneName) noexcept;
		[[nodiscard]] bool Exists(const std::string& sceneName) noexcept;
		[[nodiscard]] SceneHandle PromoteToHandle(const UUID& uuid) noexcept;
	private:
		std::queue<uint16_t> m_FreeList;
		std::vector<std::shared_ptr<Scene>> m_Scenes;
		std::unordered_map<std::string, SceneHandle> m_SceneNameToHandleMap;
	};
}