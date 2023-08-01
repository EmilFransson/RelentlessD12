#include "SceneSerializer.h"
#pragma warning(push, 0)
#define YAML_CPP_STATIC_DEFINE 1
#include "yaml-cpp/yaml.h"
#pragma warning(pop)
namespace Relentless
{
	void SceneSerializer::Serialize(const std::shared_ptr<Scene>& pScene, const std::string& filepath) noexcept
	{
		YAML::Emitter out;
		out << YAML::BeginMap;

		out << YAML::Key << "Scene";
		out << YAML::Value << pScene->GetName();
		
		out << YAML::EndMap;

		std::ofstream outFile(filepath);
		outFile << out.c_str();
	}
}
