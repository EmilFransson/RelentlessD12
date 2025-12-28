#include "ModelFactory.h"

#include "Assets/AssetManager.h"
#include "Core/Application.h"
#include "File/File.h"
#include "Graphics/RHI/Device.h"
#include "Mesh/Mesh.h"
#include "Mesh/Vertex.h"
#include "Utility/FilepathUtils.h"
#include "Graphics/Resources/Material.h"
#include "Graphics/Resources/Texture2D.h"

#include "../../../vendor/includes/DirectXTK/WICTextureLoader.h"
#include "../../../vendor/includes/DirectXTK/ResourceUploadBatch.h"
#include "../../../vendor/includes/directxtex/DirectXTex.h"
#include "../../../vendor/includes/meshoptimizer/meshoptimizer.h"

namespace Relentless
{

	static Matrix ConvertMatrix(aiMatrix4x4& inMat)
	{
		Matrix convertedMatrix = Matrix
		(
			inMat.a1, inMat.b1, inMat.c1, inMat.d1,
			inMat.a2, inMat.b2, inMat.c2, inMat.d2,
			inMat.a3, inMat.b3, inMat.c3, inMat.d3,
			inMat.a4, inMat.b4, inMat.c4, inMat.d4
		);

		return convertedMatrix;
	}

	static DXGI_FORMAT GetCompressedDXGITextureFormat(ETextureCompressionType compressionType, bool srgb) noexcept
	{
		DXGI_FORMAT compressedFormat{};
		switch (compressionType)
		{
		case ETextureCompressionType::BC5:
		{
			compressedFormat = DXGI_FORMAT::DXGI_FORMAT_BC5_UNORM;
			break;
		}
		case ETextureCompressionType::BC7:
		case ETextureCompressionType::BC7_Quick:
		{
			compressedFormat = srgb ? DXGI_FORMAT::DXGI_FORMAT_BC7_UNORM_SRGB : DXGI_FORMAT_BC7_UNORM;
			break;
		}
		default:
			RLS_ASSERT(false, "Unreachable.")
			return compressedFormat;
		}
	}

	static EExtensionType GetExtensionTypeFromPath(const std::filesystem::path& fullPath) noexcept
	{
		const std::string extension = FilepathUtils::ExtractExtension(fullPath);
		if (extension == ".jpg")
			return EExtensionType::JPG;
		else if (extension == ".jpeg")
			return EExtensionType::JPEG;
		else if (extension == ".png")
			return EExtensionType::PNG;
		else if (extension == ".tga")
			return EExtensionType::TGA;
		else if (extension == ".tif" || extension == ".tiff")
			return EExtensionType::TIFF;
		else if (extension == ".dds")
			return EExtensionType::DDS;
		else if (extension == ".bmp")
			return EExtensionType::BMP;
		else if (extension == ".hdr")
			return EExtensionType::HDR;
		else if (extension == ".exr")
			return EExtensionType::EXR;
		else if (extension == ".fbx")
			return EExtensionType::FBX;
		else if (extension == ".obj")
			return EExtensionType::OBJ;
		else if (extension == ".gltf")
			return EExtensionType::GLTF;
		else
			return EExtensionType::UNKNOWN;
	}

	static void LogHR(HRESULT hr, const std::string& contextualString, const std::filesystem::path& srcFilepath) noexcept
	{
		if (hr != S_OK)
		{
			const _com_error error(hr);
			RLS_CORE_ERROR("[ModelFactory]: Failed to {0} file with path '{1}'; operation failed with message '{2}'", contextualString.c_str(), srcFilepath.string().c_str(), error.ErrorMessage());
		}
	}

	ModelFactory::~ModelFactory() = default;

	void ModelFactory::SetGenerateCollisionMeshes(bool enable) noexcept
	{
		m_GenerateCollisionMeshes = enable;
	}

	void ModelFactory::SetGenerateTextureMipmaps(bool enable) noexcept
	{
		m_GenerateTextureMipmaps = enable;
	}

	void ModelFactory::SetOptimizeMeshes(bool enable) noexcept
	{
		m_OptimizeMeshes = enable;
	}

