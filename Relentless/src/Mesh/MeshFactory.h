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
	};
	struct Model
	{
		std::vector<Mesh> Meshes;

	};
	class MeshFactory
	{
	public:
		explicit MeshFactory() noexcept = default;
		~MeshFactory() noexcept = default;
		[[nodiscard]] std::vector<Mesh>& LoadFromFile(const std::filesystem::path& filePath) noexcept;
	private:
		void ProcessNode(aiNode* pNode, const aiScene* pScene) noexcept;
		[[nodiscard]] Mesh ProcessMesh(aiMesh* pMesh, const aiScene* pScene) noexcept;
	private:
		std::vector<Mesh> meshes;
	};
}