#pragma once
#include "Scene.h"
namespace Relentless
{
	class SceneSerializer
	{
	public:
		static void Serialize(const std::shared_ptr<Scene>& pScene, const std::string& filepath) noexcept;
		static bool Deserialize(const std::shared_ptr<Scene>& pScene, const std::string& filepath) noexcept;
	};
}