	void ModelFactory::SetImportMaterialsAndTextures(bool enable) noexcept
	{
		m_ImportMaterialsAndTextures = enable;
	}

	void ModelFactory::SetTextureCompressionType(ETextureCompressionType compressionType) noexcept
	{
		m_TextureCompressionType = compressionType;
	}

	bool ModelFactory::CanCreateNew() const noexcept
	{
		return false;
	}

	bool ModelFactory::CanImport(const Path& aPath) const noexcept
	{
		const String extension = FilepathUtils::ExtractExtension(aPath);
		return std::ranges::any_of(m_SupportedExtensions, [&](const String& aExtension) { return aExtension == extension; });
	}

	Ref<IFactory> ModelFactory::Clone() noexcept
	{
		return new ModelFactory();
	}

	bool ModelFactory::DoesSupportAsset(IAsset* aAsset) const noexcept
	{
		return dynamic_cast<Mesh*>(aAsset) != nullptr;
	}

	std::vector<String> ModelFactory::GetSupportedFileExtensions() const noexcept
	{
		return std::vector<String>(m_SupportedExtensions.begin(), m_SupportedExtensions.end());
	}

	std::vector<String> ModelFactory::GetFormats() const noexcept
	{
		return std::vector<String>(m_SupportedFormats.begin(), m_SupportedFormats.end());
	}

	const FactoryResult& ModelFactory::ImportFromFile(const Path& aPath, const Path& aPackagePath, const String& aName, Ref<FeedbackContext> aFeedbackContext /*= nullptr*/) noexcept
	{
		if (!File::Exists(aPath))
		{
			Finalize(false);
			return std::unexpected{ "File does not exist." };
		}

		m_pDevice = Application::Get().GetGraphicsDevice();
		m_MainModelPath = aPath;

		if (!InitializeImporter())
		{
			Finalize(false);
			return std::unexpected{ "Failed to initialize importer." };
		}

		if (!ParseModel())
		{
			Finalize(false);
			return std::unexpected{ "Failed to parse model." };
		}

		ImportModel();
		ResolveSceneNodeHierarchy();

		Finalize(true);

		return m_ImportedAsset;
	}

	void ModelFactory::SetGraphicsDevice(GraphicsDevice* aGraphicsDevice) noexcept
	{
		m_pDevice = aGraphicsDevice;
	}

	bool ModelFactory::SupportsFileExtension(const std::string_view aFileExtension) const noexcept
	{
		return std::ranges::any_of(m_SupportedExtensions, [&](const String& aExtension) { return aExtension == aFileExtension; });
	}

	void ModelFactory::Finalize(bool succeeded) noexcept
	{
		//SetProgress(1.0f);
		//OnDone(m_ImportedAssets, succeeded);
	}

	void ModelFactory::ImportMaterials() noexcept
	{
		auto&& GetTextureAssetHandle = [this](const Path& path) -> AssetHandle
			{
				for (const TextureImportInfo& textureImportInfo : m_UniqueTextures)
				{
					if (textureImportInfo.AbsolutePath == path)
						return textureImportInfo.HandleToImportedTexture;
				}

				return NULL_HANDLE;
			};

		for (MaterialImportInfo& materialInfo: m_UniqueMaterials)
		{
			Ref<Material> pNewMaterial = new Material();
			pNewMaterial->SetName(materialInfo.Name);
			pNewMaterial->SetBlendMode(materialInfo.Opacity < 1.0f ? EBlendMode::AlphaBlend : EBlendMode::Opaque);
			pNewMaterial->SetIsTwoSided(materialInfo.TwoSided);
			pNewMaterial->SetAlbedoColor(materialInfo.DiffuseColor);

			for (auto& textureDependency : materialInfo.TextureDependencies)
			{
				const AssetHandle textureHandle = GetTextureAssetHandle(textureDependency.AbsolutePath);
				if (textureHandle == NULL_HANDLE)
					continue;

				switch (textureDependency.Type)
				{
				case aiTextureType_BASE_COLOR:
				case aiTextureType_DIFFUSE:
					pNewMaterial->SetTexture(ETextureType::Albedo, textureHandle);
					break;
				case aiTextureType_METALNESS:
					pNewMaterial->SetTexture(ETextureType::Metallic, textureHandle);
					break;
				case aiTextureType_DIFFUSE_ROUGHNESS:
					pNewMaterial->SetTexture(ETextureType::Roughness, textureHandle);
					break;
				case aiTextureType_NORMALS:
				case aiTextureType_NORMAL_CAMERA:
					pNewMaterial->SetTexture(ETextureType::NormalMap, textureHandle);
					break;
				case aiTextureType_AMBIENT_OCCLUSION:
					pNewMaterial->SetTexture(ETextureType::AmbientOcclusion, textureHandle);
					break;
				case aiTextureType_EMISSION_COLOR:
					pNewMaterial->SetTexture(ETextureType::Emission, textureHandle);
					break;
				case aiTextureType_DISPLACEMENT:
					pNewMaterial->SetTexture(ETextureType::DisplacementMap, textureHandle);
					break;
				default:
					RLS_ASSERT(false, "[Importer]: Unknown texture type encountered.");
					break;
				}
			}

			materialInfo.HandleToImportedMaterial = AssetManager::RegisterAsset<Material>(pNewMaterial);

			FactoryResult importedAsset;
			importedAsset = materialInfo.HandleToImportedMaterial;
			
			StoreImportedAsset(std::move(importedAsset));
			IncreaseProgress();
		}
	}

