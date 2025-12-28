#include "Importer.h"
#include "AssetManager.h"

#include "Assets/Factory/ModelFactory.h"
#include "Assets/Factory/TextureFactory.h"

#include "Utility/Common.h"
#include "Utility/FilepathUtils.h"
#include "Graphics/Resources/Material.h"
#include "ImportSettings.h"
#include "Mesh/Mesh.h"
#include "Mesh/Vertex.h"
#include "../../../vendor/includes/DirectXTK/WICTextureLoader.h"
#include "../../../vendor/includes/DirectXTK/ResourceUploadBatch.h"
#include "../../vendor/includes/directxtex/DirectXTex.h"
#include "../../vendor/includes/Assimp/Importer.hpp"
#include "../../vendor/includes/Assimp/postprocess.h"
#include "../../vendor/includes/Assimp/scene.h"
#include "../../vendor/includes/meshoptimizer/meshoptimizer.h"

#include "Math/CommonMath.h"

namespace Relentless
{
	using namespace Microsoft::WRL;

	//Matrix ConvertMatrix(aiMatrix4x4& inMat)
	//{
	//	Matrix convertedMatrix = Matrix
	//	(
	//		inMat.a1, inMat.b1, inMat.c1, inMat.d1,
	//		inMat.a2, inMat.b2, inMat.c2, inMat.d2,
	//		inMat.a3, inMat.b3, inMat.c3, inMat.d3,
	//		inMat.a4, inMat.b4, inMat.c4, inMat.d4
	//	);
	//
	//	return convertedMatrix;
	//}
	//
	//static void ResolveMeshHierarchy(aiNode* pNode, const aiScene* pAssimpScene, const Matrix& inTransform, std::unordered_map<const aiMesh*, Transform>& aiMeshToImportedTransformMap) noexcept
	//{
	//	RLS_ASSERT(pNode && pAssimpScene, "Assimp data is invalid.");
	//
	//	const Matrix aiTransform = ConvertMatrix(pNode->mTransformation);
	//	const Matrix accumulatedTransform = aiTransform * inTransform;
	//
	//	for (uint32_t i{ 0u }; i < pNode->mNumMeshes; ++i)
	//	{
	//		const aiMesh* pMesh = pAssimpScene->mMeshes[pNode->mMeshes[i]];
	//		
	//		Transform transform;
	//		transform.Matrix = accumulatedTransform;
	//		transform.Matrix.Decompose(transform.Scale, transform.Rotation, transform.Location);
	//		
	//		aiMeshToImportedTransformMap[pMesh] = transform;
	//	}
	//
	//	for (uint32_t i{ 0u }; i < pNode->mNumChildren; ++i)
	//	{
	//		ResolveMeshHierarchy(pNode->mChildren[i], pAssimpScene, accumulatedTransform, aiMeshToImportedTransformMap);
	//	}
	//}
	//
	//std::unordered_map<AssetType, std::function<bool(GraphicsDevice* pDevice, const ImportRequest& request)>> Importer::m_LoadFuncsEx = {
	//	{AssetType::Texture, [](GraphicsDevice* pDevice, const ImportRequest& request)
	//		{
	//			if (!ImportTextureEx(pDevice, request))
	//				return false;
	//
	//			return true;
	//		}},
	//	{AssetType::Mesh, [](GraphicsDevice* pDevice, const ImportRequest& request)
	//	{
	//		return ImportModelEx(pDevice, request);
	//	}}
	//};

	//static void LogHR(HRESULT hr, const std::string& contextualString, const std::filesystem::path& srcFilepath) noexcept
	//{
	//	if (hr != S_OK)
	//	{
	//		const _com_error error(hr);
	//		RLS_CORE_ERROR("[Importer]: Failed to {0} file with path '{1}'; operation failed with message '{2}'", contextualString.c_str(), srcFilepath.string().c_str(), error.ErrorMessage());
	//	}
	//}

	//[[nodiscard]] AssetType GetAssetTypeFromExtensionType(ExtensionType extensionType) noexcept
	//{
	//	switch (extensionType)
	//	{
	//	case ExtensionType::JPG:
	//	case ExtensionType::JPEG:
	//	case ExtensionType::PNG:
	//	case ExtensionType::TGA:
	//	case ExtensionType::DDS:
	//	case ExtensionType::TIFF:
	//	case ExtensionType::HDR:
	//	case ExtensionType::EXR:
	//		return AssetType::Texture;
	//	case ExtensionType::FBX:
	//	case ExtensionType::OBJ:
	//	case ExtensionType::GLTF:
	//		return AssetType::Mesh;
	//	default:
	//		return AssetType::Undefined;
	//	}
	//}

	//std::future<void> Importer::RequestAsyncLoadFromFile(const std::filesystem::path& srcPath, const std::filesystem::path& dstAssetDirectorPath, const std::optional<AssetImportSettingsVariant>& optionalImportSettings) noexcept
	//{
	//	return Application::Get().GetThreadPool().Submit([srcPath, dstAssetDirectorPath, optionalImportSettings]()
	//		{
	//			const ExtensionType extensionType = GetExtensionTypeFromPath(srcPath);
	//			const AssetType assetType = GetAssetTypeFromExtensionType(extensionType);
	//			if (assetType == AssetType::Undefined)
	//			{
	//				RLS_CORE_ERROR("[Importer]: File with path {0} is not supported.", srcPath.string().c_str());
	//				return;
	//			}
	//
	//			//if (m_LoadFuncs[assetType](srcPath.string(), dstAssetDirectorPath, optionalImportSettings, true))
	//			//{
	//			//	RLS_CORE_INFO("[Importer]: Imported asset(s) with path {0} to directory {1}", srcPath.string().c_str(), dstAssetDirectorPath.string().c_str());
	//			//}
	//			//else
	//			//{
	//			//	RLS_CORE_ERROR("[Importer]: Failed to load asset from file with path: '{0}'.", srcPath.string().c_str());
	//			//}
	//		});
	//}

	Ref<IFactory> Importer::CreateDefaultFactory(ExtensionType extensionType) noexcept
	{
		switch (extensionType)
		{
		case ExtensionType::GLTF:
		case ExtensionType::OBJ:
			return RLS_NEW ModelFactory();
		case ExtensionType::BMP:
		case ExtensionType::DDS:
		case ExtensionType::EXR:
		case ExtensionType::HDR:
		case ExtensionType::JPEG:
		case ExtensionType::JPG:
		case ExtensionType::PNG:
		case ExtensionType::TGA:
		case ExtensionType::TIFF:
			return RLS_NEW TextureFactory();
		default:
		{
			RLS_ASSERT(false, "Unreachable.");
			return nullptr;
		}
		}
	}

