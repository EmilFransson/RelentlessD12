#include "Mesh.h"
#include "Vertex.h"
#include "../../vendor/includes/Assimp/Importer.hpp"

#include "../../vendor/includes/Assimp/postprocess.h"
#include "../Scene/Scene.h"
#include "../Graphics/Resources/AssetManager.h"

namespace Relentless
{
	Mesh::Mesh(const std::string& name, const VertexBuffer::Specification& vbSpec, const IndexBuffer::Specification& ibSpec) noexcept
		:m_Name{ name }
	{
		m_pVertexBuffer = std::make_unique<VertexBuffer>(&vbSpec);
		m_pIndexBuffer = std::make_unique<IndexBuffer>(&ibSpec);
	}

	void Mesh::SetName(const std::string& name) noexcept
	{
		m_Name = name;
	}

	Mesh& MeshManager::GetMesh(const MeshHandle& meshHandle) noexcept
	{
		RLS_ASSERT(m_Meshes.size() > meshHandle.Index, "Mesh handle is invalid.");
		return m_Meshes[meshHandle.Index];
	}

	Mesh& MeshManager::GetByString(const std::string& meshString) noexcept
	{
		MeshHandle& meshHandle = GetHandleByString(meshString);
		RLS_ASSERT(m_Meshes.size() > meshHandle.Index, "Mesh \"" + meshString + "\" does not exist.");
		return m_Meshes[meshHandle.Index];
	}

	MeshHandle& MeshManager::GetHandleByString(const std::string& meshString) noexcept
	{
		RLS_ASSERT(m_MeshNameToHandleMap.contains(meshString), "Mesh \"" + meshString + "\" does not exist.");
		return m_MeshNameToHandleMap[meshString];
	}

	void MeshManager::LoadModelFromFile(const std::string& fullPath, Scene* pScene) noexcept
	{
		RLS_ASSERT(std::filesystem::exists(fullPath), "File does not exist.");
		std::filesystem::path workingDirectory = fullPath;
		workingDirectory = workingDirectory.remove_filename();

		Assimp::Importer importer;
		constexpr const uint32_t flags = (uint32_t)(aiProcess_ConvertToLeftHanded | aiProcess_GenSmoothNormals | aiProcess_Triangulate
			| aiProcess_ImproveCacheLocality | aiProcess_JoinIdenticalVertices | aiProcess_CalcTangentSpace | aiProcess_GenBoundingBoxes);
		const aiScene* pAssimpScene = importer.ReadFile(fullPath, flags);
		RLS_ASSERT(pAssimpScene && !(pAssimpScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) && pAssimpScene->mRootNode, importer.GetErrorString());

		DirectX::XMFLOAT4X4 identity;
		DirectX::XMStoreFloat4x4(&identity, DirectX::XMMatrixIdentity());

		ProcessNode(pAssimpScene->mRootNode, pAssimpScene, pScene, identity, workingDirectory);
	}

	bool MeshManager::Exists(const std::string& path) noexcept
	{
		return (m_MeshNameToHandleMap.find(path) != m_MeshNameToHandleMap.end());
	}

	//General praxis is that the filename has both types in it.
	static bool IsCombinedRoughnessAndMetalnessMap(std::string_view path) noexcept 
	{
		if ((path.find("Roughness") != std::string::npos || path.find("roughness") != std::string::npos)
			&& (path.find("Metalness") != std::string::npos || path.find("metalness") != std::string::npos))
		{
			return true;
		}
		return false;
	}

	DirectX::XMMATRIX MeshManager::ConvertMatrix(aiMatrix4x4& inMat) {
		DirectX::XMFLOAT4X4 t = DirectX::XMFLOAT4X4(
			inMat.a1, inMat.b1, inMat.c1, inMat.d1,
			inMat.a2, inMat.b2, inMat.c2, inMat.d2,
			inMat.a3, inMat.b3, inMat.c3, inMat.d3,
			inMat.a4, inMat.b4, inMat.c4, inMat.d4);

		return DirectX::XMLoadFloat4x4(&t);
	}