	void ModelFactory::ImportModel() noexcept
	{
		if (m_ImportMaterialsAndTextures)
		{
			ImportTextures();
			ImportMaterials();
		}

		ImportMeshes();
	}

	void ModelFactory::ImportTextures() noexcept
	{
		std::vector<std::future<void>> textureImportFutures;
		textureImportFutures.reserve(m_UniqueTextures.size());

		ThreadPool& threadPool = Application::Get().GetThreadPool();

		for (size_t i = 0u; i < m_UniqueTextures.size(); ++i)
		{
			textureImportFutures.push_back(threadPool.Submit([this, i]()
				{
					TextureImportInfo& importInfo = m_UniqueTextures[i];

					Ref<Texture2D> pTexture = nullptr;
					AssetHandle assetHandle = NULL_HANDLE;
					if (ImportTexture(importInfo.AbsolutePath, importInfo.IsSRGB, pTexture, assetHandle))
					{
						importInfo.HandleToImportedTexture = assetHandle;

						FactoryResult importedAsset;
						importedAsset = assetHandle;
						StoreImportedAsset(std::move(importedAsset));
					}
					IncreaseProgress();
				}));
		}

		for (const auto& textureFuture : textureImportFutures)
			textureFuture.wait();
	}

	bool ModelFactory::ImportTexture(const Path& absolutePath, bool srgb, Ref<Texture2D>& pOutTexture, AssetHandle& outAssetHandle) noexcept
	{
		DirectX::ScratchImage image;
		HRESULT result = S_OK;
		
		const EExtensionType extensionType = GetExtensionTypeFromPath(absolutePath);
		switch (extensionType)
		{
		case EExtensionType::TGA:
			result = LoadFromTGAFile(absolutePath.c_str(), nullptr, image);
			break;
		case EExtensionType::JPG:
		case EExtensionType::JPEG:
		case EExtensionType::PNG:
		case EExtensionType::TIFF:
		{
			DirectX::WIC_FLAGS importFlags = DirectX::WIC_FLAGS::WIC_FLAGS_NONE;
			if (srgb)
				importFlags |= DirectX::WIC_FLAGS::WIC_FLAGS_FORCE_SRGB;
			else
				importFlags |= DirectX::WIC_FLAGS::WIC_FLAGS_FORCE_RGB;

			result = LoadFromWICFile(absolutePath.c_str(), importFlags, nullptr, image);
			break;
		}
		case EExtensionType::HDR:
		case EExtensionType::EXR:
		{
			result = LoadFromHDRFile(absolutePath.c_str(), nullptr, image);
			break;
		}
		case EExtensionType::DDS:
		{
			result = LoadFromDDSFile(absolutePath.c_str(), DirectX::DDS_FLAGS_NONE, nullptr, image);
			break;
		}
		default:
		{
			RLS_CORE_ERROR("[Importer]: Failed to import texture file with path '{0}'; file type is not supported.", absolutePath.string().c_str());
			return false;
		}
		}

		if (result != S_OK)
		{
			LogHR(result, "import", absolutePath);
			return false;
		}

		if (image.GetMetadata().format == DXGI_FORMAT_B8G8R8A8_UNORM && srgb)
			image.OverrideFormat(DXGI_FORMAT_B8G8R8A8_UNORM_SRGB);
		else if (image.GetMetadata().format == DXGI_FORMAT_R8G8B8A8_UNORM && srgb)
			image.OverrideFormat(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);

		if (m_GenerateTextureMipmaps)
		{
			DirectX::ScratchImage mipChain;
			const HRESULT hr = GenerateMipMaps(image.GetImages()[0], DirectX::TEX_FILTER_DEFAULT, 0u, mipChain);
			if (hr != S_OK)
				LogHR(hr, "generate mipmaps", absolutePath);
			else
				image = std::move(mipChain);
		}

		const bool shouldCompress = m_TextureCompressionType != ETextureCompressionType::Uncompressed;
		if (shouldCompress)
		{
			DirectX::TEX_COMPRESS_FLAGS compressFlags = DirectX::TEX_COMPRESS_FLAGS::TEX_COMPRESS_PARALLEL;
			if (m_TextureCompressionType == ETextureCompressionType::BC7_Quick)
				compressFlags |= DirectX::TEX_COMPRESS_BC7_QUICK;

			DirectX::ScratchImage compressedImage;
			const HRESULT hr = Compress(image.GetImages(), image.GetImageCount(), image.GetMetadata(), GetCompressedDXGITextureFormat(m_TextureCompressionType, srgb), compressFlags, DirectX::TEX_THRESHOLD_DEFAULT, compressedImage);
			if (hr != S_OK)
				LogHR(hr, "compress", absolutePath);
			else
				image = std::move(compressedImage);
		}

		auto& metaData = image.GetMetadata();
		const std::string fileName = FilepathUtils::ExtractFilename(absolutePath);

		Ref<Texture2D> pNewTexture = new Texture2D(TextureDesc::Create2D(metaData.width, metaData.height, D3D::ConvertFormat(metaData.format), metaData.mipLevels, TextureFlag::ShaderResource), std::move(image));  //m_pDevice->CreateTexture(TextureDesc::Create2D(metaData.width, metaData.height, D3D::ConvertFormat(metaData.format), metaData.mipLevels, TextureFlag::ShaderResource), fileName.c_str(), initData);
		pNewTexture->SetName(fileName);
		
		pOutTexture = pNewTexture;
		outAssetHandle = AssetManager::RegisterAsset<Texture2D>(pNewTexture);

		return true;
	}