	//AssetImportBatch Importer::RequestAsyncLoad(Span<AssetImportTask> importTasks, Ref<ImporterFeedbackContext> pFeedbackContext /*= nullptr*/) noexcept
	//{
	//	ThreadPool& threadPool = Application::Get().GetThreadPool();
	//	GraphicsDevice* pDevice = Application::Get().GetGraphicsDevice();
	//
	//	AssetImportBatch batch;
	//	batch.Futures.reserve(importTasks.GetSize());
	//
	//	for (const AssetImportTask& task : importTasks)
	//	{
	//		batch.Futures.push_back(threadPool.Submit([pDevice, task, pFeedbackContext]()
	//			{
	//				if (task.pFactory)
	//					task.pFactory->Execute(task.FilePath, pDevice);
	//				else
	//					CreateDefaultFactory(GetExtensionTypeFromPath(task.FilePath))->Execute(task.FilePath, pDevice);
	//			}));
	//	}
	//
	//	return batch;
	//}

	//bool Importer::ImportAssimpMesh(aiMesh* pMesh, const std::filesystem::path& srcPath, const std::filesystem::path& destinationDirectory, bool isABlockingOperation, AssetHandle& outHandle) noexcept
	//{
	//	if (!pMesh)
	//	{
	//		RLS_CORE_ERROR("[Importer]: Failed to import mesh; assimp mesh is invalid.");
	//		outHandle = NULL_HANDLE;
	//		return false;
	//	}
	//
	//	if (!pMesh->HasPositions() || !pMesh->HasFaces() || !pMesh->HasNormals() || !pMesh->HasTangentsAndBitangents() || !pMesh->HasTextureCoords(0u))
	//	{
	//		RLS_CORE_ERROR("[Importer]: Error in importing mesh with name '{0}'; one or more vertex fields does not exist.", pMesh->mName.C_Str());
	//		outHandle = NULL_HANDLE;
	//		return false;
	//	}
	//
	//	std::vector<SimpleVertex> vertices;
	//	std::vector<uint32_t> indices;
	//	for (uint32_t i{ 0u }; i < pMesh->mNumVertices; ++i)
	//	{
	//		SimpleVertex vertex{};
	//
	//		vertex.Position.x = pMesh->mVertices[i].x;
	//		vertex.Position.y = pMesh->mVertices[i].y;
	//		vertex.Position.z = pMesh->mVertices[i].z;
	//
	//		vertex.Normal.x = pMesh->mNormals[i].x;
	//		vertex.Normal.y = pMesh->mNormals[i].y;
	//		vertex.Normal.z = pMesh->mNormals[i].z;
	//
	//		vertex.Tangent.x = pMesh->mTangents[i].x;
	//		vertex.Tangent.y = pMesh->mTangents[i].y;
	//		vertex.Tangent.z = pMesh->mTangents[i].z;
	//
	//		vertex.BiTangent.x = pMesh->mBitangents[i].x;
	//		vertex.BiTangent.y = pMesh->mBitangents[i].y;
	//		vertex.BiTangent.z = pMesh->mBitangents[i].z;
	//
	//		vertex.TextureCoords.x = pMesh->mTextureCoords[0][i].x;
	//		vertex.TextureCoords.y = pMesh->mTextureCoords[0][i].y;
	//
	//		vertices.push_back(vertex);
	//	}
	//	for (uint32_t i{ 0u }; i < pMesh->mNumFaces; ++i)
	//	{
	//		aiFace face = pMesh->mFaces[i];
	//		for (uint32_t j{ 0u }; j < face.mNumIndices; ++j)
	//			indices.push_back(face.mIndices[j]);
	//	}
	//
	//	const size_t nrOfIndices = indices.size();
	//	const size_t nrOfVertices = vertices.size();
	//
	//	//Remap table:
	//	std::vector<uint32_t> remap(nrOfIndices);
	//	const size_t optimizedVertexCount = meshopt_generateVertexRemap(remap.data(), indices.data(), nrOfIndices, vertices.data(), nrOfVertices, sizeof(SimpleVertex));
	//
	//	std::vector<uint32_t> optimizedIndices;
	//	std::vector<SimpleVertex> optimizedVertices;
	//	optimizedIndices.resize(nrOfIndices);
	//	optimizedVertices.resize(optimizedVertexCount);
	//
	//	//Optimization 1: remove duplicate vertices:
	//	meshopt_remapIndexBuffer(optimizedIndices.data(), indices.data(), nrOfIndices, remap.data());
	//	meshopt_remapVertexBuffer(optimizedVertices.data(), vertices.data(), nrOfVertices, sizeof(SimpleVertex), remap.data());
	//
	//	//Optimization 2: improve vertex locality:
	//	meshopt_optimizeVertexCache(optimizedIndices.data(), optimizedIndices.data(), nrOfIndices, optimizedVertexCount);
	//
	//	//Optimization 3: Reduce pixel overdraw:
	//	meshopt_optimizeOverdraw(optimizedIndices.data(), optimizedIndices.data(), nrOfIndices, &(optimizedVertices[0].Position.x), optimizedVertexCount, sizeof(SimpleVertex), 1.05f);
	//
	//	//Optimization 4: optimize vertex buffer accesses:
	//	meshopt_optimizeVertexFetch(optimizedVertices.data(), optimizedIndices.data(), nrOfIndices, optimizedVertices.data(), optimizedVertexCount, sizeof(SimpleVertex));
	//
	//	const uint32_t vertexBufferSizeInBytes = (uint32_t)optimizedVertices.size() * sizeof(SimpleVertex);
	//	const uint32_t indexBufferSizeInBytes = (uint32_t)optimizedIndices.size() * sizeof(uint32_t);
	//
	//	const std::string sanitizedName = FilepathUtils::SanitizeFileName(pMesh->mName.C_Str());
	//
	//	//ResourceManager& resourceManager = Application::Get().GetResourceManager();
	//	//ResourceHandle vbHandle = resourceManager.CreateVertexBuffer(sanitizedName + std::string(" Vertex Buffer"), vertexBufferSizeInBytes, (uint32_t)optimizedVertices.size());
	//	//ResourceHandle ibHandle = resourceManager.CreateIndexBuffer(sanitizedName + std::string(" Index buffer"), indexBufferSizeInBytes, (uint32_t)optimizedIndices.size());
	//
	//	//std::shared_ptr<VertexBuffer> pVB = resourceManager.GetVertexBuffer(vbHandle);
	//	//std::shared_ptr<IndexBuffer> pIB = resourceManager.GetIndexBuffer(ibHandle);
	//
	//	//DirectX::ResourceUploadBatch uploadBatch(D3D12Core::GetDevice().Get());
	//	//uploadBatch.Begin();
	//
	//	{
	//		D3D12_SUBRESOURCE_DATA subresourceData = {};
	//		subresourceData.pData = optimizedVertices.data();
	//		subresourceData.RowPitch = vertexBufferSizeInBytes;
	//		subresourceData.SlicePitch = subresourceData.RowPitch;
	//		//uploadBatch.Upload(pVB->GetInterface().Get(), 0, &subresourceData, 1);
	//		//uploadBatch.Transition(pVB->GetInterface().Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	//		//pVB->SetCurrentState(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	//	}
	//
	//	{
	//		D3D12_SUBRESOURCE_DATA subresourceData = {};
	//		subresourceData.pData = optimizedIndices.data();
	//		subresourceData.RowPitch = indexBufferSizeInBytes;
	//		subresourceData.SlicePitch = subresourceData.RowPitch;
	//		//uploadBatch.Upload(pIB->GetInterface().Get(), 0, &subresourceData, 1);
	//		//uploadBatch.Transition(pIB->GetInterface().Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	//		//pIB->SetCurrentState(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	//	}
	//
	//	//auto finish = uploadBatch.End(Application::Get().GetGPUTaskManager().GetCommandQueue(CommandType::Direct).Get());
	//	//finish.wait();
	//
	//	std::shared_ptr<Mesh> pNewMesh = std::make_shared<Mesh>();
	//	pNewMesh->SetName(sanitizedName);
	//	//pNewMesh->SetVertexBufferHandle(vbHandle);
	//	//pNewMesh->SetIndexBufferHandle(ibHandle);
	//
	//	//const uint32_t index = AssetManager::GetStorage<Mesh>().Add(pNewMesh);
	//	//const auto& [handle, _] = AssetManager::InsertMetaData(CreateUUID(), index, AssetType::Mesh);
	//
	//	AssetMetaData metaData;
	//	metaData.Name = sanitizedName;
	//	metaData.SourcePath = srcPath;
	//	//metaData.Uuid = handle->second.Uuid;
	//	metaData.AssetType = AssetType::Mesh;
	//
	//	auto now = std::chrono::system_clock::now();
	//	auto duration = now.time_since_epoch();
	//	auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
	//	metaData.ModificationDateAndTime = static_cast<uint64_t>(millis);
	//
	//	const std::string filename = sanitizedName + ASSET_EXTENSION;
	//	const std::filesystem::path fullDestinationPath = FilepathUtils::Combine(destinationDirectory, filename);
	//
	//	AssetRegistry::Map(fullDestinationPath, metaData, AssetRegistry::MapOperation::Override);
	//	//AssetManager::Link(fullDestinationPath.string(), handle->second.Uuid);
	//	
	//	//if (!Serializer::Serialize(fullDestinationPath, handle->second, isABlockingOperation))
	//	//	RLS_CORE_ERROR("[Importer]: Failed to serialize imported mesh with name '{0}'.", metaData.Name.c_str());
	//
	//	//RLS_CORE_INFO("Loaded mesh '{0}' with GUID: '{1}'.", pNewMesh->GetName(), ConvertUUIDToString(handle->second.Uuid));
	//
	//	//outHandle = handle->second;
	//}

