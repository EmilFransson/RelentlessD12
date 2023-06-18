#include "MeshFactory.h"
#include "../../vendor/includes/Assimp/Importer.hpp"
#include "../../vendor/includes/Assimp/scene.h"
#include "../../vendor/includes/Assimp/postprocess.h"

#include "../Graphics/Resources/AssetManager.h"
namespace Relentless
{
	std::vector<Mesh>& MeshFactory::LoadFromFile(const std::filesystem::path& filePath) noexcept
	{
		meshes.clear();

		Assimp::Importer importer;
		constexpr const uint32_t flags = (uint32_t)(aiProcess_ConvertToLeftHanded | aiProcess_GenSmoothNormals | aiProcess_Triangulate
			| aiProcess_ImproveCacheLocality | aiProcess_JoinIdenticalVertices | aiProcess_CalcTangentSpace | aiProcess_GenBoundingBoxes);
		const aiScene* pScene = importer.ReadFile(filePath.string(), flags);
		RLS_ASSERT(pScene && !(pScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) && pScene->mRootNode, importer.GetErrorString());

		ProcessNode(pScene->mRootNode, pScene);

		//At this point all the model's meshes have been loaded into the vector.
		return meshes;
	}

	void MeshFactory::ProcessNode(aiNode* pNode, const aiScene* pScene) noexcept
	{
		RLS_ASSERT(pNode && pScene, "Assimp data is invalid.");

		for (uint32_t i{ 0u }; i < pNode->mNumMeshes; ++i)
		{
			aiMesh* pMesh = pScene->mMeshes[pNode->mMeshes[i]];
			meshes.push_back(ProcessMesh(pMesh, pScene));
		}

		for (uint32_t i{ 0u }; i < pNode->mNumChildren; ++i)
		{
			ProcessNode(pNode->mChildren[i], pScene);
		}
	}

	Mesh MeshFactory::ProcessMesh(aiMesh* pMesh, const aiScene* pScene) noexcept
	{
		RLS_ASSERT(pMesh && pScene, "Assimp data is invalid.");
		RLS_ASSERT(pMesh->HasPositions(), "Mesh contains no position data.");
		RLS_ASSERT(pMesh->HasFaces(), "Mesh contains no faces data.");
		RLS_ASSERT(pMesh->HasNormals(), "Mesh contains no normal data.");
		RLS_ASSERT(pMesh->HasTextureCoords(0u), "Mesh contains no texture coordinate data.")

		Mesh mesh;
		mesh.Vertices.reserve(pMesh->mNumVertices);
		for (uint32_t i{ 0u }; i < pMesh->mNumVertices; ++i)
		{
			SimpleVertex vertex{};

			vertex.Position.x = pMesh->mVertices[i].x;
			vertex.Position.y = pMesh->mVertices[i].y;
			vertex.Position.z = pMesh->mVertices[i].z;

			vertex.Normal.x = pMesh->mNormals[i].x;
			vertex.Normal.y = pMesh->mNormals[i].y;
			vertex.Normal.z = pMesh->mNormals[i].z;

			vertex.TextureCoords.x = pMesh->mTextureCoords[0][i].x;
			vertex.TextureCoords.y = pMesh->mTextureCoords[0][i].y;

			mesh.Vertices.push_back(vertex);
		}
		for (uint32_t i{ 0u }; i < pMesh->mNumFaces; ++i)
		{
			aiFace face = pMesh->mFaces[i];
			for (uint32_t j{ 0u }; j < face.mNumIndices; ++j)
				mesh.Indices.push_back(face.mIndices[j]);
		}

		aiAABB aabb = pMesh->mAABB;

		DirectX::XMFLOAT3 nearBottomLeft = DirectX::XMFLOAT3(aabb.mMin.x, aabb.mMin.y, aabb.mMin.z);
		DirectX::XMFLOAT3 farTopRight = DirectX::XMFLOAT3(aabb.mMax.x, aabb.mMax.y, aabb.mMax.z);

		DirectX::XMFLOAT3 nearTopLeft = DirectX::XMFLOAT3(nearBottomLeft.x, aabb.mMax.y, nearBottomLeft.z);
		DirectX::XMFLOAT3 nearTopRight = DirectX::XMFLOAT3(aabb.mMax.x, nearTopLeft.y, nearBottomLeft.z);
		DirectX::XMFLOAT3 nearBottomRight = DirectX::XMFLOAT3(nearTopRight.x, nearBottomLeft.y, nearBottomLeft.z);

		DirectX::XMFLOAT3 farBottomLeft = DirectX::XMFLOAT3(nearBottomLeft.x, nearBottomLeft.y, farTopRight.z);
		DirectX::XMFLOAT3 farTopLeft = DirectX::XMFLOAT3(farBottomLeft.x, farTopRight.y, farBottomLeft.z);
		DirectX::XMFLOAT3 farBottomRight = DirectX::XMFLOAT3(farTopRight.x, farBottomLeft.y, farTopRight.z);


		return mesh;
	}
}