	void ModelFactory::ImportMeshes() noexcept
	{
		std::vector<std::future<void>> meshImportFutures;
		meshImportFutures.reserve(m_UniqueMeshes.size());

		ThreadPool& threadPool = Application::Get().GetThreadPool();

		for (size_t i = 0u; i < m_UniqueMeshes.size(); ++i)
		{
			meshImportFutures.push_back(threadPool.Submit([this, i]()
				{
					MeshImportInfo& importInfo = m_UniqueMeshes[i];

					Ref<Mesh> pMesh = nullptr;
					AssetHandle assetHandle = NULL_HANDLE;
					if (ImportMesh(importInfo.pMesh, pMesh, assetHandle))
					{
						pMesh->SetDefaultMaterial(m_UniqueMaterials[importInfo.pMesh->mMaterialIndex].HandleToImportedMaterial);

						importInfo.HandleToImportedMesh = assetHandle;

						FactoryResult importedAsset;
						importedAsset = assetHandle;

						StoreImportedAsset(std::move(importedAsset));
					}

					IncreaseProgress();
				}));
		}

		for (const auto& meshFuture : meshImportFutures)
			meshFuture.wait();
	}

	bool ModelFactory::ImportMesh(const aiMesh* pMesh, Ref<Mesh>& pOutMesh, AssetHandle& outHandle) noexcept
	{
		if (!pMesh->HasPositions() || !pMesh->HasFaces() || !pMesh->HasNormals() || !pMesh->HasTangentsAndBitangents() || !pMesh->HasTextureCoords(0u))
		{
			RLS_CORE_ERROR("[Importer]: Error in importing mesh with name '{0}'; one or more vertex fields does not exist.", pMesh->mName.C_Str());
			return false;
		}

		std::vector<SimpleVertex> vertices;
		vertices.reserve(pMesh->mNumVertices);

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

		std::vector<uint32_t> indices;

		for (uint32_t i{ 0u }; i < pMesh->mNumFaces; ++i)
		{
			aiFace face = pMesh->mFaces[i];
			for (uint32_t j{ 0u }; j < face.mNumIndices; ++j)
				indices.push_back(face.mIndices[j]);
		}

		const size_t nrOfIndices = indices.size();
		const size_t nrOfVertices = vertices.size();

		//Remap table:
		std::vector<uint32_t> remap(nrOfIndices);
		const size_t optimizedVertexCount = meshopt_generateVertexRemap(remap.data(), indices.data(), nrOfIndices, vertices.data(), nrOfVertices, sizeof(SimpleVertex));

		std::vector<uint32_t> optimizedIndices;
		std::vector<SimpleVertex> optimizedVertices;
		optimizedIndices.resize(nrOfIndices);
		optimizedVertices.resize(optimizedVertexCount);

		//Optimization 1: remove duplicate vertices:
		meshopt_remapIndexBuffer(optimizedIndices.data(), indices.data(), nrOfIndices, remap.data());
		meshopt_remapVertexBuffer(optimizedVertices.data(), vertices.data(), nrOfVertices, sizeof(SimpleVertex), remap.data());

		//Optimization 2: improve vertex locality:
		meshopt_optimizeVertexCache(optimizedIndices.data(), optimizedIndices.data(), nrOfIndices, optimizedVertexCount);

		//Optimization 3: Reduce pixel overdraw:
		meshopt_optimizeOverdraw(optimizedIndices.data(), optimizedIndices.data(), nrOfIndices, &(optimizedVertices[0].Position.x), optimizedVertexCount, sizeof(SimpleVertex), 1.05f);

		//Optimization 4: optimize vertex buffer accesses:
		meshopt_optimizeVertexFetch(optimizedVertices.data(), optimizedIndices.data(), nrOfIndices, optimizedVertices.data(), optimizedVertexCount, sizeof(SimpleVertex));

		const uint32_t vertexBufferSizeInBytes = (uint32_t)optimizedVertices.size() * sizeof(SimpleVertex);
		const uint32_t indexBufferSizeInBytes = (uint32_t)optimizedIndices.size() * sizeof(uint32_t);

		const std::string sanitizedName = FilepathUtils::SanitizeFileName(pMesh->mName.C_Str());

		Ref<Buffer> pVertexBuffer = m_pDevice->CreateBuffer(BufferDesc::CreateVertexBuffer((uint32_t)optimizedVertices.size(), sizeof(SimpleVertex), BufferFlag::ShaderResource), "Vertex Buffer", optimizedVertices.data());
		Ref<Buffer> pIndexBuffer = m_pDevice->CreateBuffer(BufferDesc::CreateIndexBuffer((uint32_t)optimizedIndices.size(), ResourceFormat::R32_UINT, BufferFlag::ShaderResource), "Index Buffer", optimizedIndices.data());

		Ref<Mesh> pNewMesh = new Mesh(pVertexBuffer, pIndexBuffer, sanitizedName);

		AssetHandle handle = AssetManager::RegisterAsset<Mesh>(pNewMesh);

		RLS_CORE_INFO("Loaded mesh '{0}' with GUID: '{1}'.", pNewMesh->GetName(), ConvertUUIDToString(handle.Uuid));

		pOutMesh = pNewMesh;
		outHandle = handle;

		return true;
	}