	//bool Importer::ImportAssimpMeshEx(GraphicsDevice* pDevice, aiMesh* pMesh, AssetHandle& outHandle) noexcept
	//{
	//	if (!pMesh)
	//	{
	//		RLS_CORE_ERROR("[Importer]: Failed to import mesh; assimp mesh is invalid.");
	//		outHandle = NULL_HANDLE;
	//		return false;
	//	}
	//
	//	if (!pMesh->HasPositions() || !pMesh->HasFaces() || !pMesh->HasNormals() || !pMesh->HasTangentsAndBitangents() || !pMesh->HasTextureCoords(0u))
	//	{
	//		RLS_CORE_ERROR("[Importer]: Error in importing mesh with name '{0}'; one or more vertex fields does not exist.", pMesh->mName.C_Str());
	//		outHandle = NULL_HANDLE;
	//		return false;
	//	}
	//
	//	std::vector<SimpleVertex> vertices;
	//	std::vector<uint32_t> indices;
	//	for (uint32_t i{ 0u }; i < pMesh->mNumVertices; ++i)
	//	{
	//		SimpleVertex vertex{};
	//
	//		vertex.Position.x = pMesh->mVertices[i].x;
	//		vertex.Position.y = pMesh->mVertices[i].y;
	//		vertex.Position.z = pMesh->mVertices[i].z;
	//
	//		vertex.Normal.x = pMesh->mNormals[i].x;
	//		vertex.Normal.y = pMesh->mNormals[i].y;
	//		vertex.Normal.z = pMesh->mNormals[i].z;
	//
	//		vertex.Tangent.x = pMesh->mTangents[i].x;
	//		vertex.Tangent.y = pMesh->mTangents[i].y;
	//		vertex.Tangent.z = pMesh->mTangents[i].z;
	//
	//		vertex.BiTangent.x = pMesh->mBitangents[i].x;
	//		vertex.BiTangent.y = pMesh->mBitangents[i].y;
	//		vertex.BiTangent.z = pMesh->mBitangents[i].z;
	//
	//		vertex.TextureCoords.x = pMesh->mTextureCoords[0][i].x;
	//		vertex.TextureCoords.y = pMesh->mTextureCoords[0][i].y;
	//
	//		vertices.push_back(vertex);
	//	}
	//	for (uint32_t i{ 0u }; i < pMesh->mNumFaces; ++i)
	//	{
	//		aiFace face = pMesh->mFaces[i];
	//		for (uint32_t j{ 0u }; j < face.mNumIndices; ++j)
	//			indices.push_back(face.mIndices[j]);
	//	}
	//
	//	const size_t nrOfIndices = indices.size();
	//	const size_t nrOfVertices = vertices.size();
	//
	//	//Remap table:
	//	std::vector<uint32_t> remap(nrOfIndices);
	//	const size_t optimizedVertexCount = meshopt_generateVertexRemap(remap.data(), indices.data(), nrOfIndices, vertices.data(), nrOfVertices, sizeof(SimpleVertex));
	//
	//	std::vector<uint32_t> optimizedIndices;
	//	std::vector<SimpleVertex> optimizedVertices;
	//	optimizedIndices.resize(nrOfIndices);
	//	optimizedVertices.resize(optimizedVertexCount);
	//
	//	//Optimization 1: remove duplicate vertices:
	//	meshopt_remapIndexBuffer(optimizedIndices.data(), indices.data(), nrOfIndices, remap.data());
	//	meshopt_remapVertexBuffer(optimizedVertices.data(), vertices.data(), nrOfVertices, sizeof(SimpleVertex), remap.data());
	//
	//	//Optimization 2: improve vertex locality:
	//	meshopt_optimizeVertexCache(optimizedIndices.data(), optimizedIndices.data(), nrOfIndices, optimizedVertexCount);
	//
	//	//Optimization 3: Reduce pixel overdraw:
	//	meshopt_optimizeOverdraw(optimizedIndices.data(), optimizedIndices.data(), nrOfIndices, &(optimizedVertices[0].Position.x), optimizedVertexCount, sizeof(SimpleVertex), 1.05f);
	//
	//	//Optimization 4: optimize vertex buffer accesses:
	//	meshopt_optimizeVertexFetch(optimizedVertices.data(), optimizedIndices.data(), nrOfIndices, optimizedVertices.data(), optimizedVertexCount, sizeof(SimpleVertex));
	//
	//	const uint32_t vertexBufferSizeInBytes = (uint32_t)optimizedVertices.size() * sizeof(SimpleVertex);
	//	const uint32_t indexBufferSizeInBytes = (uint32_t)optimizedIndices.size() * sizeof(uint32_t);
	//
	//	const std::string sanitizedName = FilepathUtils::SanitizeFileName(pMesh->mName.C_Str());
	//
	//	Ref<Buffer> pVertexBuffer = pDevice->CreateBuffer(BufferDesc::CreateVertexBuffer((uint32_t)optimizedVertices.size(), sizeof(SimpleVertex), BufferFlag::ShaderResource), "Vertex Buffer", optimizedVertices.data());
	//	Ref<Buffer> pIndexBuffer = pDevice->CreateBuffer(BufferDesc::CreateIndexBuffer((uint32_t)optimizedIndices.size(), ResourceFormat::R32_UINT, BufferFlag::ShaderResource), "Index Buffer", optimizedIndices.data());
	//
	//	Ref<Mesh> pNewMesh = new Mesh(pVertexBuffer, pIndexBuffer, sanitizedName);
	//
	//	const uint32_t sparseIndex = AssetManager::GetStorage<Mesh>().Add(pNewMesh);
	//	const auto& [handle, _] = AssetManager::InsertMetaData(CreateUUID(), sparseIndex, AssetType::Mesh);
	//
	//	RLS_CORE_INFO("Loaded mesh '{0}' with GUID: '{1}'.", pNewMesh->GetName(), ConvertUUIDToString(handle->second.Uuid));
	//
	//	outHandle = handle->second;
	//}