	inline static std::mutex g_CreateEntityMutex;
	void MeshManager::ProcessNode(aiNode* pNode, const aiScene* pAssimpScene, Scene* pScene, const DirectX::XMFLOAT4X4& transform, const std::filesystem::path& workingDirectory) noexcept
	{
		//First time the root node should be made, which would otherwise be the only node!
		RLS_ASSERT(pNode && pAssimpScene, "Assimp data is invalid.");

		DirectX::XMMATRIX aiTransform = ConvertMatrix(pNode->mTransformation);
		DirectX::XMMATRIX accumulatedTransform = DirectX::XMMatrixMultiply(aiTransform, DirectX::XMLoadFloat4x4(&transform));
		DirectX::XMFLOAT4X4 currentTransform;
		DirectX::XMStoreFloat4x4(&currentTransform, accumulatedTransform);

		for (uint32_t i{ 0u }; i < pNode->mNumMeshes; ++i)
		{
			aiMesh* pMesh = pAssimpScene->mMeshes[pNode->mMeshes[i]];
			MeshHandle meshHandle = ProcessMesh(pMesh, pAssimpScene);
			MaterialHandle materialHandle = ProcessMaterial(pMesh, pAssimpScene, workingDirectory);
			
			if (pScene)
			{
				const std::lock_guard<std::mutex> lock(g_CreateEntityMutex);

				auto e = pScene->CreateEntity(pMesh->mName.C_Str());

				pScene->GetEntityManager().AddOrReplace<OpaquePassComponent>(e);
				pScene->GetEntityManager().Get<DirtyTransformComponent>(e).AdjustedWorldSpace = false;
				auto& tc = pScene->GetEntityManager().Get<TransformComponent>(e);
				tc.Transform = currentTransform;
				ImGuizmo::DecomposeMatrixToComponents(*tc.Transform.m, &tc.Translation.x, &tc.Rotation.x, &tc.Scale.x);

				auto& mrc = pScene->GetEntityManager().Add<MeshRendererComponent>(e);
				mrc.MaterialHandle = materialHandle;
				auto& mfc = pScene->GetEntityManager().Add<MeshFilterComponent>(e);
				mfc.MeshHandle = meshHandle;

			}
		}

		for (uint32_t i{ 0u }; i < pNode->mNumChildren; ++i)
		{
			ProcessNode(pNode->mChildren[i], pAssimpScene, pScene, currentTransform, workingDirectory);
		}
	}

