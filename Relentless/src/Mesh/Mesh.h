#pragma once
#include "../Graphics/Resources/VertexBuffer.h"
#include "../Graphics/Resources/IndexBuffer.h"
#include "../Graphics/Resources/Helper.h"
#include "../../vendor/includes/Assimp/scene.h"

struct aiNode;
struct aiScene;
struct aiMesh;
namespace Relentless
{
	typedef UUID MeshHandle;
	class FullMesh
	{
	public:
		explicit FullMesh(const std::string& name, const VertexBuffer::Specification& vbSpec, const IndexBuffer::Specification& ibSpec) noexcept;
		FullMesh() = default;
		~FullMesh() noexcept = default;
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
		explicit MeshManager() noexcept = default;
		~MeshManager() noexcept = default;
		[[nodiscard]] FullMesh& Get(const MeshHandle& meshHandle) noexcept;
		[[nodiscard]] FullMesh& GetByString(const std::string& meshString) noexcept;
		[[nodiscard]] MeshHandle& GetHandleByString(const std::string& meshString) noexcept;
		void LoadModelFromFile(const std::string& fullPath, Scene* pScene = nullptr) noexcept;
		[[nodiscard]] bool Exists(const std::string& path) noexcept; //TODO: Go by name probably!
	private:
	private:
		DirectX::XMMATRIX ConvertMatrix(aiMatrix4x4& inMat);
		void ProcessNode(aiNode* pNode, const aiScene* pAssimpScene, Scene* pScene, const DirectX::XMFLOAT4X4& transform, const std::filesystem::path& workingDirectory) noexcept;
		[[nodiscard]] MeshHandle ProcessMesh(aiMesh* pMesh, const aiScene* pScene) noexcept;
		[[nodiscard]] UUID ProcessMaterial(aiMesh* pMesh, const aiScene* pScene, const std::filesystem::path& workingDirectory) noexcept;
	private:
		std::unordered_map<MeshHandle, FullMesh> m_Meshes;
		std::unordered_map<std::string, MeshHandle> m_MeshNameToHandleMap;
	};
}