	//bool Importer::ImportModel(const std::filesystem::path& srcPath, const std::filesystem::path& destinationDirectory, bool isABlockingOperation, const MeshImportSettings& importSettings) noexcept
	//{
	//	RLS_ASSERT(false, "TODO!");
	//
	//	uint32_t importFlags = (uint32_t)(aiProcess_ConvertToLeftHanded | aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals);
	//	if (importSettings.OptimizeMesh)
	//		importFlags |= (uint32_t)(aiProcess_Triangulate | aiProcess_ImproveCacheLocality | aiProcess_JoinIdenticalVertices);
	//	if (importSettings.GenerateColliders)
	//		importFlags |= (uint32_t)(aiProcess_GenBoundingBoxes);
	//
	//	Assimp::Importer importer;
	//	const aiScene* pAssimpScene = importer.ReadFile(srcPath.string(), importFlags);
	//	const bool incompleteScene = pAssimpScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE;
	//
	//	if (!pAssimpScene || incompleteScene || !pAssimpScene->mRootNode)
	//	{
	//		RLS_CORE_ERROR("[Importer]: Reading file for model with path '{0}' failed with error message: '{1}'.", srcPath.string().c_str(), importer.GetErrorString());
	//		return false;
	//	}
	//
	//	std::unordered_map<UUID, std::pair<AssetHandle, aiMesh*>> uuidToAIMeshMap;
	//
	//	std::set<aiMesh*> uniqueMeshes;
	//	for (uint32_t i{ 0u }; i < pAssimpScene->mNumMeshes; ++i)
	//		uniqueMeshes.insert(pAssimpScene->mMeshes[i]);
	//
	//	std::vector<std::future<bool>> meshFutures;
	//	std::mutex mtx;
	//	for (aiMesh* mesh : uniqueMeshes)
	//	{
	//		meshFutures.push_back(Application::Get().GetThreadPool().Submit([mesh, srcPath, destinationDirectory, isABlockingOperation, &mtx, pAssimpScene, &uuidToAIMeshMap]()
	//			{
	//				const std::string fullyQualifiedMeshAssetFilename = FilepathUtils::SanitizeFileName(std::string(mesh->mName.C_Str())) + ".rasset";
	//				const std::filesystem::path fullDestinationPath = FilepathUtils::Combine(destinationDirectory, fullyQualifiedMeshAssetFilename);
	//				if (AssetManager::IsLoaded(fullDestinationPath.string()))
	//					return false;
	//		
	//				AssetHandle assetHandle = NULL_HANDLE;
	//				if (!ImportAssimpMesh(mesh, srcPath, destinationDirectory, isABlockingOperation, assetHandle))
	//					return false;
	//
	//				std::lock_guard<std::mutex> guard(mtx);
	//				uuidToAIMeshMap[assetHandle.Uuid] = { assetHandle,mesh };
	//			}));
	//	}
	//
	//	const std::filesystem::path workingDirectory = srcPath.parent_path();
	//
	//	struct MaterialImportTextures
	//	{
	//		std::filesystem::path Path;
	//		aiTextureType AiTextureType;
	//	};
	//
	//	struct MaterialImportMetaData
	//	{
	//		std::vector<MaterialImportTextures> MaterialTextures;
	//		bool UsesDiffuseColor = false;
	//		aiColor4D DiffuseColor{};
	//	};
	//
	//	std::unordered_map<std::string, MaterialImportMetaData> materialToTextureDependencies;
	//
	//	std::vector<std::future<void>> textureFutures;
	//	if (importSettings.ImportMaterialsAndTextures)
	//	{
	//		std::set<std::filesystem::path> uniqueTextures;
	//		std::unordered_map<std::filesystem::path, bool> pathToSRGBBoolMap;
	//
	//		for (uint32_t i{ 0u }; i < pAssimpScene->mNumMaterials; ++i)
	//		{
	//			auto&& TryGetTexture = [&uniqueTextures, &pathToSRGBBoolMap, &workingDirectory, &destinationDirectory, &materialToTextureDependencies](const aiMaterial* pMaterial, const std::string name, aiTextureType textureType, bool asSRGB) -> bool
	//				{
	//					aiString path;
	//					if (pMaterial->GetTexture(textureType, 0, &path) == aiReturn::aiReturn_SUCCESS)
	//					{
	//						const std::filesystem::path abolutePath = FilepathUtils::Combine(workingDirectory, path.C_Str());
	//						auto [_, inserted] = uniqueTextures.insert(abolutePath);
	//						if (inserted)
	//							pathToSRGBBoolMap[abolutePath] = asSRGB;
	//
	//						std::string filename = FilepathUtils::ExtractFilenameWithoutExtension(path.C_Str()) + ASSET_EXTENSION;
	//						filename = FilepathUtils::SanitizeFileName(filename);
	//						const std::filesystem::path fullDestinationPath = FilepathUtils::Combine(destinationDirectory, filename);
	//						materialToTextureDependencies[pMaterial->GetName().C_Str()].MaterialTextures.push_back({ fullDestinationPath, textureType });
	//
	//						return true;
	//					}
	//					return false;
	//				};
	//			const aiMaterial* pMaterial = pAssimpScene->mMaterials[i];
	//			std::string name = FilepathUtils::SanitizeFileName(std::string(pMaterial->GetName().C_Str()));
	//			if (name.empty())
	//				name = "Unnamed";
	//			
	//			if (!TryGetTexture(pMaterial, name, aiTextureType_BASE_COLOR, true))
	//			{
	//				TryGetTexture(pMaterial, name, aiTextureType_DIFFUSE, true);
	//			}
	//			TryGetTexture(pMaterial, name, aiTextureType_METALNESS, false);
	//			TryGetTexture(pMaterial, name, aiTextureType_DIFFUSE_ROUGHNESS, false);
	//			if (!TryGetTexture(pMaterial, name, aiTextureType_NORMALS, false))
	//			{
	//				TryGetTexture(pMaterial, name, aiTextureType_NORMAL_CAMERA, false);
	//			}
	//			TryGetTexture(pMaterial, name, aiTextureType_AMBIENT_OCCLUSION, false);
	//			TryGetTexture(pMaterial, name, aiTextureType_EMISSION_COLOR, true);
	//			TryGetTexture(pMaterial, name, aiTextureType_DISPLACEMENT, false);
	//
	//			aiColor4D diffuseColor{};
	//			if (pMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor) == AI_SUCCESS)
	//			{
	//				materialToTextureDependencies[name].DiffuseColor = diffuseColor;
	//				materialToTextureDependencies[name].UsesDiffuseColor = true;
	//			}
	//		}
	//
	//		for (auto& path : uniqueTextures)
	//		{
	//			TextureImportSettings importSettings;
	//			importSettings.GenerateMipMaps = true;
	//			importSettings.IsHDR = false;
	//			importSettings.IsSRGB = pathToSRGBBoolMap[path.string()];
	//			importSettings.TextureCompressionType = importSettings.TextureCompressionType;
	//		
	//			textureFutures.push_back(RequestAsyncLoadFromFile(path, destinationDirectory, importSettings));
	//		}
	//	}
	//	
	//	for (auto& future : meshFutures)
	//		future.wait();
	//
	//	for (auto& future : textureFutures)
	//		future.wait();
	//
	//	for (auto& [materialName, materialMeta] : materialToTextureDependencies)
	//	{
	//		//const AssetHandle handle = AssetManager::CreateNew<Material>();
	//		//Ref<Material> material = AssetManager::Get<Material>(handle);
	//		//material->SetName(materialName);
	//
	//		//if (materialMeta.UsesDiffuseColor)
	//		//	material->m_AlbedoColor = { materialMeta.DiffuseColor.r, materialMeta.DiffuseColor.g, materialMeta.DiffuseColor.b, materialMeta.DiffuseColor.a };
	//
	//		for (auto& materialMeta : materialMeta.MaterialTextures)
	//		{
	//			const AssetHandle textureHandle = AssetManager::GetHandleByPath(materialMeta.Path);
	//			switch (materialMeta.AiTextureType)
	//			{
	//			case aiTextureType_BASE_COLOR:
	//			case aiTextureType_DIFFUSE:
	//				//material->SetAlbedoTexture(textureHandle);
	//				break;
	//			case aiTextureType_METALNESS:
	//				//material->SetMetallicTexture(textureHandle);
	//				break;
	//			case aiTextureType_DIFFUSE_ROUGHNESS:
	//				//material->SetRoughnessTexture(textureHandle);
	//				break;
	//			case aiTextureType_NORMALS:
	//			case aiTextureType_NORMAL_CAMERA:
	//				//material->SetNormalMap(textureHandle);
	//				break;
	//			case aiTextureType_AMBIENT_OCCLUSION:
	//				//material->SetAmbientOcclusionTexture(textureHandle);
	//				break;
	//			case aiTextureType_EMISSION_COLOR:
	//				//material->SetEmissionTexture(textureHandle);
	//				break;
	//			case aiTextureType_DISPLACEMENT:
	//				//material->SetHeightMap(textureHandle);
	//				break;
	//			default:
	//				RLS_ASSERT(false, "[Importer]: Unknown texture type encountered.");
	//				break;
	//			}
	//		}
	//
	//		const std::string filename = FilepathUtils::SanitizeFileName(materialName) + ASSET_EXTENSION;
	//		const std::filesystem::path fullDestinationPath = FilepathUtils::Combine(destinationDirectory, filename);
	//		
	//		AssetMetaData metaData;
	//		metaData.Name = materialName;
	//		metaData.SourcePath = srcPath;
	//		//metaData.Uuid = handle.Uuid;
	//		metaData.AssetType = AssetType::Material;
	//
	//		auto now = std::chrono::system_clock::now();
	//		auto duration = now.time_since_epoch();
	//		auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
	//		metaData.ModificationDateAndTime = static_cast<uint64_t>(millis);
	//
	//		AssetRegistry::Map(fullDestinationPath, metaData, AssetRegistry::MapOperation::Override);
	//		//AssetManager::Link(fullDestinationPath.string(), handle.Uuid);
	//		
	//		//if (!Serializer::Serialize(fullDestinationPath, handle, isABlockingOperation))
	//		//	RLS_CORE_ERROR("[Importer]: Failed to serialize imported material with name '{0}'.", materialName.c_str());
	//
	//		//Application::Get().GetMemorymanager().SetDirtyMaterial(handle);
	//		//RLS_CORE_INFO("Loaded material '{0}' with GUID: '{1}'.", materialName.c_str(), ConvertUUIDToString(handle.Uuid));
	//	}
	//
	//	std::unordered_map<const aiMesh*, Transform> aiMeshToImportedTransformMap;
	//
	//	DirectX::XMFLOAT4X4 identity;
	//	DirectX::XMStoreFloat4x4(&identity, DirectX::XMMatrixIdentity());
	//	ResolveMeshHierarchy(pAssimpScene->mRootNode, pAssimpScene, identity, aiMeshToImportedTransformMap);
	//
	//	for (auto& [uuid, aiMeshAndHandlePair] : uuidToAIMeshMap)
	//	{
	//		auto& [handle, mesh] = aiMeshAndHandlePair;
	//		AssetManager::Get<Mesh>(handle)->SetOffsetTransform(aiMeshToImportedTransformMap[mesh]);
	//	}
	//
	//	return true;
	//}