	void ModelFactory::IncreaseProgress() noexcept
	{
		//std::lock_guard guard(m_ProgressionMutex);
		//
		//m_Progress += m_ProgressPerAsset;
		//OnProgressIncreased(m_Progress);
	}

	bool ModelFactory::InitializeImporter() noexcept
	{
		uint32_t importFlags = (uint32_t)(aiProcess_ConvertToLeftHanded | aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals);
		if (m_OptimizeMeshes)
			importFlags |= (uint32_t)(aiProcess_Triangulate | aiProcess_ImproveCacheLocality | aiProcess_JoinIdenticalVertices);
		if (m_GenerateCollisionMeshes)
			importFlags |= (uint32_t)(aiProcess_GenBoundingBoxes);

		m_pImporter = std::make_unique<Assimp::Importer>();
		m_pScene = m_pImporter->ReadFile(m_MainModelPath.string(), importFlags);

		const bool isSceneIncomplete = m_pScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE;

		if (!m_pScene || isSceneIncomplete || !m_pScene->mRootNode)
		{
			RLS_CORE_ERROR("[ModelFactory::InitializeImporter]: Reading file for model with path '{0}' failed with error message: '{1}'.", m_MainModelPath.string().c_str(), m_pImporter->GetErrorString());
			return false;
		}

		return true;
	}

