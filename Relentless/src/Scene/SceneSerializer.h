#pragma once
#include "Scene.h"
#include "SceneManager.h"
namespace Relentless
{
	class SceneSerializer
	{
	public:
		static void Serialize(const std::shared_ptr<Scene>& pScene, const std::string& filepath) noexcept;
		static bool Deserialize(const std::shared_ptr<Scene>& pScene, const std::string& filepath) noexcept;

		static void Serialize(const SceneHandle& sceneHandle, const std::string& filepath) noexcept;
		static SceneHandle Deserialize(const std::string& filepath) noexcept;
		static UUID SerializeDefault(const std::string& path) noexcept;
	};
}