#include "MeshSerializer.h"
#include "../Graphics/Resources/AssetManager.h"
#include "../Utility/SerializeUtilities.h"
namespace Relentless
{
	

	void ModelSerializer::Serialize(std::vector<MeshHandle> meshHandles, const std::string& path, [[maybe_unused]] const std::string& toReplace) noexcept
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

	void ModelSerializer::SerializeBinary(MeshHandle& meshHandle, const std::string& path) noexcept
	{
		RLS_ASSERT(meshHandle != NULL_HANDLE, "Mesh handle is invalid.");

		const Mesh& mesh = AssetManager::Get<Mesh>(meshHandle);
		const std::unique_ptr<VertexBuffer>& pVertexBuffer = mesh.GetVertexBuffer();
		const std::unique_ptr<IndexBuffer>& pIndexBuffer = mesh.GetIndexBuffer();

		VertexBuffer::Specification& vertexBufferSpecification = pVertexBuffer->GetSpecification();
		IndexBuffer::Specification& indexBufferSpecification = pIndexBuffer->GetSpecification();

		MeshDataHeader meshDataHeader
		{
			.VertexBufferSizeInBytes = vertexBufferSpecification.TotalSizeInBytes,
			.IndexBufferSizeInBytes = indexBufferSpecification.TotalSizeInBytes,
			.VertexCount = vertexBufferSpecification.NrOfVertices,
			.IndexCount = indexBufferSpecification.NrOfIndices
		};

		// Open a file stream and write the data to the file
		std::string fullPath = path + mesh.GetName() + ".rmesh";
		std::ofstream outFile(fullPath, std::ios::binary);

		//Write the header:
		outFile.write(reinterpret_cast<char*>(&meshDataHeader), sizeof(meshDataHeader));

		// Write the vertex and index data
		outFile.write(static_cast<char*>(vertexBufferSpecification.pBuffer), meshDataHeader.VertexBufferSizeInBytes);
		outFile.write(static_cast<char*>(indexBufferSpecification.pBuffer), meshDataHeader.IndexBufferSizeInBytes);

		outFile.close();

		#if defined RLS_DEBUG
		std::ifstream inFile(fullPath, std::ios::binary);

		MeshDataHeader readHeader;

		inFile.read(reinterpret_cast<char*>(&readHeader), sizeof(readHeader));
		RLS_ASSERT(memcmp(&readHeader, &meshDataHeader, sizeof(readHeader)) == 0, "Data is not equal");

		std::vector<char> readVertexData(meshDataHeader.VertexBufferSizeInBytes);
		std::vector<char> readIndexData(meshDataHeader.IndexBufferSizeInBytes);

		inFile.read(reinterpret_cast<char*>(readVertexData.data()), meshDataHeader.VertexBufferSizeInBytes);
		RLS_ASSERT(memcmp(readVertexData.data(), vertexBufferSpecification.pBuffer, meshDataHeader.VertexBufferSizeInBytes) == 0, "Data is not equal");

		inFile.read(reinterpret_cast<char*>(readIndexData.data()), meshDataHeader.IndexBufferSizeInBytes);
		RLS_ASSERT(memcmp(readIndexData.data(), indexBufferSpecification.pBuffer, meshDataHeader.IndexBufferSizeInBytes) == 0, "Data is not equal");
		#endif

		//Corresponding rasset-file:
		std::string rassetPath = fullPath + ".rasset";
		if (!std::filesystem::exists(rassetPath))
		{
			YAML::Emitter out;
			out << YAML::BeginMap;

			out << YAML::Key << "Mesh" << YAML::Value << YAML::BeginMap;
			out << YAML::Key << "Name" << YAML::Value << mesh.GetName();
			out << YAML::Key << "GUID" << YAML::Value << meshHandle.UUID;

			out << YAML::EndMap;
			out << YAML::EndMap;

			std::ofstream rassetOutFile(rassetPath);

			rassetOutFile << out.c_str();
			rassetOutFile.close();

			::SetFileAttributesA(rassetPath.c_str(), GetFileAttributesA(rassetPath.c_str()) | FILE_ATTRIBUTE_HIDDEN);
		}
	}

	MeshHandle ModelSerializer::Deserialize(const std::string& path, const std::shared_ptr<Scene> pScene) noexcept
	{
		std::string rassetPath = path + ".rasset";
		RLS_ASSERT(std::filesystem::exists(rassetPath), "Path is invalid for deserialization.");
		
		YAML::Node root = YAML::LoadFile(rassetPath);
		RLS_ASSERT(root["Mesh"], "Yaml buffer does not contain a 'Mesh' node.");
		std::string name = root["Mesh"]["Name"].as<std::string>();
		UUID uuid = ConvertStringToGUID(root["Mesh"]["GUID"].as<std::string>());

		MeshHandle meshHandle = AssetManager::GetMeshManager().LoadMeshBinary(path, uuid);
		AssetManager::GetMeshManager().GetMesh(meshHandle).SetName(name);
		return meshHandle;
	}
}