	inline static std::mutex g_LoadMutex2;
	MeshHandle MeshManager::ProcessMesh(aiMesh* pMesh, const aiScene* pAssimpScene) noexcept
	{
		RLS_ASSERT(pMesh && pAssimpScene, "Assimp data is invalid.");
		RLS_ASSERT(pMesh->HasPositions(), "Mesh contains no position data.");
		RLS_ASSERT(pMesh->HasFaces(), "Mesh contains no faces data.");
		RLS_ASSERT(pMesh->HasNormals(), "Mesh contains no normal data.");
		RLS_ASSERT(pMesh->HasTangentsAndBitangents(), "Mesh contains no tangent and/or bitangent data.");
		RLS_ASSERT(pMesh->HasTextureCoords(0u), "Mesh contains no texture coordinate data.");

		const std::lock_guard<std::mutex> lock(g_LoadMutex2);

		if (Exists(pMesh->mName.C_Str()))
		{
			return m_MeshNameToHandleMap[pMesh->mName.C_Str()];
		}

		std::vector<SimpleVertex> vertices;
		std::vector<uint32_t> indices;
		for (uint32_t i{ 0u }; i < pMesh->mNumVertices; ++i)
		{
			SimpleVertex vertex{};

			vertex.Position.x = pMesh->mVertices[i].x;
			vertex.Position.y = pMesh->mVertices[i].y;
			vertex.Position.z = pMesh->mVertices[i].z;

			vertex.Normal.x = pMesh->mNormals[i].x;
			vertex.Normal.y = pMesh->mNormals[i].y;
			vertex.Normal.z = pMesh->mNormals[i].z;

			vertex.Tangent.x = pMesh->mTangents[i].x;
			vertex.Tangent.y = pMesh->mTangents[i].y;
			vertex.Tangent.z = pMesh->mTangents[i].z;

			vertex.BiTangent.x = pMesh->mBitangents[i].x;
			vertex.BiTangent.y = pMesh->mBitangents[i].y;
			vertex.BiTangent.z = pMesh->mBitangents[i].z;

			vertex.TextureCoords.x = pMesh->mTextureCoords[0][i].x;
			vertex.TextureCoords.y = pMesh->mTextureCoords[0][i].y;

			vertices.push_back(vertex);
		}
		for (uint32_t i{ 0u }; i < pMesh->mNumFaces; ++i)
		{
			aiFace face = pMesh->mFaces[i];
			for (uint32_t j{ 0u }; j < face.mNumIndices; ++j)
				indices.push_back(face.mIndices[j]);
		}

		VertexBuffer::Specification vbSpec
		{
			.NrOfVertices = (uint32_t)vertices.size(),
			.TotalSizeInBytes = (uint32_t)vertices.size() * sizeof(SimpleVertex),
			.Stride = sizeof(SimpleVertex),
			.pBuffer = (void*)vertices.data(),
			.Name = pMesh->mName.C_Str() + std::string(" Vertex Buffer")
		};

		IndexBuffer::Specification ibSpec
		{
			.NrOfIndices = (uint32_t)indices.size(),
			.TotalSizeInBytes = (uint32_t)indices.size() * sizeof(uint32_t),
			.Stride = sizeof(uint32_t),
			.pBuffer = (void*)indices.data(),
			.Name = pMesh->mName.C_Str() + std::string(" Index Buffer")
		};

		MeshHandle meshHandle;
		meshHandle.UUID = CreateUUID();

		if (!m_FreeList.empty())
		{
			meshHandle.Index = m_FreeList.front();
			m_FreeList.pop();
			m_Meshes[meshHandle.Index] = Mesh(pMesh->mName.C_Str(), vbSpec, ibSpec);
		}
		else
		{
			meshHandle.Index = m_Meshes.size();
			m_Meshes.emplace_back(Mesh(pMesh->mName.C_Str(), vbSpec, ibSpec));
		}

		m_MeshNameToHandleMap[std::string(pMesh->mName.C_Str())] = meshHandle;

		return meshHandle;
	}

