#pragma once
#include "../Graphics/Resources/VertexBuffer.h"
#include "../Graphics/Resources/IndexBuffer.h"
#include "../Graphics/Resources/Helper.h"
#include "../../vendor/includes/Assimp/scene.h"
#include "../Utility/ManagerUtilities.h"

struct aiNode;
struct aiScene;
struct aiMesh;
namespace Relentless
{
	class Mesh
	{
	public:
		explicit Mesh(const std::string& name, const VertexBuffer::Specification& vbSpec, const IndexBuffer::Specification& ibSpec) noexcept;
		void SetName(const std::string& name) noexcept;
		[[nodiscard]] const std::string& GetName() const noexcept { return m_Name; }
		[[nodiscard]] const std::unique_ptr<VertexBuffer>& GetVertexBuffer() const noexcept { return m_pVertexBuffer; }
		[[nodiscard]] const std::unique_ptr<IndexBuffer>& GetIndexBuffer() const noexcept { return m_pIndexBuffer; }
	private:
		std::string m_Name;
		std::unique_ptr<VertexBuffer> m_pVertexBuffer;
		std::unique_ptr<IndexBuffer> m_pIndexBuffer;
	};

	class Scene;
	struct MeshRendererComponent;
	class MeshManager
	{
	public:
		MeshManager() noexcept = default;
		~MeshManager() noexcept = default;
		[[nodiscard]] Mesh& GetMesh(const MeshHandle& meshHandle) noexcept;
		[[nodiscard]] Mesh& GetByString(const std::string& meshString) noexcept;
		[[nodiscard]] MeshHandle& GetHandleByString(const std::string& meshString) noexcept;
		void LoadModelFromFile(const std::string& fullPath, Scene* pScene = nullptr) noexcept;
		[[nodiscard]] bool Exists(const std::string& path) noexcept; //TODO: Go by name probably!
	private:
		DirectX::XMMATRIX ConvertMatrix(aiMatrix4x4& inMat);
		void ProcessNode(aiNode* pNode, const aiScene* pAssimpScene, Scene* pScene, const DirectX::XMFLOAT4X4& transform, const std::filesystem::path& workingDirectory) noexcept;
		[[nodiscard]] MeshHandle ProcessMesh(aiMesh* pMesh, const aiScene* pScene) noexcept;
		[[nodiscard]] MaterialHandle ProcessMaterial(aiMesh* pMesh, const aiScene* pScene, const std::filesystem::path& workingDirectory) noexcept;
	private:
		std::queue<uint16_t> m_FreeList;
		std::vector<Mesh> m_Meshes;
		std::unordered_map<std::string, MeshHandle> m_MeshNameToHandleMap;
	};
}