	//bool Importer::ImportModelEx(GraphicsDevice* pDevice, const ImportRequest& request) noexcept
	//{
	//	MeshImportSettings importSettings{};
	//
	//	if (request.ImportSettings.has_value())
	//	{
	//		const auto& variant = *request.ImportSettings;
	//
	//		if (const MeshImportSettings* pImportSettings = std::get_if<MeshImportSettings>(&variant))
	//			importSettings = *pImportSettings;
	//	}
	//
	//	uint32_t importFlags = (uint32_t)(aiProcess_ConvertToLeftHanded | aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals);
	//	if (importSettings.OptimizeMesh)
	//		importFlags |= (uint32_t)(aiProcess_Triangulate | aiProcess_ImproveCacheLocality | aiProcess_JoinIdenticalVertices);
	//	if (importSettings.GenerateColliders)
	//		importFlags |= (uint32_t)(aiProcess_GenBoundingBoxes);
	//
	//	Assimp::Importer importer;
	//	const aiScene* pAssimpScene = importer.ReadFile(request.Filepath.string(), importFlags);
	//	const bool incompleteScene = pAssimpScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE;
	//
	//	if (!pAssimpScene || incompleteScene || !pAssimpScene->mRootNode)
	//	{
	//		RLS_CORE_ERROR("[Importer]: Reading file for model with path '{0}' failed with error message: '{1}'.", request.Filepath.string().c_str(), importer.GetErrorString());
	//		return false;
	//	}
	//	
	//	struct MaterialImportTexture
	//{
	//	aiTextureType AiTextureType;
	//	String AbsolutePath;
	//	};
	//
	//	struct MaterialImportMetaData
	//{
	//	std::vector<MaterialImportTexture> MaterialTextures;
	//	bool UsesDiffuseColor = false;
	//	aiColor4D DiffuseColor{};
	//	uint32 Index = std::numeric_limits<uint32>::max();
	//
	//	bool IsTwoSided = false;
	//	EBlendMode BlendMode = EBlendMode::Opaque;
	//};
	//
	//	std::unordered_map<std::string, MaterialImportMetaData> materialToTextureDependencies;
	//	std::unordered_map<String, AssetHandle> texturePathToAssetHandle;
	//
	//	std::vector<std::future<void>> textureFutures;
	//
	//	const std::filesystem::path workingDirectory = request.Filepath.parent_path();
	//	if (importSettings.ImportMaterialsAndTextures)
	//	{
	//		std::set<std::filesystem::path> uniqueTextures;
	//		std::unordered_map<std::filesystem::path, bool> pathToSRGBBoolMap;
	//	
	//		for (uint32_t i{ 0u }; i < pAssimpScene->mNumMaterials; ++i)
	//		{
	//			auto&& TryGetTexture = [&uniqueTextures, &pathToSRGBBoolMap, &workingDirectory, &materialToTextureDependencies](const aiMaterial* pMaterial, const std::string name, aiTextureType textureType, bool asSRGB) -> bool
	//				{
	//					aiString path;
	//					if (pMaterial->GetTexture(textureType, 0, &path) == aiReturn::aiReturn_SUCCESS)
	//					{
	//						const std::filesystem::path abolutePath = FilepathUtils::Combine(workingDirectory, path.C_Str());
	//						auto [_, inserted] = uniqueTextures.insert(abolutePath);
	//						if (inserted)
	//							pathToSRGBBoolMap[abolutePath] = asSRGB;
	//	
	//						std::string filename = FilepathUtils::ExtractFilenameWithoutExtension(path.C_Str()) + ASSET_EXTENSION;
	//						filename = FilepathUtils::SanitizeFileName(filename);
	//						materialToTextureDependencies[pMaterial->GetName().C_Str()].MaterialTextures.push_back({ textureType, abolutePath.string() });
	//	
	//						return true;
	//					}
	//					return false;
	//				};
	//			const aiMaterial* pMaterial = pAssimpScene->mMaterials[i];
	//			std::string name = FilepathUtils::SanitizeFileName(std::string(pMaterial->GetName().C_Str()));
	//			if (name.empty())
	//				name = "Unnamed";
	//	
	//			if (!TryGetTexture(pMaterial, name, aiTextureType_BASE_COLOR, true))
	//			{
	//				TryGetTexture(pMaterial, name, aiTextureType_DIFFUSE, true);
	//			}
	//			TryGetTexture(pMaterial, name, aiTextureType_METALNESS, false);
	//			TryGetTexture(pMaterial, name, aiTextureType_DIFFUSE_ROUGHNESS, false);
	//			if (!TryGetTexture(pMaterial, name, aiTextureType_NORMALS, false))
	//			{
	//				TryGetTexture(pMaterial, name, aiTextureType_NORMAL_CAMERA, false);
	//			}
	//			TryGetTexture(pMaterial, name, aiTextureType_AMBIENT_OCCLUSION, false);
	//			TryGetTexture(pMaterial, name, aiTextureType_EMISSION_COLOR, true);
	//			TryGetTexture(pMaterial, name, aiTextureType_DISPLACEMENT, false);
	//	
	//			aiColor4D diffuseColor{};
	//			if (pMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor) == AI_SUCCESS)
	//			{
	//				materialToTextureDependencies[name].DiffuseColor = diffuseColor;
	//				materialToTextureDependencies[name].UsesDiffuseColor = true;
	//			}
	//
	//			float opacity = 1.0f; 
	//			if (pMaterial->Get(AI_MATKEY_OPACITY, opacity) == aiReturn_SUCCESS && opacity < 1.0f)
	//			{
	//				materialToTextureDependencies[name].BlendMode = EBlendMode::AlphaBlend;
	//			}
	//
	//			int isTwoSided = 0;
	//			if (pMaterial->Get(AI_MATKEY_TWOSIDED, isTwoSided) == aiReturn_SUCCESS)
	//			{
	//				materialToTextureDependencies[name].IsTwoSided = (isTwoSided != 0);
	//			}
	//
	//			materialToTextureDependencies[name].Index = i;
	//		}
	//	
	//		std::mutex textureImportMutex;
	//		for (auto& path : uniqueTextures)
			//{
			//	TextureImportSettings importSettings;
			//	importSettings.GenerateMipMaps = true;
			//	importSettings.IsHDR = false;
			//	importSettings.IsSRGB = pathToSRGBBoolMap[path.string()];
			//	importSettings.TextureCompressionType = importSettings.TextureCompressionType;
			//
			//	textureFutures.push_back(Application::Get().GetThreadPool().Submit([path, importSettings, pDevice, &textureImportMutex, &texturePathToAssetHandle]()
			//		{
			//			ImportRequest request;
			//			request.Filepath = path;
			//			request.ImportSettings = importSettings;
			//
			//			request.OnAssetImported.Connect([&](const AssetHandle& handle, bool success)
			//				{
			//					if (success)
			//					{
			//						std::lock_guard guard(textureImportMutex);
			//						texturePathToAssetHandle[path.string()] = handle;
			//					}
			//					else
			//					{
			//						RLS_CORE_ERROR("FAILED TO IMPORT TEXTURE");
			//					}
			//				});
			//
			//			ImportTextureEx(pDevice, request);
			//		}));
			//}
	//	}
	//
	//	std::unordered_map<uint32, AssetHandle> materialIndexToHandleMap;
	//
	//	for (auto& future : textureFutures)
	//		future.wait();
	//
	//	for (auto& [name, materialMeta] : materialToTextureDependencies)
		//{
		//	Ref<Material> pNewMaterial = new Material();
		//	pNewMaterial->SetName(name);
		//	pNewMaterial->SetBlendMode(materialMeta.BlendMode);
		//	pNewMaterial->SetIsTwoSided(materialMeta.IsTwoSided);
		//
		//	if (materialMeta.UsesDiffuseColor)
		//		pNewMaterial->m_AlbedoColor = DirectX::XMFLOAT4(materialMeta.DiffuseColor.r, materialMeta.DiffuseColor.g, materialMeta.DiffuseColor.b, materialMeta.DiffuseColor.a);
		//
		//	for (auto& materialMeta : materialMeta.MaterialTextures)
		//	{
		//		AssetHandle textureHandle = texturePathToAssetHandle[materialMeta.AbsolutePath];
		//		switch (materialMeta.AiTextureType)
		//		{
		//		case aiTextureType_BASE_COLOR:
		//		case aiTextureType_DIFFUSE:
		//			pNewMaterial->SetAlbedoTexture(textureHandle);
		//			break;
		//		case aiTextureType_METALNESS:
		//			pNewMaterial->SetMetallicTexture(textureHandle);
		//			break;
		//		case aiTextureType_DIFFUSE_ROUGHNESS:
		//			pNewMaterial->SetRoughnessTexture(textureHandle);
		//			break;
		//		case aiTextureType_NORMALS:
		//		case aiTextureType_NORMAL_CAMERA:
		//			pNewMaterial->SetNormalMap(textureHandle);
		//			break;
		//		case aiTextureType_AMBIENT_OCCLUSION:
		//			pNewMaterial->SetAmbientOcclusionTexture(textureHandle);
		//			break;
		//		case aiTextureType_EMISSION_COLOR:
		//			pNewMaterial->SetEmissionTexture(textureHandle);
		//			break;
		//		case aiTextureType_DISPLACEMENT:
		//			pNewMaterial->SetHeightMap(textureHandle);
		//			break;
		//		default:
		//			RLS_ASSERT(false, "[Importer]: Unknown texture type encountered.");
		//			break;
		//		}
		//	}
		//
		//	const uint32 index = AssetManager::GetStorage<Material>().Add(pNewMaterial);
		//	auto [handle, _] = AssetManager::InsertMetaData(CreateUUID(), index, AssetType::Material);
		//	materialIndexToHandleMap[materialMeta.Index] = handle->second;
		//}
	//
	//	std::unordered_map<UUID, std::pair<AssetHandle, aiMesh*>> uuidToAIMeshMap;
	//
	//	std::set<aiMesh*> uniqueMeshes;
	//	for (uint32_t i{ 0u }; i < pAssimpScene->mNumMeshes; ++i)
	//		uniqueMeshes.insert(pAssimpScene->mMeshes[i]);
	//
	//	std::vector<std::future<bool>> meshFutures;
	//	meshFutures.reserve(uniqueMeshes.size());
	//
	//	std::mutex mtx;
	//	for (aiMesh* mesh : uniqueMeshes)
		//{
		//	meshFutures.push_back(Application::Get().GetThreadPool().Submit([mesh, &mtx, &uuidToAIMeshMap, pDevice, &materialIndexToHandleMap]()
		//		{
		//			AssetHandle assetHandle = NULL_HANDLE;
		//			if (!ImportAssimpMeshEx(pDevice, mesh, assetHandle))
		//				return false;
		//
		//			std::lock_guard<std::mutex> guard(mtx);
		//			uuidToAIMeshMap[assetHandle.Uuid] = { assetHandle, mesh };
		//
		//			AssetHandle handle = NULL_HANDLE;
		//			auto it = materialIndexToHandleMap.find(mesh->mMaterialIndex);
		//			if (it != materialIndexToHandleMap.end())
		//				handle = it->second;
		//
		//			AssetManager::Get<Mesh>(assetHandle)->SetDefaultMaterial(handle);
		//		}));
		//}
	//
	//	for (auto& future : meshFutures)
	//		future.wait();
	//
	//	std::unordered_map<const aiMesh*, Transform> aiMeshToImportedTransformMap;
	//
	//	DirectX::XMFLOAT4X4 identity;
	//	DirectX::XMStoreFloat4x4(&identity, DirectX::XMMatrixIdentity());
	//	ResolveMeshHierarchy(pAssimpScene->mRootNode, pAssimpScene, identity, aiMeshToImportedTransformMap);
	//
	//	for (auto& [uuid, aiMeshAndHandlePair] : uuidToAIMeshMap)
	//	{
	//		const auto& [handle, mesh] = aiMeshAndHandlePair;
	//		AssetManager::Get<Mesh>(handle)->SetOffsetTransform(aiMeshToImportedTransformMap[mesh]);
	//		
	//		request.OnAssetImported(handle, true);
	//	}
	//
	//	return true;
	//}

