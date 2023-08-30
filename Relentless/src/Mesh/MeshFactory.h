#pragma once
#include "Vertex.h"

struct aiNode;
struct aiScene;
struct aiMesh;
namespace Relentless
{
	struct MeshStruct
	{
		std::vector<SimpleVertex> Vertices;
		std::vector<uint32_t> Indices;

		std::vector<MeshStruct> SubMeshes;
		std::string Name;
	};
	struct Model
	{
		std::vector<MeshStruct> Meshes;
		uint32_t NrOfMeshes{ 0u };
		std::string Name;
	};
	class MeshFactory
	{
	public:
		explicit MeshFactory() noexcept = default;
		~MeshFactory() noexcept = default;
		[[nodiscard]] std::vector<MeshStruct>& LoadFromFile(const std::filesystem::path& filePath) noexcept;
		[[nodiscard]] Model LoadModelFromFile(const std::filesystem::path& filePath) noexcept;
	private:
		void ProcessNode(aiNode* pNode, const aiScene* pScene, std::vector<MeshStruct>& meshes) noexcept;
		[[nodiscard]] MeshStruct ProcessMesh(aiMesh* pMesh, const aiScene* pScene) noexcept;
	private:
		std::vector<MeshStruct> meshes;
		bool headDone{ false };
		uint32_t nrOfMeshes{ 0u };
	};
}