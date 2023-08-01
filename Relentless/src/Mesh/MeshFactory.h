#pragma once
#include "Vertex.h"

struct aiNode;
struct aiScene;
struct aiMesh;
namespace Relentless
{
	struct Mesh
	{
		std::vector<SimpleVertex> Vertices;
		std::vector<uint32_t> Indices;

		std::vector<Mesh> SubMeshes;
		std::string Name;
	};
	struct Model
	{
		std::vector<Mesh> Meshes;
		uint32_t NrOfMeshes{ 0u };
		std::string Name;
	};
	class MeshFactory
	{
	public:
		explicit MeshFactory() noexcept = default;
		~MeshFactory() noexcept = default;
		[[nodiscard]] std::vector<Mesh>& LoadFromFile(const std::filesystem::path& filePath) noexcept;
		[[nodiscard]] Model LoadModelFromFile(const std::filesystem::path& filePath) noexcept;
	private:
		void ProcessNode(aiNode* pNode, const aiScene* pScene, std::vector<Mesh>& meshes) noexcept;
		[[nodiscard]] Mesh ProcessMesh(aiMesh* pMesh, const aiScene* pScene) noexcept;
	private:
		std::vector<Mesh> meshes;
		bool headDone{ false };
		uint32_t nrOfMeshes{ 0u };
	};
}