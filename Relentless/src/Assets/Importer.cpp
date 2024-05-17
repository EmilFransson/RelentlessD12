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
	using namespace Microsoft::WRL;

	std::unordered_map<AssetType, std::function<bool(const std::filesystem::path&, const std::filesystem::path&, const std::optional<AssetImportSettingsVariant>&)>> Importer::m_LoadFuncs = {
		{AssetType::Texture2D, [](const std::filesystem::path& path, const std::filesystem::path& dstPath,const std::optional<AssetImportSettingsVariant>& importSettings)
			{
				if (!ImportTexture(path, dstPath, CastToImportSettings<TextureImportSettings>(importSettings)))
					return false;

				return true;
			}},
		{AssetType::Mesh, [](const std::filesystem::path& path, const std::filesystem::path& dstPath, const std::optional<AssetImportSettingsVariant>& importSettings)
		{
			return ImportModel(path, dstPath, CastToImportSettings<MeshImportSettings>(importSettings));
		}}
	};

	//template<>
	//void Importer::Import<Mesh>(const std::filesystem::path& fullPath, const std::string& destinationDirectory, const AssetTypeInfo<Mesh>::Settings& importSettings) noexcept
	//{
	//	RLS_ASSERT(std::filesystem::exists(fullPath), "File does not exist.");
	//	std::filesystem::path workingDirectory = fullPath;
	//	workingDirectory = workingDirectory.remove_filename();
	//
	//	uint32_t importFlags = (uint32_t)(aiProcess_ConvertToLeftHanded | aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals);
	//	if (importSettings.OptimizeMesh)
	//	{
	//		importFlags |= (uint32_t)(aiProcess_Triangulate | aiProcess_ImproveCacheLocality | aiProcess_JoinIdenticalVertices);
	//	}
	//	if (importSettings.GenerateColliders)
	//	{
	//		importFlags |= (uint32_t)(aiProcess_GenBoundingBoxes);
	//	}
	//
	//	Assimp::Importer importer;
	//	const aiScene* pAssimpScene = importer.ReadFile(fullPath.string(), importFlags);
	//	RLS_ASSERT(pAssimpScene && !(pAssimpScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) && pAssimpScene->mRootNode, importer.GetErrorString());
	//
	//	DirectX::XMFLOAT4X4 identity;
	//	DirectX::XMStoreFloat4x4(&identity, DirectX::XMMatrixIdentity());
	//
	//	entity rootEntity;
	//	//entity rootEntity = importSettings.pScene ? importSettings.pScene->CreateEntity(std::filesystem::path(fullPath).filename().stem().string().c_str()) : NULL_ENTITY;
	//	ProcessAssimpNode(pAssimpScene->mRootNode, pAssimpScene, importSettings, identity, workingDirectory, destinationDirectory, rootEntity);
	//}

	static void LogHR(HRESULT hr, const std::string& contextualString, const std::filesystem::path& srcFilepath) noexcept
	{
		if (hr != S_OK)
		{
			const _com_error error(hr);
			RLS_CORE_ERROR("[Importer]: Failed to {0} file with path '{1}'; operation failed with message '{2}'", contextualString.c_str(), srcFilepath.string().c_str(), error.ErrorMessage());
		}
	}

	[[nodiscard]] AssetType GetAssetTypeFromExtensionType(ExtensionType extensionType) noexcept
	{
		switch (extensionType)
		{
		case ExtensionType::JPG:
		case ExtensionType::PNG:
		case ExtensionType::TGA:
		case ExtensionType::DDS:
		case ExtensionType::TIFF:
			return AssetType::Texture2D;
		case ExtensionType::FBX:
		case ExtensionType::OBJ:
		case ExtensionType::GLTF:
			return AssetType::Mesh;
		default:
			return AssetType::Undefined;
		}
	}

	void Importer::RequestAsyncLoadFromFile(const std::filesystem::path& srcPath, const std::filesystem::path& dstAssetDirectorPath, const std::optional<AssetImportSettingsVariant>& optionalImportSettings /*= std::nullopt*/) noexcept
	{
	//	return Application::Get().GetThreadPool().Submit([srcPath, dstAssetDirectorPath, optionalImportSettings]()
	//		{
				const ExtensionType extensionType = GetExtensionTypeFromPath(srcPath);
				const AssetType assetType = GetAssetTypeFromExtensionType(extensionType);
				if (assetType == AssetType::Undefined)
				{
					RLS_CORE_ERROR("[Importer]: File with path {0} is not supported.", srcPath.string().c_str());
					return;
				}

				if (m_LoadFuncs[assetType](srcPath.string(), dstAssetDirectorPath, optionalImportSettings))
				{
					RLS_CORE_INFO("[Importer]: Imported asset(s) with path {0} to directory {1}", srcPath.string().c_str(), dstAssetDirectorPath.string().c_str());
				}
				else
				{
					RLS_CORE_ERROR("[Importer]: Failed to load asset from file with path: '{0}'.", srcPath.string().c_str());
				}
			//});
	}

	bool Importer::ImportTexture(const std::filesystem::path& srcPath, const std::filesystem::path& dstAssetDirectorPath, const TextureImportSettings& importSettings) noexcept
	{
		if (!File::Exists(srcPath))
		{
			RLS_CORE_ERROR("[Importer]: Failed to import texture file with path '{0}'; file does not exist.", srcPath.string().c_str());
			return false;
		}

		const ExtensionType extensionType = GetExtensionTypeFromPath(srcPath);
		DirectX::ScratchImage image;
		HRESULT result = S_OK;
		switch (extensionType)
		{
		case ExtensionType::TGA:
			result = LoadFromTGAFile(srcPath.c_str(), nullptr, image);
			break;
		case ExtensionType::JPG:
		case ExtensionType::PNG:
		case ExtensionType::TIFF:
		{
			DirectX::WIC_FLAGS importFlags = DirectX::WIC_FLAGS::WIC_FLAGS_NONE;
			importSettings.IsSRGB ? importFlags |= DirectX::WIC_FLAGS::WIC_FLAGS_FORCE_SRGB : importFlags |= DirectX::WIC_FLAGS::WIC_FLAGS_FORCE_RGB;
			result = LoadFromWICFile(srcPath.c_str(), importFlags, nullptr, image);
			break;
		}
		default:
		{
			RLS_CORE_ERROR("[Importer]: Failed to import texture file with path '{0}'; file type is not supported.", srcPath.string().c_str());
			return false;
		}
		}

		if (result != S_OK)
		{
			LogHR(result, "import", srcPath);
			return false;
		}

		if (importSettings.GenerateMipMaps)
		{
			DirectX::ScratchImage mipChain;
			const HRESULT hr = GenerateMipMaps(image.GetImages()[0], DirectX::TEX_FILTER_DEFAULT, 0u, mipChain);
			if (hr != S_OK)
				LogHR(hr, "generate mipmaps", srcPath);
			else
				image = std::move(mipChain);
		}

		const bool shouldCompress = importSettings.TextureCompressionType != ETextureCompressionType::Uncompressed;
		if (shouldCompress)
		{
			DirectX::TEX_COMPRESS_FLAGS compressFlags = DirectX::TEX_COMPRESS_FLAGS::TEX_COMPRESS_PARALLEL;
			if (importSettings.TextureCompressionType == ETextureCompressionType::BC7_Quick)
				compressFlags |= DirectX::TEX_COMPRESS_BC7_QUICK;

			DirectX::ScratchImage compressedImage;
			const HRESULT hr = Compress(image.GetImages(), image.GetImageCount(), image.GetMetadata(), GetCompressedDXGITextureFormat(importSettings), compressFlags, DirectX::TEX_THRESHOLD_DEFAULT, compressedImage);
			if (hr != S_OK)
				LogHR(hr, "compress", srcPath);
			else
				image = std::move(compressedImage);
		}

		const Microsoft::WRL::ComPtr<ID3D12Resource> pTextureResource = CreateAndUploadTexture2DFromImage(image);
		if (!pTextureResource)
			return false;

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
		specification.Name = FilepathUtils::ExtractFilename(srcPath);
		specification.Width = resourceDescriptor.Width;
		specification.Height = resourceDescriptor.Height;
		specification.Format = resourceDescriptor.Format;
		specification.MipCount = resourceDescriptor.MipLevels;
		specification.IsSRGB = importSettings.IsSRGB;
		specification.SampleCount = resourceDescriptor.SampleDesc.Count;

		std::shared_ptr<Texture2D> pNewTexture = std::make_shared<Texture2D>(specification);

		const uint32_t index = AssetManager::GetStorage<Texture2D>().Add(std::move(pNewTexture));
		const auto&[handle, _] = AssetManager::InsertMetaData(CreateUUID(), index, AssetType::Texture2D);

		AssetMetaData metaData;
		metaData.Name = specification.Name;
		metaData.SourcePath = srcPath;
		metaData.Uuid = handle->second.Uuid;
		metaData.AssetType = AssetType::Texture2D;

		auto now = std::chrono::system_clock::now();
		auto duration = now.time_since_epoch();
		auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
		metaData.ModificationDateAndTime = static_cast<uint64_t>(millis);

		const std::string filename = FilepathUtils::ExtractFilenameWithoutExtension(srcPath) + ASSET_EXTENSION;
		const std::filesystem::path fullDestinationPath = FilepathUtils::Combine(dstAssetDirectorPath, filename);

		AssetRegistry::Map(fullDestinationPath, metaData, AssetRegistry::MapOperation::Override);
		if (!Serializer::Serialize(fullDestinationPath, handle->second))
			RLS_CORE_ERROR("[Importer]: Failed to serialize imported texture with name '{0}'.", metaData.Name.c_str());

		return true;
	}

	static bool ImportAssimpMesh(aiMesh* pMesh, AssetHandle& outHandle) noexcept
	{
		if (!pMesh)
		{
			RLS_CORE_ERROR("[Importer]: Failed to import mesh; assimp mesh is invalid.");
			outHandle = NULL_HANDLE;
			return false;
		}

		if (!pMesh->HasPositions() || !pMesh->HasFaces() || !pMesh->HasNormals() || !pMesh->HasTangentsAndBitangents() || !pMesh->HasTextureCoords(0u))
		{
			RLS_CORE_ERROR("[Importer]: Error in importing mesh with name '{0}'; one or more vertex fields does not exist.", pMesh->mName.C_Str());
			outHandle = NULL_HANDLE;
			return false;
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

		AssetHandle meshHandle = AssetManager::CreateNew<Mesh>();
		Mesh& mesh = AssetManager::Get<Mesh>(meshHandle);
		mesh.SetName(pMesh->mName.C_Str());

		GPUTaskManager& gpuTaskManager = Application::Get().GetGPUTaskManager();

		{
			D3D12_HEAP_PROPERTIES heapProperties = {};
			heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
			heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			heapProperties.CreationNodeMask = 0u;
			heapProperties.VisibleNodeMask = 0u;

			D3D12_HEAP_DESC heapDescriptor = {};
			heapDescriptor.SizeInBytes = vbSpec.TotalSizeInBytes;
			heapDescriptor.Properties = heapProperties;
			heapDescriptor.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
			heapDescriptor.Flags = D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES | D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES;

			Microsoft::WRL::ComPtr<ID3D12Heap> pVBHeap{ nullptr };
			DXCall(D3D12Core::GetDevice()->CreateHeap(&heapDescriptor, IID_PPV_ARGS(&pVBHeap)));

			D3D12_RESOURCE_DESC resourceDescriptor = {};
			resourceDescriptor.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			resourceDescriptor.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
			resourceDescriptor.Width = vbSpec.TotalSizeInBytes;
			resourceDescriptor.Height = 1u;
			resourceDescriptor.DepthOrArraySize = 1u;
			resourceDescriptor.MipLevels = 1u;
			resourceDescriptor.Format = DXGI_FORMAT_UNKNOWN;
			resourceDescriptor.SampleDesc = { 1u, 0u };
			resourceDescriptor.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			resourceDescriptor.Flags = D3D12_RESOURCE_FLAG_NONE;

			Microsoft::WRL::ComPtr<ID3D12Resource> pVBResource = nullptr;
			DXCall(D3D12Core::GetDevice()->CreatePlacedResource
			(
				pVBHeap.Get(),
				0u,
				&resourceDescriptor,
				D3D12_RESOURCE_STATE_COPY_DEST,
				nullptr,
				IID_PPV_ARGS(&pVBResource)
			));

			D3D12_HEAP_PROPERTIES uploadHeapProperties = {};
			{
				uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
				uploadHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
				uploadHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
				uploadHeapProperties.CreationNodeMask = 0u;
				uploadHeapProperties.VisibleNodeMask = 0u;
			}

			D3D12_HEAP_DESC uploadHeapDescriptor = {};
			{
				uploadHeapDescriptor.SizeInBytes = vbSpec.TotalSizeInBytes;
				uploadHeapDescriptor.Properties = uploadHeapProperties;
				uploadHeapDescriptor.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
				uploadHeapDescriptor.Flags = D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES | D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES;
			}

			Microsoft::WRL::ComPtr<ID3D12Heap> pTempHeap{ nullptr };
			DXCall(D3D12Core::GetDevice()->CreateHeap(&uploadHeapDescriptor, IID_PPV_ARGS(&pTempHeap)));

			D3D12_RESOURCE_DESC uploadBufferDescriptor = {};
			{
				uploadBufferDescriptor.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
				uploadBufferDescriptor.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
				uploadBufferDescriptor.Width = vbSpec.TotalSizeInBytes;
				uploadBufferDescriptor.Height = 1u;
				uploadBufferDescriptor.DepthOrArraySize = 1u;
				uploadBufferDescriptor.MipLevels = 1u;
				uploadBufferDescriptor.Format = DXGI_FORMAT_UNKNOWN;
				uploadBufferDescriptor.SampleDesc.Count = 1u;
				uploadBufferDescriptor.SampleDesc.Quality = 0u;
				uploadBufferDescriptor.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
				uploadBufferDescriptor.Flags = D3D12_RESOURCE_FLAG_NONE;
			}

			Microsoft::WRL::ComPtr<ID3D12Resource> pUploadResource = nullptr;
			DXCall(D3D12Core::GetDevice()->CreatePlacedResource
			(
				pTempHeap.Get(),
				0u,
				&uploadBufferDescriptor,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&pUploadResource))
			);

			D3D12_RANGE nullRange = { 0,0 };
			unsigned char* mappedPtr = nullptr;
			DXCall(pUploadResource->Map(0u, &nullRange, reinterpret_cast<void**>(&mappedPtr)));
			
			std::memcpy(mappedPtr, vbSpec.pBuffer, vbSpec.TotalSizeInBytes);

			ComPtr<ID3D12GraphicsCommandList4> copyCommandList = gpuTaskManager.RequestCommandList(CommandType::Copy);

			DXCall_STD(copyCommandList->CopyBufferRegion(pVBResource.Get(), 0u, pUploadResource.Get(), 0, vbSpec.TotalSizeInBytes));
			D3D12_RESOURCE_BARRIER resourceTransitionBarrier{};
			resourceTransitionBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			resourceTransitionBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			resourceTransitionBarrier.Transition.pResource = pVBResource.Get();
			resourceTransitionBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
			resourceTransitionBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;//D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
			resourceTransitionBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			DXCall_STD(copyCommandList->ResourceBarrier(1u, &resourceTransitionBarrier));
			
			gpuTaskManager.ExecuteCommandListBlocking(copyCommandList);

			DXCall_STD(pUploadResource->Unmap(0u, &nullRange));

			NAME_D12_OBJECT(pUploadResource, ConvertStringToWstring(vbSpec.Name + " - upload buffer").c_str());
			NAME_D12_OBJECT(pVBResource, ConvertStringToWstring(vbSpec.Name).c_str());

			mesh.SetVertexBuffer(std::make_unique<VertexBuffer>(vbSpec.NrOfVertices, vbSpec.TotalSizeInBytes, vbSpec.Stride, pVBResource, vbSpec.Name, vbSpec.pBuffer));
		}

		//INDEX BUFFER!
		{
			D3D12_HEAP_PROPERTIES heapProperties = {};
			heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
			heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			heapProperties.CreationNodeMask = 0u;
			heapProperties.VisibleNodeMask = 0u;

			D3D12_HEAP_DESC heapDescriptor = {};
			heapDescriptor.SizeInBytes = ibSpec.TotalSizeInBytes;
			heapDescriptor.Properties = heapProperties;
			heapDescriptor.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
			heapDescriptor.Flags = D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES | D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES;

			Microsoft::WRL::ComPtr<ID3D12Heap> pIBHeap{ nullptr };
			DXCall(D3D12Core::GetDevice()->CreateHeap(&heapDescriptor, IID_PPV_ARGS(&pIBHeap)));

			D3D12_RESOURCE_DESC resourceDescriptor = {};
			resourceDescriptor.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			resourceDescriptor.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
			resourceDescriptor.Width = ibSpec.TotalSizeInBytes;
			resourceDescriptor.Height = 1u;
			resourceDescriptor.DepthOrArraySize = 1u;
			resourceDescriptor.MipLevels = 1u;
			resourceDescriptor.Format = DXGI_FORMAT_UNKNOWN;
			resourceDescriptor.SampleDesc = { 1u, 0u };
			resourceDescriptor.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			resourceDescriptor.Flags = D3D12_RESOURCE_FLAG_NONE;

			Microsoft::WRL::ComPtr<ID3D12Resource> pIBResource = nullptr;
			DXCall(D3D12Core::GetDevice()->CreatePlacedResource
			(
				pIBHeap.Get(),
				0u,
				&resourceDescriptor,
				D3D12_RESOURCE_STATE_COPY_DEST,
				nullptr,
				IID_PPV_ARGS(&pIBResource)
			));

			D3D12_HEAP_PROPERTIES uploadHeapProperties = {};
			{
				uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
				uploadHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
				uploadHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
				uploadHeapProperties.CreationNodeMask = 0u;
				uploadHeapProperties.VisibleNodeMask = 0u;
			}

			D3D12_HEAP_DESC uploadHeapDescriptor = {};
			{
				uploadHeapDescriptor.SizeInBytes = ibSpec.TotalSizeInBytes;
				uploadHeapDescriptor.Properties = uploadHeapProperties;
				uploadHeapDescriptor.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
				uploadHeapDescriptor.Flags = D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES | D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES;
			}

			Microsoft::WRL::ComPtr<ID3D12Heap> pTempHeap{ nullptr };
			DXCall(D3D12Core::GetDevice()->CreateHeap(&uploadHeapDescriptor, IID_PPV_ARGS(&pTempHeap)));

			D3D12_RESOURCE_DESC uploadBufferDescriptor = {};
			{
				uploadBufferDescriptor.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
				uploadBufferDescriptor.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
				uploadBufferDescriptor.Width = ibSpec.TotalSizeInBytes;
				uploadBufferDescriptor.Height = 1u;
				uploadBufferDescriptor.DepthOrArraySize = 1u;
				uploadBufferDescriptor.MipLevels = 1u;
				uploadBufferDescriptor.Format = DXGI_FORMAT_UNKNOWN;
				uploadBufferDescriptor.SampleDesc.Count = 1u;
				uploadBufferDescriptor.SampleDesc.Quality = 0u;
				uploadBufferDescriptor.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
				uploadBufferDescriptor.Flags = D3D12_RESOURCE_FLAG_NONE;
			}

			Microsoft::WRL::ComPtr<ID3D12Resource> pUploadResource = nullptr;
			DXCall(D3D12Core::GetDevice()->CreatePlacedResource
			(
				pTempHeap.Get(),
				0u,
				&uploadBufferDescriptor,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&pUploadResource))
			);

			D3D12_RANGE nullRange = { 0,0 };
			unsigned char* mappedPtr = nullptr;
			DXCall(pUploadResource->Map(0u, &nullRange, reinterpret_cast<void**>(&mappedPtr)));
			
			std::memcpy(mappedPtr, ibSpec.pBuffer, ibSpec.TotalSizeInBytes);

			ComPtr<ID3D12GraphicsCommandList4> copyCommandList = gpuTaskManager.RequestCommandList(CommandType::Copy);

			DXCall_STD(copyCommandList->CopyBufferRegion(pIBResource.Get(), 0u, pUploadResource.Get(), 0, ibSpec.TotalSizeInBytes));
			D3D12_RESOURCE_BARRIER resourceTransitionBarrier{};
			resourceTransitionBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			resourceTransitionBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			resourceTransitionBarrier.Transition.pResource = pIBResource.Get();
			resourceTransitionBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
			resourceTransitionBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
			resourceTransitionBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			DXCall_STD(copyCommandList->ResourceBarrier(1u, &resourceTransitionBarrier));

			gpuTaskManager.ExecuteCommandListBlocking(copyCommandList);

			DXCall_STD(pUploadResource->Unmap(0u, &nullRange));

			NAME_D12_OBJECT(pUploadResource, ConvertStringToWstring(ibSpec.Name + " - upload buffer").c_str());
			NAME_D12_OBJECT(pIBResource, ConvertStringToWstring(ibSpec.Name).c_str());

			mesh.SetIndexBuffer(std::make_unique<IndexBuffer>(ibSpec.NrOfIndices, ibSpec.TotalSizeInBytes, ibSpec.Stride, pIBResource, ibSpec.Name, ibSpec.pBuffer));
		}

		RLS_CORE_INFO("Loaded mesh '{0}' with GUID: '{1}'.", pMesh->mName.C_Str(), ConvertUUIDToString(meshHandle.Uuid));
		outHandle = meshHandle;
	}

	bool Importer::ImportModel(const std::filesystem::path& srcPath, const std::filesystem::path& destinationDirectory, const MeshImportSettings& importSettings) noexcept
	{
		uint32_t importFlags = (uint32_t)(aiProcess_ConvertToLeftHanded | aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals);
		if (importSettings.OptimizeMesh)
			importFlags |= (uint32_t)(aiProcess_Triangulate | aiProcess_ImproveCacheLocality | aiProcess_JoinIdenticalVertices);
		if (importSettings.GenerateColliders)
			importFlags |= (uint32_t)(aiProcess_GenBoundingBoxes);

		Assimp::Importer importer;
		const aiScene* pAssimpScene = importer.ReadFile(srcPath.string(), importFlags);
		const bool incompleteScene = pAssimpScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE;

		if (!pAssimpScene || incompleteScene || !pAssimpScene->mRootNode)
		{
			RLS_CORE_ERROR("[Importer]: Reading file for model with path '{0}' failed with error message: '{1}'.", srcPath.string().c_str(), importer.GetErrorString());
			return false;
		}
		
		std::vector<std::future<void>> futures;

		std::set<aiMesh*> uniqueMeshes;
		for (uint32_t i{ 0u }; i < pAssimpScene->mNumMeshes; ++i)
			uniqueMeshes.insert(pAssimpScene->mMeshes[i]);

		for (aiMesh* mesh : uniqueMeshes)
		{
			//futures.push_back(Application::Get().GetThreadPool().Submit([mesh, srcPath, destinationDirectory]()
			//	{
					const std::string fullyQualifiedMeshAssetFilename = std::string(mesh->mName.C_Str()) + ".rasset";
					const std::filesystem::path fullDestinationPath = FilepathUtils::Combine(destinationDirectory, fullyQualifiedMeshAssetFilename);
					if (AssetManager::IsLoaded(fullDestinationPath.string()))
						return false;

					AssetHandle assetHandle = NULL_HANDLE;
					if (!ImportAssimpMesh(mesh, assetHandle))
						return false;

					AssetMetaData metaData;
					metaData.AssetType = AssetType::Mesh;
					metaData.Name = mesh->mName.C_Str();
					metaData.Uuid = assetHandle.Uuid;
					metaData.SourcePath = srcPath.string();
					auto now = std::chrono::system_clock::now();
					auto duration = now.time_since_epoch();
					auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
					metaData.ModificationDateAndTime = static_cast<uint64_t>(millis);

					AssetRegistry::Map(fullDestinationPath, metaData, AssetRegistry::MapOperation::Override);

					if (!destinationDirectory.empty())
						Serializer::Serialize(fullDestinationPath, assetHandle);
				//}));
		}

		const std::filesystem::path workingDirectory = srcPath.parent_path();

		if (importSettings.ImportMaterialsAndTextures)
		{
			std::set<std::filesystem::path> uniqueTextures;
			std::unordered_map<std::filesystem::path, bool> pathToSRGBBoolMap;

			for (uint32_t i{ 0u }; i < pAssimpScene->mNumMaterials; ++i)
			{
				auto&& TryGetTexture = [&uniqueTextures, &pathToSRGBBoolMap, &workingDirectory](const aiMaterial* pMaterial, aiTextureType textureType, bool asSRGB) -> bool
					{
						aiString path;
						if (pMaterial->GetTexture(textureType, 0, &path) == aiReturn::aiReturn_SUCCESS)
						{
							const std::filesystem::path abolutePath = FilepathUtils::Combine(workingDirectory, path.C_Str());
							auto [_, inserted] = uniqueTextures.insert(abolutePath);
							if (inserted)
								pathToSRGBBoolMap[abolutePath] = asSRGB;
							
							return true;
						}
						return false;
					};

				const aiMaterial* pMaterial = pAssimpScene->mMaterials[i];
				if (!TryGetTexture(pMaterial, aiTextureType_BASE_COLOR, true))
				{
					TryGetTexture(pMaterial, aiTextureType_DIFFUSE, true);
				}
				TryGetTexture(pMaterial, aiTextureType_METALNESS, false);
				TryGetTexture(pMaterial, aiTextureType_DIFFUSE_ROUGHNESS, false);
				if (!TryGetTexture(pMaterial, aiTextureType_NORMALS, false))
				{
					TryGetTexture(pMaterial, aiTextureType_NORMAL_CAMERA, false);
				}
				TryGetTexture(pMaterial, aiTextureType_AMBIENT_OCCLUSION, false);
				TryGetTexture(pMaterial, aiTextureType_EMISSION_COLOR, true);
				TryGetTexture(pMaterial, aiTextureType_DISPLACEMENT, false);
			}

			for (auto& path : uniqueTextures)
			{
				TextureImportSettings importSettings;
				importSettings.GenerateMipMaps = true;
				importSettings.IsHDR = false;
				importSettings.IsSRGB = pathToSRGBBoolMap[path.string()];
				importSettings.TextureCompressionType = importSettings.TextureCompressionType;

				RequestAsyncLoadFromFile(path, destinationDirectory, importSettings);
				//futures.push_back();
			}
		}
		
		//for (auto& future : futures)
		//	future.wait();

		return true;
	}

	ExtensionType Importer::GetExtensionTypeFromPath(const std::filesystem::path& fullPath) noexcept
	{
		const std::string extension = FilepathUtils::ExtractExtension(fullPath);
		if (extension == ".jpg")
			return ExtensionType::JPG;
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
		else if (extension == ".fbx")
			return ExtensionType::FBX;
		else if (extension == ".obj")
			return ExtensionType::OBJ;
		else if (extension == ".gltf")
			return ExtensionType::GLTF;
		else
			return ExtensionType::UNKNOWN;
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

		Microsoft::WRL::ComPtr<ID3D12Resource> d3d12TextureResource = nullptr;
		const HRESULT hr = D3D12Core::GetDevice()->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&textureDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(d3d12TextureResource.GetAddressOf())
		);
		if (hr != S_OK)
			return nullptr;

		DirectX::ResourceUploadBatch uploadBatch(D3D12Core::GetDevice().Get());
		uploadBatch.Begin();

		const DirectX::Image* pImg = image.GetImages();
		for (uint32_t i{ 0u }; i < image.GetImageCount(); ++i, ++pImg)
		{
			D3D12_SUBRESOURCE_DATA subresourceData = {};
			subresourceData.pData = pImg->pixels;
			subresourceData.RowPitch = pImg->rowPitch;
			subresourceData.SlicePitch = pImg->slicePitch;
			uploadBatch.Upload(d3d12TextureResource.Get(), static_cast<UINT>(i), &subresourceData, 1);
		}

		auto finish = uploadBatch.End(Application::Get().GetGPUTaskManager().GetCommandQueue(CommandType::Copy).Get());
		finish.wait();

		D3D12_RESOURCE_BARRIER resourceTransitionBarrier{};
		resourceTransitionBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		resourceTransitionBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		resourceTransitionBarrier.Transition.pResource = d3d12TextureResource.Get();
		resourceTransitionBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		resourceTransitionBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		resourceTransitionBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		ComPtr<ID3D12GraphicsCommandList4> pDirectCommandList = Application::Get().GetGPUTaskManager().RequestCommandList(CommandType::Direct);

		DXCall_STD(pDirectCommandList->ResourceBarrier(1u, &resourceTransitionBarrier));
		Application::Get().GetGPUTaskManager().ExecuteCommandListBlocking(pDirectCommandList);
		
		return d3d12TextureResource;
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

	//void Importer::ProcessAssimpNode(aiNode* pNode, const aiScene* pAssimpScene, const MeshImportSettings& importSettings, const DirectX::XMFLOAT4X4& transform, const std::filesystem::path& workingDirectory, const std::string& destinationDirectory, entity parent) noexcept
	//{
	//	RLS_ASSERT(pNode && pAssimpScene, "Assimp data is invalid.");
	//
	//	DirectX::XMMATRIX aiTransform = ConvertMatrix(pNode->mTransformation);
	//	DirectX::XMMATRIX transformMatrix = aiTransform;
	//	DirectX::XMMATRIX accumulatedTransform = DirectX::XMMatrixMultiply(aiTransform, DirectX::XMLoadFloat4x4(&transform));
	//	DirectX::XMFLOAT4X4 currentTransform;
	//	DirectX::XMStoreFloat4x4(&currentTransform, accumulatedTransform);
	//
	//	entity aParent = parent;
	//	for (uint32_t i{ 0u }; i < pNode->mNumMeshes; ++i)
	//	{
	//		aiMesh* pMesh = pAssimpScene->mMeshes[pNode->mMeshes[i]];
	//		const AssetHandle meshHandle = ProcessMesh(pMesh, workingDirectory, destinationDirectory);
	//
	//		AssetHandle materialHandle;
	//		if (importSettings.ImportMaterialsAndTextures)
	//		{
	//			materialHandle = ProcessMaterial(pMesh, pAssimpScene, workingDirectory, destinationDirectory);
	//		}
	//		else
	//		{
	//			materialHandle = AssetManager::GetDefaultMaterialHandle();
	//		}
	//
	//		//if (importSettings.pScene)
			//{
			//	auto entity = importSettings.pScene->CreateEntity(pMesh->mName.C_Str());
			//
			//	importSettings.pScene->GetEntityManager().AddOrReplace<OpaquePassComponent>(entity);
			//	importSettings.pScene->GetEntityManager().Get<DirtyTransformComponent>(entity).AdjustedWorldSpace = true;
			//	auto& tc = importSettings.pScene->GetEntityManager().Get<TransformComponent>(entity);
			//	tc.Transform = currentTransform;
			//
			//	ImGuizmo::DecomposeMatrixToComponents(*tc.Transform.m, &tc.Translation.x, &tc.Rotation.x, &tc.Scale.x);
			//
			//	if (importSettings.ImportMaterialsAndTextures)
			//	{
			//		importSettings.pScene->GetEntityManager().Add<MeshRendererComponent>(entity).AssetHandle = materialHandle;
			//	}
			//	importSettings.pScene->GetEntityManager().Add<MeshFilterComponent>(entity).AssetHandle = meshHandle;
			//
			//	if (parent != NULL_ENTITY)
			//	{
			//		importSettings.pScene->ParentEntity(entity, parent);
			//	}
			//	aParent = entity;
			//}
	//	}
	//
	//	for (uint32_t i{ 0u }; i < pNode->mNumChildren; ++i)
	//	{
	//		ProcessAssimpNode(pNode->mChildren[i], pAssimpScene, importSettings, currentTransform, workingDirectory, destinationDirectory, aParent);
	//	}
	//}

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

	//AssetHandle Importer::ProcessMaterial(aiMesh* pMesh, const aiScene* pAssimpScene, const std::filesystem::path& workingDirectory, const std::string& destinationDirectory) noexcept
	//{
	//	aiMaterial* m = pAssimpScene->mMaterials[pMesh->mMaterialIndex];
	//
	//	const std::string inPath = destinationDirectory + "\\" +  std::string(m->GetName().C_Str()) + ".rasset";
	//	
	//	if (AssetManager::IsLoaded(inPath))
	//	{
	//		return AssetManager::GetHandleByPath(inPath);
	//	}
	//	
	//	AssetHandle materialHandle = AssetManager::CreateNew<Material>();
	//
	//	Material& mat = AssetManager::Get<Material>(materialHandle);
	//	mat.SetName(m->GetName().C_Str());
	//
	//	aiString path;
	//	if (m->GetTexture(aiTextureType_BASE_COLOR, 0, &path) == aiReturn::aiReturn_SUCCESS)
	//	{
	//		std::filesystem::path fullPath = std::filesystem::absolute(workingDirectory / path.C_Str());
	//		RequestAsyncLoadFromFile(fullPath, destinationDirectory, 
	//			[&mat](AssetHandle assetHandle)
	//			{
	//				if (assetHandle.IsValid())
	//					mat.SetAlbedoTexture(assetHandle);
	//				else
	//					mat.SetAlbedoTexture(AssetManager::GetInvalidTextureHandle());
	//			});
	//
	//		int usesAlpha;
	//		if (aiReturn::aiReturn_SUCCESS == m->Get(AI_MATKEY_TEXFLAGS(aiTextureType_BASE_COLOR, 0), usesAlpha)) 
	//		{
	//			if (usesAlpha & aiTextureFlags_UseAlpha) 
	//			{
	//				RLS_ASSERT(false, "HALT");
	//			}
	//		}
	//
	//	}
	//	else if (m->GetTexture(aiTextureType_DIFFUSE, 0, &path) == aiReturn::aiReturn_SUCCESS)
	//	{
	//		std::filesystem::path fullPath = std::filesystem::absolute(workingDirectory / path.C_Str());
	//		AssetManager::RequestAsyncLoadFromFile(fullPath, destinationDirectory,
	//			[&mat](AssetHandle assetHandle)
	//			{
	//				if (assetHandle.IsValid())
	//					mat.SetAlbedoTexture(assetHandle);
	//				else
	//					mat.SetAlbedoTexture(AssetManager::GetInvalidTextureHandle());
	//			});
	//	}
	//	else
	//	{
	//		aiColor4D baseColor(0.f, 0.f, 0.f, 0.0f);
	//		if (m->Get(AI_MATKEY_COLOR_DIFFUSE, baseColor) == AI_SUCCESS)
	//		{
	//			mat.m_AlbedoColor = { baseColor.r, baseColor.g , baseColor.b, baseColor.a };
	//		}
	//	}
	//	if (m->GetTexture(aiTextureType_METALNESS, 0, &path) == aiReturn::aiReturn_SUCCESS)
	//	{
	//		std::filesystem::path fullPath = std::filesystem::absolute(workingDirectory / path.C_Str());
	//
	//		TextureImportSettings importSettings;
	//		importSettings.IsSRGB = false;
	//
	//		AssetManager::RequestAsyncLoadFromFile(fullPath, destinationDirectory,
	//			[&mat](AssetHandle assetHandle)
	//			{
	//				if (assetHandle.IsValid())
	//					mat.SetMetallicTexture(assetHandle);
	//			}, importSettings);
	//	}
	//
	//	if (m->GetTexture(aiTextureType::aiTextureType_DIFFUSE_ROUGHNESS, 0, &path) == aiReturn::aiReturn_SUCCESS)
	//	{
	//		std::filesystem::path fullPath = std::filesystem::absolute(workingDirectory / path.C_Str());
	//
	//		TextureImportSettings importSettings;
	//		importSettings.IsSRGB = false;
	//
	//		AssetManager::RequestAsyncLoadFromFile(fullPath, destinationDirectory,
	//			[&mat](AssetHandle assetHandle)
	//			{
	//				if (assetHandle.IsValid())
	//					mat.SetRoughnessTexture(assetHandle);
	//			}, importSettings);
	//	}
	//	
	//	if (m->GetTexture(aiTextureType::aiTextureType_NORMALS, 0, &path) == aiReturn::aiReturn_SUCCESS)
	//	{
	//		std::filesystem::path fullPath = std::filesystem::absolute(workingDirectory / path.C_Str());
	//
	//		TextureImportSettings importSettings;
	//		importSettings.IsSRGB = false;
	//
	//		AssetManager::RequestAsyncLoadFromFile(fullPath, destinationDirectory,
	//			[&mat](AssetHandle assetHandle)
	//			{
	//				if (assetHandle.IsValid())
	//					mat.SetNormalMap(assetHandle);
	//			}, importSettings);
	//	}
	//	else if (m->GetTexture(aiTextureType::aiTextureType_NORMAL_CAMERA, 0, &path) == aiReturn::aiReturn_SUCCESS)
	//	{
	//		std::filesystem::path fullPath = std::filesystem::absolute(workingDirectory / path.C_Str());
	//
	//		TextureImportSettings importSettings;
	//		importSettings.IsSRGB = false;
	//
	//		AssetManager::RequestAsyncLoadFromFile(fullPath, destinationDirectory,
	//			[&mat](AssetHandle assetHandle)
	//			{
	//				if (assetHandle.IsValid())
	//					mat.SetNormalMap(assetHandle);
	//			}, importSettings);
	//	}
	//	if (m->GetTexture(aiTextureType::aiTextureType_AMBIENT_OCCLUSION, 0, &path) == aiReturn::aiReturn_SUCCESS)
	//	{
	//		std::filesystem::path fullPath = std::filesystem::absolute(workingDirectory / path.C_Str());
	//
	//		AssetManager::RequestAsyncLoadFromFile(fullPath, destinationDirectory,
	//			[&mat](AssetHandle assetHandle)
	//			{
	//				if (assetHandle.IsValid())
	//					mat.SetAmbientOcclusionTexture(assetHandle);
	//			});
	//	}
	//	if (m->GetTexture(aiTextureType::aiTextureType_EMISSION_COLOR, 0, &path) == aiReturn::aiReturn_SUCCESS)
	//	{
	//		std::filesystem::path fullPath = std::filesystem::absolute(workingDirectory / path.C_Str());
	//
	//		TextureImportSettings importSettings;
	//		importSettings.IsSRGB = false;
	//
	//		AssetManager::RequestAsyncLoadFromFile(fullPath, destinationDirectory,
	//			[&mat](AssetHandle assetHandle)
	//			{
	//				if (assetHandle.IsValid())
	//				{
	//					mat.SetEmissionTexture(assetHandle);
	//					mat.m_EmissionIntensity = 1.0f;
	//				}
	//			}, importSettings);
	//	}
	//	else
	//	{
	//		aiColor4D emissionColor(0.f, 0.f, 0.f, 0.0f);
	//		if (m->Get(AI_MATKEY_COLOR_EMISSIVE, emissionColor) == AI_SUCCESS)
	//		{
	//			mat.m_EmissionColor = { emissionColor.r, emissionColor.g, emissionColor.b, emissionColor.a };
	//			if (emissionColor.r > 0.0f || emissionColor.g > 0.0f || emissionColor.b > 0.0f)
	//			{
	//				mat.m_EmissionIntensity = 1.0f;
	//			}
	//		}
	//	}
	//	if (m->GetTexture(aiTextureType::aiTextureType_DISPLACEMENT, 0, &path) == aiReturn::aiReturn_SUCCESS)
	//	{
	//		std::filesystem::path fullPath = std::filesystem::absolute(workingDirectory / path.C_Str());
	//
	//		TextureImportSettings importSettings;
	//		importSettings.IsSRGB = false;
	//
	//		AssetManager::RequestAsyncLoadFromFile(fullPath, destinationDirectory,
	//			[&mat](AssetHandle assetHandle)
	//			{
	//				if (assetHandle.IsValid())
	//					mat.SetHeightMap(assetHandle);
	//			}, importSettings);
	//	}
	//
	//	if (!destinationDirectory.empty())
	//	{
	//		const std::string fullDestinationFilepath = destinationDirectory + "\\" + std::string(m->GetName().C_Str()) + ".rasset";
	//		Serializer::Serialize<Material>(materialHandle, fullDestinationFilepath);
	//	}
	//
	//	return materialHandle;
	//}

	//AssetHandle Importer::ProcessMesh(aiMesh* pMesh, const std::filesystem::path& workingDirectory, const std::string& destinationDirectory) noexcept
	//{
	//	RLS_ASSERT(pMesh, "Assimp data is invalid.");
	//	RLS_ASSERT(pMesh->HasPositions(), "Mesh contains no position data.");
	//	RLS_ASSERT(pMesh->HasFaces(), "Mesh contains no faces data.");
	//	RLS_ASSERT(pMesh->HasNormals(), "Mesh contains no normal data.");
	//	RLS_ASSERT(pMesh->HasTangentsAndBitangents(), "Mesh contains no tangent and/or bitangent data.");
	//	RLS_ASSERT(pMesh->HasTextureCoords(0u), "Mesh contains no texture coordinate data.");
	//
	//	const std::string fullDestinationPath = destinationDirectory + "\\" + std::string(pMesh->mName.C_Str()) + ".rasset";
	//	if (AssetManager::IsLoaded(fullDestinationPath))
	//	{
	//		return AssetManager::GetHandleByPath(fullDestinationPath);
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
	//	VertexBuffer::Specification vbSpec
	//	{
	//		.NrOfVertices = (uint32_t)vertices.size(),
	//		.TotalSizeInBytes = (uint32_t)vertices.size() * sizeof(SimpleVertex),
	//		.Stride = sizeof(SimpleVertex),
	//		.pBuffer = (void*)vertices.data(),
	//		.Name = pMesh->mName.C_Str() + std::string(" Vertex Buffer")
	//	};
	//
	//	IndexBuffer::Specification ibSpec
	//	{
	//		.NrOfIndices = (uint32_t)indices.size(),
	//		.TotalSizeInBytes = (uint32_t)indices.size() * sizeof(uint32_t),
	//		.Stride = sizeof(uint32_t),
	//		.pBuffer = (void*)indices.data(),
	//		.Name = pMesh->mName.C_Str() + std::string(" Index Buffer")
	//	};
	//
	//	AssetHandle meshHandle = AssetManager::CreateNew<Mesh>();
	//	Mesh& mesh = AssetManager::Get<Mesh>(meshHandle);
	//	mesh.SetName(pMesh->mName.C_Str());
	//	mesh.SetVertexBuffer(std::make_unique<VertexBuffer>(&vbSpec));
	//	mesh.SetIndexBuffer(std::make_unique<IndexBuffer>(&ibSpec));
	//
	//	if (!destinationDirectory.empty())
	//	{
	//		Serializer::Serialize<Mesh>(meshHandle, fullDestinationPath);
	//	}
	//
	//	RLS_CORE_INFO("Loaded mesh '{0}' with GUID: {1}", pMesh->mName.C_Str(), ConvertUUIDToString(meshHandle.Uuid));
	//
	//	return meshHandle;
	//}
}