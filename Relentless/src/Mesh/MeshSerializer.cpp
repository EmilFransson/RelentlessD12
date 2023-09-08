#include "MeshSerializer.h"
#include "../Graphics/Resources/AssetManager.h"
#include "../Utility/SerializeUtilities.h"
namespace Relentless
{
	void ModelSerializer::Serialize(std::vector<MeshHandle> meshHandles, const std::string& path, const std::string& toReplace) noexcept
	{
		RLS_ASSERT(std::filesystem::exists(path), "Path is invalid for serialization.");

		YAML::Node root;

		std::string rassetPath = path + ".rasset";

		if (std::filesystem::exists(rassetPath)) {
			root = YAML::LoadFile(rassetPath);
		}

		for (auto& meshHandle : meshHandles)
		{
			Mesh& mesh = AssetManager::Get<Mesh>(meshHandle);
			if (root["Meshes"][mesh.GetName()])
			{
				//We do not yet change any info here.
				//For now we simply leave it alone:
				continue;
			}

			YAML::Node meshNode;
			meshNode["GUID"] =  GetAssetHandleAsString(meshHandle);

			if (!root["Meshes"])
				root["Meshes"] = YAML::Node(YAML::NodeType::Map);
			
			root["Meshes"][mesh.GetName()] = meshNode;
		}

		std::ofstream outFile(rassetPath);
		RLS_ASSERT(outFile.is_open(), "Unable to create/open file to serialize to.");
		outFile << root;
	}

	void ModelSerializer::Deserialize(const std::string& path, const std::shared_ptr<Scene> pScene) noexcept
	{
		std::string rassetPath = path + ".rasset";
		RLS_ASSERT(std::filesystem::exists(rassetPath), "Path is invalid for deserialization.");
		
		YAML::Node root = YAML::LoadFile(rassetPath);
		RLS_ASSERT(root["Meshes"], "Yaml buffer does not contain a 'Meshes' node.");

		AssetManager::GetMeshManager().LoadModelFromFile(path, pScene ? pScene.get() : nullptr, &root);
	}
}