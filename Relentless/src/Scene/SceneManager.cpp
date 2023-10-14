#include "SceneManager.h"
#include "SceneSerializer.h"

namespace Relentless
{
	inline static std::mutex g_CreateMutex;


	SceneHandle SceneManager::LoadSceneFromFile(const std::string& fullPath, const UUID& uuid) noexcept
	{
		if (Exists(fullPath))
		{
			return m_SceneNameToHandleMap[fullPath];
		}

		SceneHandle sceneHandle;
		sceneHandle.UUID = uuid;

		{
			const std::lock_guard<std::mutex> lock(g_CreateMutex);

			if (!m_FreeList.empty())
			{
				sceneHandle.Index = m_FreeList.front();
				m_FreeList.pop();
				
				std::shared_ptr<Scene> pScene = nullptr;
				SceneSerializer::Deserialize(pScene, fullPath);
				m_Scenes[sceneHandle.Index] = std::move(pScene);
			}
			else
			{
				sceneHandle.Index = static_cast<uint16_t>(m_Scenes.size());
				std::shared_ptr<Scene> pScene = nullptr;
				SceneSerializer::Deserialize(pScene, fullPath);
				m_Scenes.emplace_back(std::move(pScene));
			}

			m_SceneNameToHandleMap[fullPath] = sceneHandle;
		}
		return sceneHandle;
	}

	SceneHandle SceneManager::CreateNewScene(const std::string& filepath) noexcept
	{
		const std::lock_guard<std::mutex> lock(g_CreateMutex);

		SceneHandle sceneHandle;
		sceneHandle.UUID = CreateUUID();

		if (!m_FreeList.empty())
		{
			sceneHandle.Index = m_FreeList.front();
			m_FreeList.pop();

			std::shared_ptr<Scene> pScene = std::make_shared<Scene>();
			m_Scenes[sceneHandle.Index] = std::move(pScene);
		}
		else
		{
			sceneHandle.Index = static_cast<uint16_t>(m_Scenes.size());
			std::shared_ptr<Scene> pScene = std::make_shared<Scene>();
			m_Scenes.emplace_back(std::move(pScene));
		}

		m_SceneNameToHandleMap[filepath] = sceneHandle;

		return sceneHandle;
	}

	Scene& SceneManager::GetScene(const SceneHandle& sceneHandle) noexcept
	{
		RLS_ASSERT(sceneHandle != NULL_HANDLE, "Scene handle is invalid.");
		RLS_ASSERT(m_Scenes.size() > sceneHandle.Index, "Scene does not exist");

		return *m_Scenes[sceneHandle.Index];
	}

	Scene& SceneManager::GetSceneByString(const std::string& sceneName) noexcept
	{
		RLS_ASSERT(m_SceneNameToHandleMap.contains(sceneName), "Scene '" + sceneName + "' does not exist.");
		SceneHandle& sceneHandle = m_SceneNameToHandleMap[sceneName];
		RLS_ASSERT(m_Scenes.size() > sceneHandle.Index, "Scene '" + sceneName + "' does not exist.");
		return *m_Scenes[sceneHandle.Index];
	}

	SceneHandle& SceneManager::GetSceneHandleByString(const std::string& sceneName) noexcept
	{
		const std::lock_guard<std::mutex> lock(g_CreateMutex);

		RLS_ASSERT(m_SceneNameToHandleMap.contains(sceneName), "Scene '" + sceneName + "' does not exist.");
		return m_SceneNameToHandleMap[sceneName];
	}

	bool SceneManager::Exists(const std::string& sceneName) noexcept
	{
		const std::lock_guard<std::mutex> lock(g_CreateMutex);

		return m_SceneNameToHandleMap.contains(sceneName);
	}

	SceneHandle SceneManager::PromoteToHandle(const UUID& uuid) noexcept
	{
		const std::lock_guard<std::mutex> lock(g_CreateMutex);

		for (auto& [name, handle] : m_SceneNameToHandleMap)
		{
			if (handle.UUID == uuid)
			{
				return handle;
			}
		}

		RLS_ASSERT(false, "UUID does not exist in unordered_map.");
		return {};
	}
}