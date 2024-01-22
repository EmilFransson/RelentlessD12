#include "Importer.h"
#include "AssetManager.h"
#include "Utility/Common.h"
#include "Graphics/Renderer/RenderCommand.h"
#include "Graphics/Resources/Texture.h"
#include "Graphics/Resources/Material.h"
#include "Graphics/MemoryManager.h"
#include "ImportSettings.h"
#include "Mesh/Mesh.h"
#include "Mesh/Vertex.h"
#include "../../../vendor/includes/DirectXTK/WICTextureLoader.h"
#include "../../../vendor/includes/DirectXTK/ResourceUploadBatch.h"
#include "../../vendor/includes/directxtex/DirectXTex.h"
#include "../../vendor/includes/Assimp/Importer.hpp"
#include "../../vendor/includes/Assimp/postprocess.h"
#include "../../vendor/includes/Assimp/scene.h"

namespace Relentless
{
	template<>
	Texture2D Importer::Import<Texture2D>(const std::filesystem::path& fullPath, const AssetTypeInfo<Texture2D>::Settings& importSettings) noexcept
	{
		using namespace DirectX;
		const std::string extension = fullPath.extension().string();

		RLS_ASSERT(std::filesystem::exists(fullPath), "[Importer]: Path is invalid.");
		RLS_ASSERT(HasImageExtension(extension), "[Importer]: File is not of supported image format.");

		ScratchImage image;
		if (extension == ".tga")
		{
			LoadFromTGAFile(fullPath.c_str(), nullptr, image);
		}
		else
		{
			WIC_FLAGS importFlags = WIC_FLAGS::WIC_FLAGS_NONE;
			importSettings.IsSRGB ? importFlags |= WIC_FLAGS::WIC_FLAGS_FORCE_SRGB : importFlags |= WIC_FLAGS::WIC_FLAGS_FORCE_RGB;
			HRESULT hr = LoadFromWICFile(fullPath.c_str(), importFlags, nullptr, image);
			RLS_ASSERT(SUCCEEDED(hr), "Failed to load image.");
		}
		
		bool shouldCompress = importSettings.TextureCompressionType != ETextureCompressionType::Uncompressed;
		const DXGI_FORMAT textureFormat = shouldCompress ? GetCompressedDXGITextureFormat(importSettings) : image.GetMetadata().format;
		Microsoft::WRL::ComPtr<ID3D12Resource> pTextureResource{ nullptr };

		if (importSettings.GenerateMipMaps)
		{
			DirectX::ScratchImage mipChain;
			HRESULT hr = GenerateMipMaps(image.GetImages()[0], TEX_FILTER_DEFAULT, 0u, mipChain);
			RLS_ASSERT(SUCCEEDED(hr), "Failed to generate mipmaps.");

			if (shouldCompress)
			{
				TEX_COMPRESS_FLAGS compressFlags = TEX_COMPRESS_FLAGS::TEX_COMPRESS_PARALLEL;
				if (importSettings.TextureCompressionType == ETextureCompressionType::BC7_Quick)
				{
					compressFlags |= TEX_COMPRESS_BC7_QUICK;
				}

				ScratchImage compressedMipChain;
				HRESULT hr = Compress(mipChain.GetImages(), mipChain.GetImageCount(), mipChain.GetMetadata(), textureFormat, compressFlags, TEX_THRESHOLD_DEFAULT, compressedMipChain);
				RLS_ASSERT(SUCCEEDED(hr), "Failed to compress.");

				pTextureResource = CreateAndUploadTexture2DFromImage(compressedMipChain);
			}
			else
			{
				pTextureResource = CreateAndUploadTexture2DFromImage(mipChain);
			}
		}
		else
		{
			if (shouldCompress)
			{
				TEX_COMPRESS_FLAGS compressFlags = TEX_COMPRESS_FLAGS::TEX_COMPRESS_PARALLEL;
				if (importSettings.TextureCompressionType == ETextureCompressionType::BC7_Quick)
				{
					compressFlags |= TEX_COMPRESS_BC7_QUICK;
				}

				ScratchImage compressedImage;
				HRESULT hr = Compress(image.GetImages(), image.GetImageCount(), image.GetMetadata(), textureFormat, compressFlags, TEX_THRESHOLD_DEFAULT, compressedImage);
				RLS_ASSERT(SUCCEEDED(hr), "Failed to compress.");

				pTextureResource = CreateAndUploadTexture2DFromImage(compressedImage);
			}
			else
			{
				pTextureResource = CreateAndUploadTexture2DFromImage(image);
			}
		}

		const D3D12_RESOURCE_DESC resourceDescriptor = pTextureResource->GetDesc();

		D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDescriptor = {};
		shaderResourceViewDescriptor.Format = resourceDescriptor.Format;
		shaderResourceViewDescriptor.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDescriptor.Texture2D.MipLevels = resourceDescriptor.MipLevels;
		shaderResourceViewDescriptor.Texture2D.MostDetailedMip = 0;
		shaderResourceViewDescriptor.Texture2D.ResourceMinLODClamp = 0.0f;
		shaderResourceViewDescriptor.Texture2D.PlaneSlice = 0u;
		shaderResourceViewDescriptor.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		Texture2DSpecification specification;
		specification.DescriptorHandleSRV = MemoryManager::Get().CreateDescriptorHandle(DescriptorHandleType::SRV);
		DXCall_STD(D3D12Core::GetDevice()->CreateShaderResourceView(pTextureResource.Get(), &shaderResourceViewDescriptor, specification.DescriptorHandleSRV.CPUHandle));

		specification.pResource = pTextureResource;
		specification.Name = fullPath.filename().string();
		specification.Width = resourceDescriptor.Width;
		specification.Height = resourceDescriptor.Height;
		specification.Format = resourceDescriptor.Format;
		specification.MipCount = resourceDescriptor.MipLevels;
		specification.IsSRGB = importSettings.IsSRGB;
		specification.SampleCount = resourceDescriptor.SampleDesc.Count;

		//Compiler should utilize RVO to elide copy over move:
		Texture2D newTexture(specification);
		newTexture.SetCurrentState(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		return newTexture;
	}

	template<>
	void Importer::Import<Mesh>(const std::filesystem::path& fullPath, const AssetTypeInfo<Mesh>::Settings& importSettings) noexcept
	{
		RLS_ASSERT(std::filesystem::exists(fullPath), "File does not exist.");
		std::filesystem::path workingDirectory = fullPath;
		workingDirectory = workingDirectory.remove_filename();

		uint32_t importFlags = (uint32_t)(aiProcess_ConvertToLeftHanded | aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals);
		if (importSettings.OptimizeMesh)
		{
			importFlags |= (uint32_t)(aiProcess_Triangulate | aiProcess_ImproveCacheLocality | aiProcess_JoinIdenticalVertices);
		}
		if (importSettings.GenerateColliders)
		{
			importFlags |= (uint32_t)(aiProcess_GenBoundingBoxes);
		}

		Assimp::Importer importer;
		const aiScene* pAssimpScene = importer.ReadFile(fullPath.string(), importFlags);
		RLS_ASSERT(pAssimpScene && !(pAssimpScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) && pAssimpScene->mRootNode, importer.GetErrorString());

		DirectX::XMFLOAT4X4 identity;
		DirectX::XMStoreFloat4x4(&identity, DirectX::XMMatrixIdentity());

		entity rootEntity = importSettings.pScene ? importSettings.pScene->CreateEntity(std::filesystem::path(fullPath).filename().stem().string().c_str()) : NULL_ENTITY;
		ProcessAssimpNode(pAssimpScene->mRootNode, pAssimpScene, importSettings, identity, workingDirectory, rootEntity);
	}

	bool Importer::HasImageExtension(const std::string& fileExtension) noexcept
	{
		if (fileExtension == ".jpg")
		{
			return true;
		}
		else if (fileExtension == ".png")
		{
			return true;
		}
		else if (fileExtension == ".tga")
		{
			return true;
		}

		return false;
	}

	DXGI_FORMAT Importer::GetCompressedDXGITextureFormat(const TextureImportSettings& importSettings) noexcept
	{
		DXGI_FORMAT compressedFormat{};
		switch (importSettings.TextureCompressionType)
		{
		case ETextureCompressionType::BC5:
		{
			compressedFormat = DXGI_FORMAT::DXGI_FORMAT_BC5_UNORM;
			break;
		}
		case ETextureCompressionType::BC7:
		case ETextureCompressionType::BC7_Quick:
		{
			compressedFormat = importSettings.IsSRGB ? DXGI_FORMAT::DXGI_FORMAT_BC7_UNORM_SRGB : DXGI_FORMAT_BC7_UNORM;
			break;
		}
		}
		return compressedFormat;
	}

	Microsoft::WRL::ComPtr<ID3D12Resource> Importer::CreateAndUploadTexture2DFromImage(const DirectX::ScratchImage& image) noexcept
	{
		auto& metaData = image.GetMetadata();

		D3D12_RESOURCE_DESC textureDesc = {};
		textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		textureDesc.Alignment = 0;
		textureDesc.Width = metaData.width;
		textureDesc.Height = static_cast<UINT>(metaData.height);
		textureDesc.DepthOrArraySize = metaData.arraySize;
		textureDesc.MipLevels = static_cast<UINT16>(metaData.mipLevels);
		textureDesc.Format = metaData.format;
		textureDesc.SampleDesc.Count = 1u;
		textureDesc.SampleDesc.Quality = 0u;
		textureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		D3D12_HEAP_PROPERTIES heapProperties = {};
		heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProperties.CreationNodeMask = 1; 
		heapProperties.VisibleNodeMask = 1;

		Microsoft::WRL::ComPtr<ID3D12Resource> texture;
		HRESULT hr = D3D12Core::GetDevice()->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&textureDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(texture.GetAddressOf())
		);
		RLS_ASSERT(SUCCEEDED(hr), "Failed to create texture resource.");

		DirectX::ResourceUploadBatch uploadBatch(D3D12Core::GetDevice().Get());
		uploadBatch.Begin();

		const DirectX::Image* pImg = image.GetImages();
		for (uint32_t i{ 0u }; i < image.GetImageCount(); ++i, ++pImg)
		{
			D3D12_SUBRESOURCE_DATA subresourceData = {};
			subresourceData.pData = pImg->pixels;
			subresourceData.RowPitch = pImg->rowPitch;
			subresourceData.SlicePitch = pImg->slicePitch;
			uploadBatch.Upload(texture.Get(), static_cast<UINT>(i), &subresourceData, 1);
		}

		auto finish = uploadBatch.End(D3D12Core::GetCommandQueue().Get());
		finish.wait();

		RenderCommand::TransitionResource(texture, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

		return texture;
	}

	DirectX::XMMATRIX ConvertMatrix(aiMatrix4x4& inMat) 
	{
		DirectX::XMFLOAT4X4 t = DirectX::XMFLOAT4X4
		(
			inMat.a1, inMat.b1, inMat.c1, inMat.d1,
			inMat.a2, inMat.b2, inMat.c2, inMat.d2,
			inMat.a3, inMat.b3, inMat.c3, inMat.d3,
			inMat.a4, inMat.b4, inMat.c4, inMat.d4
		);

		return DirectX::XMLoadFloat4x4(&t);
	}

	void Importer::ProcessAssimpNode(aiNode* pNode, const aiScene* pAssimpScene, const MeshImportSettings& importSettings, const DirectX::XMFLOAT4X4& transform, const std::filesystem::path& workingDirectory, entity parent) noexcept
	{
		RLS_ASSERT(pNode && pAssimpScene, "Assimp data is invalid.");

		DirectX::XMMATRIX aiTransform = ConvertMatrix(pNode->mTransformation);
		DirectX::XMMATRIX transformMatrix = aiTransform;
		DirectX::XMMATRIX accumulatedTransform = DirectX::XMMatrixMultiply(aiTransform, DirectX::XMLoadFloat4x4(&transform));
		DirectX::XMFLOAT4X4 currentTransform;
		DirectX::XMStoreFloat4x4(&currentTransform, accumulatedTransform);

		entity aParent = parent;
		for (uint32_t i{ 0u }; i < pNode->mNumMeshes; ++i)
		{
			aiMesh* pMesh = pAssimpScene->mMeshes[pNode->mMeshes[i]];
			const AssetHandle meshHandle = ProcessMesh(pMesh, workingDirectory);

			AssetHandle materialHandle;
			if (importSettings.ImportMaterialsAndTextures)
			{
				materialHandle = ProcessMaterial(pMesh, pAssimpScene, workingDirectory);
			}
			else
			{
				materialHandle = AssetManager::GetDefaultMaterialHandle();
			}

			if (importSettings.pScene)
			{
				auto entity = importSettings.pScene->CreateEntity(pMesh->mName.C_Str());

				importSettings.pScene->GetEntityManager().AddOrReplace<OpaquePassComponent>(entity);
				importSettings.pScene->GetEntityManager().Get<DirtyTransformComponent>(entity).AdjustedWorldSpace = true;
				auto& tc = importSettings.pScene->GetEntityManager().Get<TransformComponent>(entity);
				tc.Transform = currentTransform;

				ImGuizmo::DecomposeMatrixToComponents(*tc.Transform.m, &tc.Translation.x, &tc.Rotation.x, &tc.Scale.x);

				if (importSettings.ImportMaterialsAndTextures)
				{
					importSettings.pScene->GetEntityManager().Add<MeshRendererComponent>(entity).AssetHandle = materialHandle;
				}
				importSettings.pScene->GetEntityManager().Add<MeshFilterComponent>(entity).AssetHandle = meshHandle;

				if (parent != NULL_ENTITY)
				{
					importSettings.pScene->ParentEntity(entity, parent);
				}
				aParent = entity;
			}
		}

		for (uint32_t i{ 0u }; i < pNode->mNumChildren; ++i)
		{
			ProcessAssimpNode(pNode->mChildren[i], pAssimpScene, importSettings, currentTransform, workingDirectory, aParent);
		}
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

	AssetHandle Importer::ProcessMaterial(aiMesh* pMesh, const aiScene* pAssimpScene, const std::filesystem::path& workingDirectory) noexcept
	{
		aiMaterial* m = pAssimpScene->mMaterials[pMesh->mMaterialIndex];

		std::filesystem::path inPath = workingDirectory / m->GetName().C_Str();
		inPath.append(".rasset");
		
		if (AssetManager::IsLoaded(inPath.string()))
		{
			return AssetManager::GetHandleByPath(inPath);
		}
		
		AssetHandle materialHandle = AssetManager::CreateNew<Material>();

		Material& mat = AssetManager::Get<Material>(materialHandle);
		mat.SetName(m->GetName().C_Str());

		aiString path;
		if (m->GetTexture(aiTextureType_BASE_COLOR, 0, &path) == aiReturn::aiReturn_SUCCESS)
		{
			std::filesystem::path fullPath = std::filesystem::absolute(workingDirectory / path.C_Str());
			AssetHandle baseColorID = AssetManager::LoadFromFile<Texture2D>(fullPath.string());
			mat.SetAlbedoTexture(baseColorID);

			int usesAlpha;
			if (aiReturn::aiReturn_SUCCESS == m->Get(AI_MATKEY_TEXFLAGS(aiTextureType_BASE_COLOR, 0), usesAlpha)) 
			{
				if (usesAlpha & aiTextureFlags_UseAlpha) 
				{
					RLS_ASSERT(false, "HALT");
				}
			}

		}
		else if (m->GetTexture(aiTextureType_DIFFUSE, 0, &path) == aiReturn::aiReturn_SUCCESS)
		{
			std::filesystem::path fullPath = std::filesystem::absolute(workingDirectory / path.C_Str());
			AssetHandle baseColorID = AssetManager::LoadFromFile<Texture2D>(fullPath.string());
			mat.SetAlbedoTexture(baseColorID);
		}
		else
		{
			aiColor4D baseColor(0.f, 0.f, 0.f, 0.0f);
			if (m->Get(AI_MATKEY_COLOR_DIFFUSE, baseColor) == AI_SUCCESS)
			{
				mat.m_AlbedoColor = { baseColor.r, baseColor.g , baseColor.b, baseColor.a };
			}
		}
		if (m->GetTexture(aiTextureType_METALNESS, 0, &path) == aiReturn::aiReturn_SUCCESS)
		{
			std::filesystem::path fullPath = std::filesystem::absolute(workingDirectory / path.C_Str());
			if (IsCombinedRoughnessAndMetalnessMap(fullPath.string()))
			{
				mat.m_CombinedRoughnessMetallnesMap = true;
			}

			AssetHandle metallicID = AssetManager::LoadFromFile<Texture2D>(fullPath.string());
			mat.SetMetallicTexture(metallicID);
		}

		if (!mat.m_CombinedRoughnessMetallnesMap)
		{
			if (m->GetTexture(aiTextureType::aiTextureType_DIFFUSE_ROUGHNESS, 0, &path) == aiReturn::aiReturn_SUCCESS)
			{
				std::filesystem::path fullPath = std::filesystem::absolute(workingDirectory / path.C_Str());
				AssetHandle roughnessID = AssetManager::LoadFromFile<Texture2D>(fullPath.string());
				mat.SetRoughnessTexture(roughnessID);
			}
		}

		if (m->GetTexture(aiTextureType::aiTextureType_NORMALS, 0, &path) == aiReturn::aiReturn_SUCCESS)
		{
			std::filesystem::path fullPath = std::filesystem::absolute(workingDirectory / path.C_Str());
			AssetHandle normalID = AssetManager::LoadFromFile<Texture2D>(fullPath.string());
			mat.SetNormalMap(normalID);
		}
		else if (m->GetTexture(aiTextureType::aiTextureType_NORMAL_CAMERA, 0, &path) == aiReturn::aiReturn_SUCCESS)
		{
			std::filesystem::path fullPath = std::filesystem::absolute(workingDirectory / path.C_Str());
			AssetHandle normalID = AssetManager::LoadFromFile<Texture2D>(fullPath.string());
			mat.SetNormalMap(normalID);
		}
		if (m->GetTexture(aiTextureType::aiTextureType_AMBIENT_OCCLUSION, 0, &path) == aiReturn::aiReturn_SUCCESS)
		{
			std::filesystem::path fullPath = std::filesystem::absolute(workingDirectory / path.C_Str());
			AssetHandle aoID = AssetManager::LoadFromFile<Texture2D>(fullPath.string());
			mat.SetAmbientOcclusionTexture(aoID);
		}
		if (m->GetTexture(aiTextureType::aiTextureType_EMISSION_COLOR, 0, &path) == aiReturn::aiReturn_SUCCESS)
		{
			std::filesystem::path fullPath = std::filesystem::absolute(workingDirectory / path.C_Str());
			AssetHandle emissionID = AssetManager::LoadFromFile<Texture2D>(fullPath.string());
			mat.SetEmissionTexture(emissionID);
			mat.m_EmissionIntensity = 1.0f;
		}
		else
		{
			aiColor4D emissionColor(0.f, 0.f, 0.f, 0.0f);
			if (m->Get(AI_MATKEY_COLOR_EMISSIVE, emissionColor) == AI_SUCCESS)
			{
				mat.m_EmissionColor = { emissionColor.r, emissionColor.g, emissionColor.b, emissionColor.a };
				if (emissionColor.r > 0.0f || emissionColor.g > 0.0f || emissionColor.b > 0.0f)
				{
					mat.m_EmissionIntensity = 1.0f;
				}
			}
		}
		if (m->GetTexture(aiTextureType::aiTextureType_DISPLACEMENT, 0, &path) == aiReturn::aiReturn_SUCCESS)
		{
			std::filesystem::path fullPath = std::filesystem::absolute(workingDirectory / path.C_Str());
			AssetHandle heightID = AssetManager::LoadFromFile<Texture2D>(fullPath.string());
			mat.SetHeightMap(heightID);
		}

		Serializer::Serialize<Material>(materialHandle, std::string(workingDirectory.string() + m->GetName().C_Str() + ".rasset"));

		return materialHandle;
	}

	AssetHandle Importer::ProcessMesh(aiMesh* pMesh, const std::filesystem::path& workingDirectory) noexcept
	{
		RLS_ASSERT(pMesh, "Assimp data is invalid.");
		RLS_ASSERT(pMesh->HasPositions(), "Mesh contains no position data.");
		RLS_ASSERT(pMesh->HasFaces(), "Mesh contains no faces data.");
		RLS_ASSERT(pMesh->HasNormals(), "Mesh contains no normal data.");
		RLS_ASSERT(pMesh->HasTangentsAndBitangents(), "Mesh contains no tangent and/or bitangent data.");
		RLS_ASSERT(pMesh->HasTextureCoords(0u), "Mesh contains no texture coordinate data.");

		std::filesystem::path fullPath = workingDirectory / pMesh->mName.C_Str();
		fullPath.append(".rasset");
		if (AssetManager::IsLoaded(fullPath.string()))
		{
			return AssetManager::GetHandleByPath(fullPath);
		}
		//TODO:
		//else if (AssetManager::IsAssetPathMapped(workingDirectory.string() + pMesh->mName.C_Str() + ".rmesh"))
		//{
		//	return ModelSerializer::Deserialize(workingDirectory.string() + pMesh->mName.C_Str() + ".rmesh");
		//}

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

		AssetHandle meshHandle = AssetManager::CreateNew<Mesh>();
		Mesh& mesh = AssetManager::Get<Mesh>(meshHandle);
		mesh.SetName(pMesh->mName.C_Str());
		mesh.SetVertexBuffer(std::make_unique<VertexBuffer>(&vbSpec));
		mesh.SetIndexBuffer(std::make_unique<IndexBuffer>(&ibSpec));

		Serializer::Serialize<Mesh>(meshHandle, workingDirectory.string());

		RLS_CORE_INFO("Loaded mesh '{0}' with GUID: {1}", pMesh->mName.C_Str(), ConvertUUIDToString(meshHandle.Uuid));

		return meshHandle;
	}
}