	//[[nodiscard]] bool Importer::ImportTextureEx(GraphicsDevice* pDevice, const ImportRequest& request) noexcept
	//{
	//	if (!File::Exists(request.Filepath))
	//	{
	//		RLS_CORE_ERROR("[Importer]: Failed to import texture file with path '{0}'; file does not exist.", request.Filepath.string().c_str());
	//		return false;
	//	}
	//
	//	const TextureImportSettings& importSettings = std::get<TextureImportSettings>(*request.ImportSettings);
	//
	//	const ExtensionType extensionType = GetExtensionTypeFromPath(request.Filepath);
	//	DirectX::ScratchImage image;
	//	HRESULT result = S_OK;
	//	switch (extensionType)
	//	{
	//	case ExtensionType::TGA:
	//		result = LoadFromTGAFile(request.Filepath.c_str(), nullptr, image);
	//		break;
	//	case ExtensionType::JPG:
	//	case ExtensionType::JPEG:
	//	case ExtensionType::PNG:
	//	case ExtensionType::TIFF:
	//	{
	//		DirectX::WIC_FLAGS importFlags = DirectX::WIC_FLAGS::WIC_FLAGS_NONE;
	//		importSettings.IsSRGB ? importFlags |= DirectX::WIC_FLAGS::WIC_FLAGS_FORCE_SRGB : importFlags |= DirectX::WIC_FLAGS::WIC_FLAGS_FORCE_RGB;
	//		result = LoadFromWICFile(request.Filepath.c_str(), importFlags, nullptr, image);
	//		break;
	//	}
	//	case ExtensionType::HDR:
	//	case ExtensionType::EXR:
	//	{
	//		result = LoadFromHDRFile(request.Filepath.c_str(), nullptr, image);
	//		break;
	//	}
	//	case ExtensionType::DDS:
	//	{
	//		result = LoadFromDDSFile(request.Filepath.c_str(), DirectX::DDS_FLAGS_NONE, nullptr, image);
	//		break;
	//	}
	//
	//	default:
	//	{
	//		RLS_CORE_ERROR("[Importer]: Failed to import texture file with path '{0}'; file type is not supported.", request.Filepath.string().c_str());
	//		return false;
	//	}
	//	}
	//
	//	if (result != S_OK)
	//	{
	//		LogHR(result, "import", request.Filepath);
	//		return false;
	//	}
	//	if (image.GetMetadata().format == DXGI_FORMAT_B8G8R8A8_UNORM && importSettings.IsSRGB)
	//		image.OverrideFormat(DXGI_FORMAT_B8G8R8A8_UNORM_SRGB);
	//	else if (image.GetMetadata().format == DXGI_FORMAT_R8G8B8A8_UNORM && importSettings.IsSRGB)
	//		image.OverrideFormat(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
	//
	//	if (importSettings.GenerateMipMaps)
	//	{
	//		DirectX::ScratchImage mipChain;
	//		const HRESULT hr = GenerateMipMaps(image.GetImages()[0], DirectX::TEX_FILTER_DEFAULT, 0u, mipChain);
	//		if (hr != S_OK)
	//			LogHR(hr, "generate mipmaps", request.Filepath);
	//		else
	//			image = std::move(mipChain);
	//	}
	//
	//	const bool shouldCompress = importSettings.TextureCompressionType != ETextureCompressionType::Uncompressed;
	//	if (shouldCompress)
	//	{
	//		DirectX::TEX_COMPRESS_FLAGS compressFlags = DirectX::TEX_COMPRESS_FLAGS::TEX_COMPRESS_PARALLEL;
	//		if (importSettings.TextureCompressionType == ETextureCompressionType::BC7_Quick)
	//			compressFlags |= DirectX::TEX_COMPRESS_BC7_QUICK;
	//
	//		DirectX::ScratchImage compressedImage;
	//		const HRESULT hr = Compress(image.GetImages(), image.GetImageCount(), image.GetMetadata(), GetCompressedDXGITextureFormat(importSettings), compressFlags, DirectX::TEX_THRESHOLD_DEFAULT, compressedImage);
	//		if (hr != S_OK)
	//			LogHR(hr, "compress", request.Filepath);
	//		else
	//			image = std::move(compressedImage);
	//	}
	//
	//	auto& metaData = image.GetMetadata();
	//
	//	//TextureDesc textureDesc;
	//	//textureDesc.Width = metaData.width;
	//	//textureDesc.Height = metaData.height;
	//	//textureDesc.SampleCount = 1;
	//	//textureDesc.Mips = metaData.mipLevels;
	//	//textureDesc.Type = TextureType::Texture2D;
	//	//textureDesc.Format = D3D::ConvertFormat(metaData.format);
	//
	//	const std::string fileName = FilepathUtils::ExtractFilename(request.Filepath);
	//
	//	//DirectX::ResourceUploadBatch uploadBatch(pDevice->GetDevice());
	//	//uploadBatch.Begin();
	//
	//	const DirectX::Image* pImg = image.GetImages();
	//	std::vector<D3D12_SUBRESOURCE_DATA> initData;
	//	for (uint32_t i{ 0u }; i < image.GetImageCount(); ++i, ++pImg)
	//	{
	//		D3D12_SUBRESOURCE_DATA subresourceData = {};
	//		subresourceData.pData = pImg->pixels;
	//		subresourceData.RowPitch = pImg->rowPitch;
	//		subresourceData.SlicePitch = pImg->slicePitch;
	//
	//		initData.push_back(subresourceData);
	//		//uploadBatch.Upload(pTexture->GetResource(), static_cast<UINT>(i), &subresourceData, 1);
	//	}
	//
	//	Ref<Texture> pNewTexture = pDevice->CreateTexture(TextureDesc::Create2D(metaData.width, metaData.height, D3D::ConvertFormat(metaData.format), metaData.mipLevels, TextureFlag::ShaderResource), fileName.c_str(), initData);
	//	const uint32 index = AssetManager::GetStorage<Texture>().Add(pNewTexture);
	//	auto [handle, _] = AssetManager::InsertMetaData(CreateUUID(), index, AssetType::Texture);
	//	request.OnAssetImported(handle->second, true);
	//
	//	//uploadBatch.Transition(pTexture->GetResource(), pTexture->GetResourceState(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	//	//auto finish = uploadBatch.End(pDevice->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT)->GetCommandQueue());
	//	//finish.wait();
	//	//pTexture->SetResourceState(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	//
	//	//AssetMetaData assetMetaData;
	//	//assetMetaData.Name = pTexture->GetName();
	//	//assetMetaData.SourcePath = fullPath;
	//	//assetMetaData.Uuid = handle.Uuid;
	//	//assetMetaData.AssetType = AssetType::TextureEx;
	//
	//	auto now = std::chrono::system_clock::now();
	//	auto duration = now.time_since_epoch();
	//	auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
	//	//assetMetaData.ModificationDateAndTime = static_cast<uint64_t>(millis);
	//
	//	//const std::filesystem::path fullDestinationPath = FilepathUtils::Combine(dstAssetDirectorPath, fileName);
	//
	//	//AssetRegistry::Map(fullDestinationPath, assetMetaData, AssetRegistry::MapOperation::Override);
	//	//AssetManager::Link(fullDestinationPath.string(), handle.Uuid);
	//
	//	//if (!Serializer::Serialize(fullDestinationPath, handle, isABlockingOperation))
	//	//	RLS_CORE_ERROR("[Importer]: Failed to serialize imported texture with name '{0}'.", assetMetaData.Name.c_str());
	//
	//	return true;
	//}