	MaterialHandle MeshManager::ProcessMaterial(aiMesh* pMesh, const aiScene* pAssimpScene, const std::filesystem::path& workingDirectory) noexcept
	{
		aiMaterial* m = pAssimpScene->mMaterials[pMesh->mMaterialIndex];
		MaterialHandle materialHandle = AssetManager::Create<Material>(m->GetName().C_Str());
		auto& materialManager = AssetManager::GetMaterialManager();
		Material& mat = materialManager.GetMaterial(materialHandle);

		aiString path;
		if (m->GetTexture(aiTextureType_BASE_COLOR, 0, &path) == aiReturn::aiReturn_SUCCESS)
		{
			std::filesystem::path fullPath = workingDirectory / std::string(path.C_Str());
			AssetHandle baseColorID = AssetManager::Load<Texture2D>(fullPath.string());
			mat.SetAlbedoTexture(baseColorID);

			int usesAlpha;
			if (aiReturn::aiReturn_SUCCESS == m->Get(AI_MATKEY_TEXFLAGS(aiTextureType_BASE_COLOR, 0), usesAlpha)) {
				if (usesAlpha & aiTextureFlags_UseAlpha) {
					RLS_ASSERT(false, "HALT");
				}
			}

		}
		else if (m->GetTexture(aiTextureType_DIFFUSE, 0, &path) == aiReturn::aiReturn_SUCCESS)
		{
			std::filesystem::path fullPath = workingDirectory / std::string(path.C_Str());
			AssetHandle baseColorID = AssetManager::Load<Texture2D>(fullPath.string());
			mat.SetAlbedoTexture(baseColorID);
		}
		else 
		{
			aiColor3D baseColor(0.f, 0.f, 0.f);
			if (m->Get(AI_MATKEY_COLOR_DIFFUSE, baseColor) == AI_SUCCESS) 
			{
				mat.m_AlbedoColor = { baseColor.r, baseColor.g , baseColor.b };
			}
		}
		if (m->GetTexture(aiTextureType_METALNESS, 0, &path) == aiReturn::aiReturn_SUCCESS)
		{
			std::filesystem::path fullPath = workingDirectory / std::string(path.C_Str());
			if (IsCombinedRoughnessAndMetalnessMap(fullPath.string()))
			{
				//This is a combined map.
				mat.m_CombinedRoughnessMetallnesMap = true;
				//std::string s = std::string(fullPath.string());
				//s = s.substr(0, s.find("Roughness")) + "Metalness" + fullPath.extension().string();
				//fullPath = s;
			}

			AssetHandle metallicID = AssetManager::Load<Texture2D>(fullPath.string());
			mat.SetMetallicTexture(metallicID);
		}

		if (!mat.m_CombinedRoughnessMetallnesMap)
		{
			if (m->GetTexture(aiTextureType::aiTextureType_DIFFUSE_ROUGHNESS, 0, &path) == aiReturn::aiReturn_SUCCESS)
			{
				std::filesystem::path fullPath = workingDirectory / std::string(path.C_Str());
				AssetHandle roughnessID = AssetManager::Load<Texture2D>(fullPath.string());
				mat.SetRoughnessTexture(roughnessID);
			}
		}
		
		if (m->GetTexture(aiTextureType::aiTextureType_NORMALS, 0, &path) == aiReturn::aiReturn_SUCCESS)
		{
			std::filesystem::path fullPath = workingDirectory / std::string(path.C_Str());
			AssetHandle normalID = AssetManager::Load<Texture2D>(fullPath.string());
			mat.SetNormalMap(normalID);
		}
		else if (m->GetTexture(aiTextureType::aiTextureType_NORMAL_CAMERA, 0, &path) == aiReturn::aiReturn_SUCCESS)
		{
			std::filesystem::path fullPath = workingDirectory / std::string(path.C_Str());
			AssetHandle normalID = AssetManager::Load<Texture2D>(fullPath.string());
			mat.SetNormalMap(normalID);
		}
		if (m->GetTexture(aiTextureType::aiTextureType_AMBIENT_OCCLUSION, 0, &path) == aiReturn::aiReturn_SUCCESS)
		{
			std::filesystem::path fullPath = workingDirectory / std::string(path.C_Str());
			AssetHandle aoID = AssetManager::Load<Texture2D>(fullPath.string());
			mat.SetAmbientOcclusionTexture(aoID);
		}
		if (m->GetTexture(aiTextureType::aiTextureType_EMISSION_COLOR, 0, &path) == aiReturn::aiReturn_SUCCESS)
		{
			std::filesystem::path fullPath = workingDirectory / std::string(path.C_Str());
			AssetHandle emissionID = AssetManager::Load<Texture2D>(fullPath.string());
			mat.SetEmissionTexture(emissionID);
		}
		else
		{
			aiColor3D emissionColor(0.f, 0.f, 0.f);
			if (m->Get(AI_MATKEY_COLOR_EMISSIVE, emissionColor) == AI_SUCCESS) 
			{
				mat.m_EmissionColor = { emissionColor.r, emissionColor.g, emissionColor.b };
				mat.m_EmissionIntensity = 1.0f;
			}
		}
		if (m->GetTexture(aiTextureType::aiTextureType_HEIGHT, 0, &path) == aiReturn::aiReturn_SUCCESS)
		{
			std::filesystem::path fullPath = workingDirectory / std::string(path.C_Str());
			AssetHandle heightID = AssetManager::Load<Texture2D>(fullPath.string());
			mat.SetHeightMap(heightID);
		}
		return materialHandle;
	}
	
}