	bool ModelFactory::ParseModel() noexcept
	{
		if (m_ImportMaterialsAndTextures)
			ParseMaterialsAndTextures();

		ParseMeshes();
		
		const size_t numAssetsToImport = m_UniqueMeshes.size() + m_UniqueMaterials.size() + m_UniqueTextures.size();
		if (numAssetsToImport == 0u)
			return false;

		m_ProgressPerAsset = 1.0f / static_cast<float>(numAssetsToImport);

		return true;
	}

	void ModelFactory::ParseMaterialsAndTextures() noexcept
	{
		std::set<Path> UniqueTexturePaths;
		const Path workingDirectory = m_MainModelPath.parent_path();

		for (uint32_t i{ 0u }; i < m_pScene->mNumMaterials; ++i)
		{
			MaterialImportInfo& materialInfo = m_UniqueMaterials.emplace_back();
			materialInfo.pMaterial = m_pScene->mMaterials[i];
			materialInfo.Name = FilepathUtils::SanitizeFileName(std::string(materialInfo.pMaterial->GetName().C_Str()));
			if (materialInfo.Name.empty())
				materialInfo.Name = "Unnamed";

			auto&& TryGetTexture = [this, &materialInfo, &UniqueTexturePaths, &workingDirectory](const aiMaterial* pMaterial, aiTextureType textureType, bool isSRGB) -> bool
				{
					aiString path;
					if (pMaterial->GetTexture(textureType, 0, &path) != aiReturn::aiReturn_SUCCESS)
						return false;
					
					const Path absolutePath = FilepathUtils::Combine(workingDirectory, path.C_Str());
					if (!File::Exists(absolutePath))
						return false;

					auto [_, inserted] = UniqueTexturePaths.insert(absolutePath);
					if (inserted)
					{
						TextureImportInfo& textureInfo = m_UniqueTextures.emplace_back();
						textureInfo.AbsolutePath = absolutePath;
						textureInfo.IsSRGB = isSRGB;
						textureInfo.Type = textureType;
					}

					TextureImportInfo& textureDependency = materialInfo.TextureDependencies.emplace_back();
					textureDependency.AbsolutePath = absolutePath;
					textureDependency.IsSRGB = isSRGB;
					textureDependency.Type = textureType;

					return true;
				};
		
			if (!TryGetTexture(materialInfo.pMaterial, aiTextureType_BASE_COLOR, true))
				TryGetTexture(materialInfo.pMaterial, aiTextureType_DIFFUSE, true);
			
			if (!TryGetTexture(materialInfo.pMaterial, aiTextureType_NORMALS, false))
				TryGetTexture(materialInfo.pMaterial, aiTextureType_NORMAL_CAMERA, false);

			TryGetTexture(materialInfo.pMaterial, aiTextureType_METALNESS, false);
			TryGetTexture(materialInfo.pMaterial, aiTextureType_DIFFUSE_ROUGHNESS, false);
			TryGetTexture(materialInfo.pMaterial, aiTextureType_AMBIENT_OCCLUSION, false);
			TryGetTexture(materialInfo.pMaterial, aiTextureType_EMISSION_COLOR, true);
			TryGetTexture(materialInfo.pMaterial, aiTextureType_DISPLACEMENT, false);

			aiColor4D diffuseColor{};
			if (materialInfo.pMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor) == AI_SUCCESS)
				materialInfo.DiffuseColor = Vector4(diffuseColor.r, diffuseColor.g, diffuseColor.b, diffuseColor.a);

			float opacity = 1.0f;
			if (materialInfo.pMaterial->Get(AI_MATKEY_OPACITY, opacity) == aiReturn_SUCCESS)
				materialInfo.Opacity = opacity;

			int isTwoSided = 0;
			if (materialInfo.pMaterial->Get(AI_MATKEY_TWOSIDED, isTwoSided) == aiReturn_SUCCESS)
				materialInfo.TwoSided = (isTwoSided != 0);
		}
	}

	void ModelFactory::ParseMeshes() noexcept
	{
		std::set<aiMesh*> uniqueMeshes;
		for (uint32_t i{ 0u }; i < m_pScene->mNumMeshes; ++i)
		{
			auto [_, inserted] = uniqueMeshes.insert(m_pScene->mMeshes[i]);
			if (inserted)
			{
				MeshImportInfo& importInfo = m_UniqueMeshes.emplace_back();
				importInfo.pMesh = m_pScene->mMeshes[i];
			}
		}
	}

	static void ResolveMeshHierarchy(aiNode* pNode, const aiScene* pAssimpScene, const Matrix& inTransform, std::unordered_map<const aiMesh*, Matrix>& aiMeshToImportedTransformMap) noexcept
	{
		RLS_ASSERT(pNode && pAssimpScene, "Assimp data is invalid.");

		const Matrix aiTransform = ConvertMatrix(pNode->mTransformation);
		const Matrix accumulatedTransform = aiTransform * inTransform;

		for (uint32 i{ 0u }; i < pNode->mNumMeshes; ++i)
		{
			const aiMesh* pMesh = pAssimpScene->mMeshes[pNode->mMeshes[i]];
			aiMeshToImportedTransformMap[pMesh] = accumulatedTransform;
		}

		for (uint32 i{ 0u }; i < pNode->mNumChildren; ++i)
			ResolveMeshHierarchy(pNode->mChildren[i], pAssimpScene, accumulatedTransform, aiMeshToImportedTransformMap);
	}

	void ModelFactory::ResolveSceneNodeHierarchy() noexcept
	{
		auto&& GetMeshAssetHandle = [this](const aiMesh* pMesh) -> AssetHandle
			{
				for (uint32 i = 0u; i < m_UniqueMeshes.size(); ++i)
				{
					if (m_UniqueMeshes[i].pMesh == pMesh)
						return m_UniqueMeshes[i].HandleToImportedMesh;
				}

				return NULL_HANDLE;
			};

		std::unordered_map<const aiMesh*, Matrix> aiMeshToImportedTransformMap;

		const Matrix identity = Matrix::Identity;
		ResolveMeshHierarchy(m_pScene->mRootNode, m_pScene, identity, aiMeshToImportedTransformMap);

		for (auto& [pMesh, transform] : aiMeshToImportedTransformMap)
		{
			const AssetHandle assetHandle = GetMeshAssetHandle(pMesh);
			if (assetHandle != NULL_HANDLE)
				AssetManager::Get<Mesh>(assetHandle)->SetOffsetTransform(transform);
		}
	}

	void ModelFactory::SetProgress(float progress) noexcept
	{
		//std::lock_guard guard(m_ProgressionMutex);
		//
		//m_Progress = progress;
		//OnProgressIncreased(m_Progress);
	}

	void ModelFactory::StoreImportedAsset(const FactoryResult& asset) noexcept
	{
		std::lock_guard guard(m_ImportAssetMutex);

		if (!m_MainAssetDone)
			m_ImportedAsset = asset;
		else
			m_AdditionalImportedAssets.push_back(asset);

		m_MainAssetDone = true;
	}

}