	ExtensionType Importer::GetExtensionTypeFromPath(const std::filesystem::path& fullPath) noexcept
	{
		const std::string extension = FilepathUtils::ExtractExtension(fullPath);
		if (extension == ".jpg")
			return ExtensionType::JPG;
		else if (extension == ".jpeg")
			return ExtensionType::JPEG;
		else if (extension == ".png")
			return ExtensionType::PNG;
		else if (extension == ".tga")
			return ExtensionType::TGA;
		else if (extension == ".tif" || extension == ".tiff")
			return ExtensionType::TIFF;
		else if (extension == ".dds")
			return ExtensionType::DDS;
		else if (extension == ".bmp")
			return ExtensionType::BMP;
		else if (extension == ".hdr")
			return ExtensionType::HDR;
		else if (extension == ".exr")
			return ExtensionType::EXR;
		else if (extension == ".fbx")
			return ExtensionType::FBX;
		else if (extension == ".obj")
			return ExtensionType::OBJ;
		else if (extension == ".gltf")
			return ExtensionType::GLTF;
		else
			return ExtensionType::UNKNOWN;
	}

	//DXGI_FORMAT Importer::GetCompressedDXGITextureFormat(const TextureImportSettings& importSettings) noexcept
	//{
	//	DXGI_FORMAT compressedFormat{};
	//	switch (importSettings.TextureCompressionType)
	//	{
	//	case ETextureCompressionType::BC5:
	//	{
	//		compressedFormat = DXGI_FORMAT::DXGI_FORMAT_BC5_UNORM;
	//		break;
	//	}
	//	case ETextureCompressionType::BC7:
	//	case ETextureCompressionType::BC7_Quick:
	//	{
	//		compressedFormat = importSettings.IsSRGB ? DXGI_FORMAT::DXGI_FORMAT_BC7_UNORM_SRGB : DXGI_FORMAT_BC7_UNORM;
	//		break;
	//	}
	//	}
	//	return compressedFormat;
	//}

	//General praxis is that the filename has both types in it.
	//static bool IsCombinedRoughnessAndMetalnessMap(std::string_view path) noexcept
	//{
	//	if ((path.find("Roughness") != std::string::npos || path.find("roughness") != std::string::npos)
	//		&& (path.find("Metalness") != std::string::npos || path.find("metalness") != std::string::npos))
	//	{
	//		return true;
	//	}
	//	return false;
	//}
}