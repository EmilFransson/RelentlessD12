#include "Assets/AssetManager.h"
#include "Core/Application.h"
#include "File/File.h"
#include "Graphics/Resources/Texture.h"
#include "Graphics/Renderer/RenderCommand.h"
#include "ImportSettings.h"
#include "Mesh/Mesh.h"
#include "Mesh/Vertex.h"
#include "Serializer.h"
#include "Utility/Common.h"
#include "Utility/SerializeUtilities.h"
#include "Utility/FilepathUtils.h"



namespace Relentless
{
	using namespace Microsoft::WRL;

	std::unordered_map<AssetType, std::function<bool(const std::string&, AssetHandle&)>> DeserializerFuncs = {
		{AssetType::Texture2D, [](const std::string& path, AssetHandle& outHandle) { return Serializer::Deserialize<Texture2D>(path, outHandle); }},
		{AssetType::Material, [](const std::string& path, AssetHandle& outHandle) { return Serializer::Deserialize<Material>(path, outHandle); }},
		{AssetType::Mesh, [](const std::string& path, AssetHandle& outHandle) { return Serializer::Deserialize<Mesh>(path, outHandle); }}
	};

	std::unordered_map<AssetType, std::function<bool(const std::string&, const AssetHandle&)>> SerializerFuncs = {
		{AssetType::Texture2D, [](const std::string& path, const AssetHandle& assetHandle) { return Serializer::Serialize<Texture2D>(assetHandle, path); }},
		{AssetType::Material, [](const std::string& path, const AssetHandle& assetHandle) { return Serializer::Serialize<Material>(assetHandle, path); }},
		{AssetType::Mesh, [](const std::string& path, const AssetHandle& assetHandle) { return Serializer::Serialize<Mesh>(assetHandle, path); }}
	};

	struct Texture2DSerializationContext
	{
		std::string Path = "?";
	};

	struct Data
	{
		std::unordered_map<UUID, Texture2DSerializationContext> UUIDToStagingResources;
	};

	static Data s_Data;

	#pragma pack(push, 1)
	struct MaterialData
	{
		uint8_t RenderMode;
		DirectX::XMFLOAT4 AlbedoColor;
		float Metallic;
		DirectX::XMFLOAT4 EmissionColor;
		float EmissionIntensity;
		float Roughness;
		DirectX::XMFLOAT2 TilingFactor;
		DirectX::XMFLOAT2 Offset;
		float HeightScale;
		float AOScale;
		uint32_t CombinedRoughnessMetallness;
		UUID AlbedoTextureUUID{ NULL_UUID };
		UUID MetallicTextureUUID{ NULL_UUID };
		UUID RoughnessTextureUUID{ NULL_UUID };
		UUID NormalMapUUID{ NULL_UUID };
		UUID HeightMapUUID{ NULL_UUID };
		UUID AmbientOcclusionTextureUUID{ NULL_UUID };
		UUID EmissionTextureUUID{ NULL_UUID };
	};
	#pragma pack(pop)

#pragma pack(push, 1)
	struct TextureHeader
	{
		uint32_t BaseWidth;
		uint32_t BaseHeight;
		uint32_t NrOfMips;
		uint32_t Samples;
		DXGI_FORMAT Format;
		bool IsSRGB;
	};
#pragma pack(pop)

	template<>
	bool Serializer::Serialize<Material>(const AssetHandle& assetHandle, const std::filesystem::path& filepath) noexcept
	{
		Material& material = AssetManager::Get<Material>(assetHandle);
		
		MaterialData data{};
		data.RenderMode = static_cast<uint8_t>(material.GetRenderMode());
		data.AlbedoColor = material.m_AlbedoColor;
		data.Metallic = material.m_Metallic;
		data.EmissionColor = material.m_EmissionColor;
		data.EmissionIntensity = material.m_EmissionIntensity;
		data.Roughness = material.m_Roughness;
		data.TilingFactor = material.m_TilingFactor;
		data.Offset = material.m_Offset;
		data.HeightScale = material.m_HeightScale;
		data.AOScale = material.m_AOScale;
		data.CombinedRoughnessMetallness = material.m_CombinedRoughnessMetallnesMap;

		data.AlbedoTextureUUID = material.m_AlbedoTextureHandle.Uuid;
		data.MetallicTextureUUID = material.m_MetallicTextureHandle.Uuid;
		data.RoughnessTextureUUID = material.m_RoughnessTextureHandle.Uuid;
		data.NormalMapUUID = material.m_NormalMapHandle.Uuid;
		data.HeightMapUUID = material.m_HeightMapHandle.Uuid;
		data.AmbientOcclusionTextureUUID = material.m_AmbientOcclusionTextureHandle.Uuid;
		data.EmissionTextureUUID = material.m_EmissionTextureHandle.Uuid;

		std::ofstream outFile(filepath, std::ios::binary);
		RLS_ASSERT(outFile.is_open(), "[Serializer]: Failed to open output file.");

		SerializeRassetHeader(assetHandle, outFile);
		SerializeAssetTags(assetHandle, outFile);

		outFile.write(reinterpret_cast<char*>(&data), sizeof(data));

		outFile.close();

		return true;
	}

	[[nodiscard]] static uint32_t GetBytesPerPixel(const DXGI_FORMAT format) noexcept 
	{
		switch (format)
		{
		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_B8G8R8A8_UNORM:
			return 4;
		case DXGI_FORMAT_R32G32B32A32_FLOAT:
			return 16;
		case DXGI_FORMAT_R16G16B16A16_FLOAT:
			return 8;
		default:
			RLS_ASSERT(false, "Unknown DXGI format encountered.");
			return 0;
		}
	}

	bool Serializer::Deserialize(const std::filesystem::path& filepath, AssetHandle& outHandle) noexcept
	{
		const std::filesystem::path absolutePath = FilepathUtils::Combine(EDITOR_ASSET_DIRECTORY, filepath);
		if (!File::Exists(absolutePath))
		{
			outHandle = NULL_HANDLE;
			return false;
		}

		const std::string extension = FilepathUtils::ExtractExtension(absolutePath);
		if (extension != ASSET_EXTENSION)
		{
			outHandle = NULL_HANDLE;
			return false;
		}

		const AssetType assetType = AssetRegistry::GetMetaData(absolutePath).AssetType;
		return DeserializerFuncs[assetType](absolutePath.string(), outHandle);
	}

	std::pair<uint32_t, uint8_t> Serializer::DeserializeSignatureAndVersion(std::ifstream& ifstream) noexcept
	{
		RLS_ASSERT(ifstream.is_open(), "[Serializer]: File stream is invalid.");

		uint32_t signature = 0u;
		uint8_t version = 0u;
		ifstream.read(reinterpret_cast<char*>(&signature), sizeof(signature));
		ifstream.read(reinterpret_cast<char*>(&version), sizeof(version));
		ifstream.seekg(std::ios_base::beg);
		return { signature, version };
	}

	bool Serializer::Serialize(const std::filesystem::path& filepath, const AssetHandle& assetHandle) noexcept
	{
		if (!assetHandle.IsValid())
			return false;

		std::filesystem::path absolutePath = FilepathUtils::Combine(EDITOR_ASSET_DIRECTORY, filepath);
	
		const bool hasExtension = FilepathUtils::HasExtension(filepath);
		if (!hasExtension)
			FilepathUtils::SetExtension(absolutePath, ASSET_EXTENSION);
		
		if (hasExtension)
		{
			const std::string extension = FilepathUtils::ExtractExtension(absolutePath);
			if(extension != ASSET_EXTENSION)
				return false;
		}

		if (!File::ExistsDir(absolutePath))
			return false;

		return SerializerFuncs[assetHandle.Type](absolutePath.string(), assetHandle);
	}

	template<>
	bool Serializer::Serialize<Texture2D>(const AssetHandle& assetHandle, const std::filesystem::path& filepath) noexcept
	{
		Texture2D& texture = AssetManager::Get<Texture2D>(assetHandle);
		const ComPtr<ID3D12Resource> gpuInterface = texture.GetInterface();
		const D3D12_RESOURCE_DESC textureDescriptor = gpuInterface->GetDesc();

		GPUTaskManager& gpuTaskManager = Application::Get().GetGPUTaskManager();
		ComPtr<ID3D12GraphicsCommandList4> pCopyCommandList = gpuTaskManager.RequestCommandList(CommandType::Copy);
		ComPtr<ID3D12GraphicsCommandList4> pDirectCommandList = gpuTaskManager.RequestCommandList(CommandType::Direct);

		D3D12_RESOURCE_BARRIER resourceTransitionBarrier{};
		resourceTransitionBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		resourceTransitionBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		resourceTransitionBarrier.Transition.pResource = texture.GetInterface().Get();
		resourceTransitionBarrier.Transition.StateBefore = texture.GetCurrentState();
		resourceTransitionBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
		resourceTransitionBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		DXCall_STD(pDirectCommandList->ResourceBarrier(1u, &resourceTransitionBarrier));

		gpuTaskManager.ExecuteCommandListBlocking(pDirectCommandList);

		const uint32_t mipLevels = textureDescriptor.MipLevels;
		Texture2DSerializationContext context = {};
		context.Path = filepath.string();
		std::vector<ComPtr<ID3D12Resource>> stagingResources(mipLevels);
		for (uint32_t mipLevel = 0u; mipLevel < mipLevels; ++mipLevel)
		{
			D3D12_HEAP_PROPERTIES heapProperties = {};
			heapProperties.Type = D3D12_HEAP_TYPE_READBACK;
			heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			heapProperties.CreationNodeMask = 1;
			heapProperties.VisibleNodeMask = 1;

			D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};
			uint64_t mipSize = 0u;
			DXCall_STD(D3D12Core::GetDevice()->GetCopyableFootprints(&textureDescriptor, mipLevel, 1, 0, &footprint, nullptr, nullptr, &mipSize));

			// Create buffer resource description for staging
			D3D12_RESOURCE_DESC bufferDesc = {};
			bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			bufferDesc.Width = mipSize;
			bufferDesc.Height = 1;
			bufferDesc.DepthOrArraySize = 1;
			bufferDesc.MipLevels = 1;
			bufferDesc.SampleDesc.Count = 1;
			bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

			DXCall(D3D12Core::GetDevice()->CreateCommittedResource(
				&heapProperties,
				D3D12_HEAP_FLAG_NONE,
				&bufferDesc,
				D3D12_RESOURCE_STATE_COPY_DEST,
				nullptr,
				IID_PPV_ARGS(&stagingResources[mipLevel])));

			// Copy the mip level to the staging resource
			D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
			srcLocation.pResource = gpuInterface.Get();
			srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
			srcLocation.SubresourceIndex = mipLevel;

			D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
			dstLocation.pResource = stagingResources[mipLevel].Get();
			dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
			dstLocation.PlacedFootprint = footprint;
		
			DXCall_STD(pCopyCommandList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr));
		}
		gpuTaskManager.ExecuteCommandListBlocking(pCopyCommandList);

		ComPtr<ID3D12GraphicsCommandList4> pTransitionCommandList = gpuTaskManager.RequestCommandList(CommandType::Direct);

		resourceTransitionBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		resourceTransitionBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		resourceTransitionBarrier.Transition.pResource = texture.GetInterface().Get();
		resourceTransitionBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
		resourceTransitionBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		resourceTransitionBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		DXCall_STD(pTransitionCommandList->ResourceBarrier(1u, &resourceTransitionBarrier));

		s_Data.UUIDToStagingResources[assetHandle.Uuid] = context;
		gpuTaskManager.ExecuteCommandListBlocking(pTransitionCommandList);

		//.........................................................//

		std::ofstream outFile(s_Data.UUIDToStagingResources[assetHandle.Uuid].Path, std::ios::binary);
		if (!outFile.is_open())
		{
			RLS_CORE_ERROR("[Serializer]: Unable to serialize Texture2D to path '{0}'; Unable to open output file.", s_Data.UUIDToStagingResources[assetHandle.Uuid].Path.c_str());
			return false;
		}

		SerializeRassetHeader(assetHandle, outFile);
		SerializeAssetTags(assetHandle, outFile);

		TextureHeader textureHeader
		{
			.BaseWidth = texture.GetWidth(),
			.BaseHeight = texture.GetHeight(),
			.NrOfMips = mipLevels,
			.Samples = texture.GetMultiSampleCount(),
			.Format = texture.GetFormat(),
			.IsSRGB = texture.IsSRGB()
		};
		outFile.write(reinterpret_cast<char*>(&textureHeader), sizeof(textureHeader));

		// Process each mip level
		for (uint32_t mipLevel = 0; mipLevel < mipLevels; ++mipLevel)
		{
			ComPtr<ID3D12Resource> pStagingResource = stagingResources[mipLevel];

			D3D12_PLACED_SUBRESOURCE_FOOTPRINT mipFootprint = {};
			uint64_t mipSize = 0u;

			DXCall_STD(D3D12Core::GetDevice()->GetCopyableFootprints
			(
				&textureDescriptor,
				mipLevel,
				1u,
				0u,
				&mipFootprint,
				nullptr,
				nullptr,
				&mipSize
			));

			// Map the staging resource
			BYTE* pData = nullptr;
			D3D12_RANGE readRange = { 0, mipSize };
			DXCall(pStagingResource->Map(0, &readRange, reinterpret_cast<void**>(&pData)));

			outFile.write(reinterpret_cast<const char*>(pData), mipSize);

			DXCall_STD(pStagingResource->Unmap(0, nullptr));
		}
		outFile.close();

		return true;
	}

	template<>
	bool Serializer::Serialize<Mesh>(const AssetHandle& assetHandle, const std::filesystem::path& filepath) noexcept
	{
		std::ofstream outFile(filepath, std::ios::binary);

		SerializeRassetHeader(assetHandle, outFile);
		SerializeAssetTags(assetHandle, outFile);

		const Mesh& mesh = AssetManager::Get<Mesh>(assetHandle);
		const std::unique_ptr<VertexBuffer>& pVertexBuffer = mesh.GetVertexBuffer();
		const std::unique_ptr<IndexBuffer>& pIndexBuffer = mesh.GetIndexBuffer();

		VertexBuffer::Specification& vertexBufferSpecification = pVertexBuffer->GetSpecification();
		IndexBuffer::Specification& indexBufferSpecification = pIndexBuffer->GetSpecification();

		const D3D12_RESOURCE_DESC vertexBufferResourceDescriptor = pVertexBuffer->GetInterface()->GetDesc();
		const uint64_t vertexBufferSizeInBytes = vertexBufferResourceDescriptor.Width;

		MeshDataHeader meshDataHeader
		{
			.VertexBufferSizeInBytes = vertexBufferSizeInBytes,
			.IndexBufferSizeInBytes = indexBufferSpecification.TotalSizeInBytes,
			.VertexCount = vertexBufferSpecification.NrOfVertices,
			.IndexCount = indexBufferSpecification.NrOfIndices
		};

		//Write the mesh header:
		outFile.write(reinterpret_cast<char*>(&meshDataHeader), sizeof(meshDataHeader));

		{
			Microsoft::WRL::ComPtr<ID3D12Resource> readbackBuffer = nullptr;

			D3D12_RESOURCE_DESC readbackDesc = {};
			readbackDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			readbackDesc.Alignment = 0;
			readbackDesc.Width = vertexBufferSizeInBytes;
			readbackDesc.Height = 1;
			readbackDesc.DepthOrArraySize = 1;
			readbackDesc.MipLevels = 1;
			readbackDesc.Format = DXGI_FORMAT_UNKNOWN;
			readbackDesc.SampleDesc.Count = 1;
			readbackDesc.SampleDesc.Quality = 0;
			readbackDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			readbackDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

			D3D12_HEAP_PROPERTIES readbackHeapProps = {};
			readbackHeapProps.Type = D3D12_HEAP_TYPE_READBACK;
			readbackHeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			readbackHeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

			// Create the readback buffer
			DXCall(D3D12Core::GetDevice()->CreateCommittedResource(
				&readbackHeapProps,
				D3D12_HEAP_FLAG_NONE,
				&readbackDesc,
				D3D12_RESOURCE_STATE_COPY_DEST,
				nullptr,
				IID_PPV_ARGS(&readbackBuffer)));

			ComPtr<ID3D12GraphicsCommandList4> copyCommandList = Application::Get().GetGPUTaskManager().RequestCommandList(CommandType::Copy);

			DXCall_STD(copyCommandList->CopyResource(readbackBuffer.Get(), pVertexBuffer->GetInterface().Get()));

			Application::Get().GetGPUTaskManager().ExecuteCommandListBlocking(copyCommandList);

			UINT8* pData = nullptr;
			DXCall(readbackBuffer->Map(0, nullptr, reinterpret_cast<void**>(&pData)));

			outFile.write(reinterpret_cast<char*>(pData), vertexBufferSizeInBytes);

			DXCall_STD(readbackBuffer->Unmap(0, nullptr));
		}
		
		const D3D12_RESOURCE_DESC indexBufferResourceDescriptor = pIndexBuffer->GetInterface()->GetDesc();
		const uint64_t indexBufferSizeInBytes = indexBufferResourceDescriptor.Width;

		{
			Microsoft::WRL::ComPtr<ID3D12Resource> readbackBuffer = nullptr;

			D3D12_RESOURCE_DESC readbackDesc = {};
			readbackDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			readbackDesc.Alignment = 0;
			readbackDesc.Width = indexBufferSizeInBytes;
			readbackDesc.Height = 1;
			readbackDesc.DepthOrArraySize = 1;
			readbackDesc.MipLevels = 1;
			readbackDesc.Format = DXGI_FORMAT_UNKNOWN;
			readbackDesc.SampleDesc.Count = 1;
			readbackDesc.SampleDesc.Quality = 0;
			readbackDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			readbackDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

			D3D12_HEAP_PROPERTIES readbackHeapProps = {};
			readbackHeapProps.Type = D3D12_HEAP_TYPE_READBACK;
			readbackHeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			readbackHeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

			// Create the readback buffer
			DXCall(D3D12Core::GetDevice()->CreateCommittedResource(
				&readbackHeapProps,
				D3D12_HEAP_FLAG_NONE,
				&readbackDesc,
				D3D12_RESOURCE_STATE_COPY_DEST,
				nullptr,
				IID_PPV_ARGS(&readbackBuffer)));

			ComPtr<ID3D12GraphicsCommandList4> copyCommandList = Application::Get().GetGPUTaskManager().RequestCommandList(CommandType::Copy);

			DXCall_STD(copyCommandList->CopyResource(readbackBuffer.Get(), pIndexBuffer->GetInterface().Get()));

			Application::Get().GetGPUTaskManager().ExecuteCommandListBlocking(copyCommandList);

			UINT8* pData = nullptr;
			DXCall(readbackBuffer->Map(0, nullptr, reinterpret_cast<void**>(&pData)));

			outFile.write(reinterpret_cast<char*>(pData), indexBufferSizeInBytes);

			DXCall_STD(readbackBuffer->Unmap(0, nullptr));
		}
		return true;
	}

	template<>
	bool Serializer::Deserialize<Mesh>(const std::filesystem::path& filepath, AssetHandle& outHandle) noexcept
	{
		if (!File::Exists(filepath))
		{
			RLS_CORE_ERROR("[Serializer]: Mesh file with path '{0}' does not exist.", filepath.string().c_str());
			outHandle = NULL_HANDLE;
			return false;
		}
		if (FilepathUtils::ExtractExtension(filepath) != ASSET_EXTENSION)
		{
			RLS_CORE_ERROR("[Serializer]: File with path '{0}' not of correct rasset extension (.rasset).", filepath.string().c_str());
			outHandle = NULL_HANDLE;
			return false;
		}

		std::ifstream inFile(filepath, std::ios::binary);
		RLS_ASSERT(inFile.is_open(), "[Serializer]: Unable to open file with path '{0}'", filepath);

		auto [signature, headerVersion] = DeserializeHeaderSignatureAndVersion(inFile);
		if (signature != RASSET_SIGNATURE)
		{
			RLS_CORE_ERROR("Unable to load asset file with path '{0}'. Asset is either invalid or corrupt (Signature invalid).", filepath.string().c_str());
			outHandle = NULL_HANDLE;
			return false;
		}

		AssetType type = AssetType::Undefined;
		UUID uuid = NULL_UUID;
		std::string name = "";
		uint16_t tagsByteSize = 0u;

		switch (headerVersion)
		{
		case 1:
		{
			const RassetHeader_1 header = DeserializeRAssetHeaderVersion1(inFile);
			type = header.AssetType;
			uuid = header.UUID;
			name = header.Name;
			tagsByteSize = header.TagsByteSize;
			break;
		}
		default:
			RLS_CORE_ERROR("Unable to load asset file with path {0}. Asset version '{1}' does not support deserialization.", filepath.string().c_str(), std::to_string(headerVersion).c_str());
			outHandle = NULL_HANDLE;
			return false;
		}

		RLS_ASSERT(type == AssetType::Mesh, "Asset is not of type mesh.");

		//Move past eventual tag data:
		inFile.seekg(tagsByteSize, std::ios_base::cur);

		MeshDataHeader readMeshHeader;
		inFile.read(reinterpret_cast<char*>(&readMeshHeader), sizeof(readMeshHeader));
	
		std::vector<char> readVertexData(readMeshHeader.VertexBufferSizeInBytes);
		std::vector<char> readIndexData(readMeshHeader.IndexBufferSizeInBytes);
	
		inFile.read(reinterpret_cast<char*>(readVertexData.data()), readMeshHeader.VertexBufferSizeInBytes);
		inFile.read(reinterpret_cast<char*>(readIndexData.data()), readMeshHeader.IndexBufferSizeInBytes);
	
		VertexBuffer::Specification vbSpec
		{
			.NrOfVertices = (uint32_t)readMeshHeader.VertexCount,
			.TotalSizeInBytes = (uint32_t)readMeshHeader.VertexBufferSizeInBytes,
			.Stride = sizeof(SimpleVertex),
			.pBuffer = (void*)readVertexData.data(),
			.Name = name + std::string(" Vertex Buffer")
		};
	
		IndexBuffer::Specification ibSpec
		{
			.NrOfIndices = (uint32_t)readMeshHeader.IndexCount,
			.TotalSizeInBytes = (uint32_t)readMeshHeader.IndexBufferSizeInBytes,
			.Stride = sizeof(uint32_t),
			.pBuffer = (void*)readIndexData.data(),
			.Name = name + std::string(" Index Buffer")
		};
	
		AssetHandle meshHandle = AssetManager::CreateNew<Mesh>(uuid, filepath.string());
		Mesh& mesh = AssetManager::Get<Mesh>(meshHandle);
		mesh.SetName(name);
		mesh.SetVertexBuffer(std::make_unique<VertexBuffer>(&vbSpec));
		mesh.SetIndexBuffer(std::make_unique<IndexBuffer>(&ibSpec));

		RLS_CORE_INFO("[Serializer]: Loaded mesh '{0}' with GUID: {1}", name.c_str(), ConvertUUIDToString(uuid).c_str());
		
		outHandle = meshHandle;
		return true;
	}

	template<>
	bool Serializer::Deserialize<Texture2D>(const std::filesystem::path& filepath, AssetHandle& outHandle) noexcept
	{
		std::ifstream inFile(filepath, std::ios::binary);
		RLS_ASSERT(inFile.is_open(), "[Serializer]: Unable to open file with path '{0}'", filepath);

		//auto [signature, headerVersion] = DeserializeHeaderSignatureAndVersion(inFile);
		//if (signature != LatestRassetHeaderVersion::Signature)
		//{
		//	RLS_CORE_ERROR("Unable to load asset file with path '{0}'. Asset is either invalid or corrupt.", filepath.c_str());
		//	outHandle = NULL_HANDLE;
		//	return false;
		//}

		AssetType type = AssetType::Undefined;
		UUID uuid = NULL_UUID;
		std::string name = "";
		uint16_t tagsByteSize = 0u;

		//switch (headerVersion)
		//{
		//case 1:
		//{
			const RassetHeader_1 header = DeserializeRAssetHeaderVersion1(inFile);
			type = header.AssetType;
			uuid = header.UUID;
			name = header.Name;
			tagsByteSize = header.TagsByteSize;
		//	break;
		//}
		//default:
		//	RLS_CORE_ERROR("Unable to load asset file with path {0}. Asset version '{1}' does not support deserialization.", filepath.c_str(), std::to_string(headerVersion).c_str());
		//	outHandle = AssetManager::GetInvalidTextureHandle();
		//	return false;
		//}

		RLS_ASSERT(type == AssetType::Texture2D, "[Serializer]: Asset type mismatch encountered.");

		//Move past eventual tag data:
		inFile.seekg(tagsByteSize, std::ios_base::cur);

		TextureHeader textureHeader = {};
		inFile.read(reinterpret_cast<char*>(&textureHeader), sizeof(TextureHeader));

		D3D12_RESOURCE_DESC textureDescriptor = {};
		textureDescriptor.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		textureDescriptor.Format = textureHeader.Format;
		textureDescriptor.MipLevels = textureHeader.NrOfMips;
		textureDescriptor.Width = textureHeader.BaseWidth;
		textureDescriptor.Height = textureHeader.BaseHeight;
		textureDescriptor.SampleDesc.Count = textureHeader.Samples;
		textureDescriptor.SampleDesc.Quality = 0u;
		textureDescriptor.DepthOrArraySize = 1u;
		textureDescriptor.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		textureDescriptor.Flags = D3D12_RESOURCE_FLAG_NONE;
		textureDescriptor.Alignment = 0u;

		D3D12_HEAP_PROPERTIES textureHeapProperties = {};
		textureHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		textureHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		textureHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
		textureHeapProperties.CreationNodeMask = 1u;
		textureHeapProperties.VisibleNodeMask = 1u;

		Microsoft::WRL::ComPtr<ID3D12Resource> pTextureResource = nullptr;

		DXCall(D3D12Core::GetDevice()->CreateCommittedResource
		(
			&textureHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&textureDescriptor,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&pTextureResource)
		));

		NAME_D12_OBJECT(pTextureResource, ConvertStringToWstring(name).c_str());

		std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> footprints(textureHeader.NrOfMips);
		uint64_t totalSize = 0;

		DXCall_STD(D3D12Core::GetDevice()->GetCopyableFootprints
		(
			&textureDescriptor,
			0,
			textureHeader.NrOfMips,
			0,
			footprints.data(),
			nullptr,
			nullptr,
			&totalSize)
		);

		D3D12_RESOURCE_DESC bufferDesc = {};
		bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		bufferDesc.Alignment = 0; 
		bufferDesc.Width = totalSize;
		bufferDesc.Height = 1;  
		bufferDesc.DepthOrArraySize = 1; 
		bufferDesc.MipLevels = 1;
		bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
		bufferDesc.SampleDesc.Count = 1; 
		bufferDesc.SampleDesc.Quality = 0;
		bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		D3D12_HEAP_PROPERTIES heapProperties = {};
		heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
		heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProperties.CreationNodeMask = 1u;
		heapProperties.VisibleNodeMask = 1u;

		Microsoft::WRL::ComPtr<ID3D12Resource> pUploadBuffer = nullptr;
		DXCall(D3D12Core::GetDevice()->CreateCommittedResource
		(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&bufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&pUploadBuffer))
		);
		NAME_D12_OBJECT(pUploadBuffer, ConvertStringToWstring("Texture2D Upload Buffer - " + name).c_str());

		void* pData = nullptr;
		DXCall(pUploadBuffer->Map(0u, nullptr, &pData));
		RLS_ASSERT(pData, "[Serializer]: Data pointer is invalid.");

		for (uint32_t mipLevel = 0u; mipLevel < textureHeader.NrOfMips; ++mipLevel)
		{
			D3D12_PLACED_SUBRESOURCE_FOOTPRINT mipFootprint = {};
			uint64_t mipSize = 0u;
			
			DXCall_STD(D3D12Core::GetDevice()->GetCopyableFootprints
			(
				&textureDescriptor,
				mipLevel,
				1u,
				0u,
				&mipFootprint,
				nullptr,
				nullptr,
				&mipSize
			));

			std::vector<BYTE> mipData(mipSize);
			inFile.read(reinterpret_cast<char*>(mipData.data()), mipSize);

			BYTE* dest = static_cast<BYTE*>(pData) + footprints[mipLevel].Offset;
			memcpy(dest, mipData.data(), mipSize);
		}

		DXCall_STD(pUploadBuffer->Unmap(0u, nullptr));

		GPUTaskManager& gpuTaskManager = Application::Get().GetGPUTaskManager();
		ComPtr<ID3D12GraphicsCommandList4> pCopyCommandList = gpuTaskManager.RequestCommandList(CommandType::Direct);
		
		for (uint32_t mipLevel = 0u; mipLevel < textureHeader.NrOfMips; ++mipLevel)
		{
			D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
			srcLocation.pResource = pUploadBuffer.Get();
			srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
			srcLocation.PlacedFootprint = footprints[mipLevel];

			D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
			dstLocation.pResource = pTextureResource.Get();
			dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
			dstLocation.SubresourceIndex = mipLevel;

			DXCall_STD(pCopyCommandList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr));
		}

		D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDescriptor = {};
		shaderResourceViewDescriptor.Format = textureHeader.Format;
		shaderResourceViewDescriptor.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDescriptor.Texture2D.MipLevels = textureHeader.NrOfMips;
		shaderResourceViewDescriptor.Texture2D.MostDetailedMip = 0;
		shaderResourceViewDescriptor.Texture2D.ResourceMinLODClamp = 0.0f;
		shaderResourceViewDescriptor.Texture2D.PlaneSlice = 0u;
		shaderResourceViewDescriptor.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		Texture2DSpecification specification;
		specification.DescriptorHandleSRV = MemoryManager::Get().CreateDescriptorHandle(DescriptorHandleType::SRV);
		DXCall_STD(D3D12Core::GetDevice()->CreateShaderResourceView(pTextureResource.Get(), &shaderResourceViewDescriptor, specification.DescriptorHandleSRV.CPUHandle));

		D3D12_RESOURCE_BARRIER resourceTransitionBarrier{};
		resourceTransitionBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		resourceTransitionBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		resourceTransitionBarrier.Transition.pResource = pTextureResource.Get();
		resourceTransitionBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		resourceTransitionBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		resourceTransitionBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		DXCall_STD(pCopyCommandList->ResourceBarrier(1u, &resourceTransitionBarrier));

		specification.pResource = pTextureResource;
		specification.Name = name;
		specification.Width = textureHeader.BaseWidth;
		specification.Height = textureHeader.BaseHeight;
		specification.Format = textureHeader.Format;
		specification.MipCount = textureHeader.NrOfMips;
		specification.IsSRGB = textureHeader.IsSRGB;
		specification.SampleCount = textureHeader.Samples;
		
		const AssetHandle handle = AssetManager::CreateNew<Texture2D>(specification, uuid, filepath.string());

		gpuTaskManager.ExecuteCommandListBlocking(pCopyCommandList);

		outHandle = handle;
		return true;
	}

	template<>
	bool Serializer::Deserialize<Material>(const std::filesystem::path& filepath, AssetHandle& outHandle) noexcept
	{
		std::ifstream inFile(filepath, std::ios_base::binary);
		RLS_ASSERT(inFile.is_open(), "[Serializer]: Unable to open material file.");

		auto [signature, headerVersion] = DeserializeHeaderSignatureAndVersion(inFile);
		if (signature != RASSET_SIGNATURE)
		{
			RLS_CORE_ERROR("Unable to load asset file with path {0}. Asset is either invalid or corrupt.", filepath.string().c_str());
			outHandle = AssetManager::GetInvalidMaterialHandle();
			return false;
		}

		AssetType type = AssetType::Undefined;
		UUID uuid = NULL_UUID;
		std::string name = "";
		uint16_t tagsByteSize = 0u;

		switch (headerVersion)
		{
		case 1:
		{
			const RassetHeader_1 header = DeserializeRAssetHeaderVersion1(inFile);
			type = header.AssetType;
			uuid = header.UUID;
			name = header.Name;
			tagsByteSize = header.TagsByteSize;
			break;
		}
		default:
			RLS_CORE_ERROR("Unable to load asset file with path {0}. Asset version '{1}' does not support deserialization.", filepath.string().c_str(), std::to_string(headerVersion).c_str());
			outHandle = AssetManager::GetInvalidMaterialHandle();
			return false;
		}

		RLS_ASSERT(type == AssetType::Material, "[Serializer]: Asset type mismatch encountered.");

		//Move past eventual tag data:
		inFile.seekg(tagsByteSize, std::ios_base::cur);

		MaterialData data{};
		inFile.read(reinterpret_cast<char*>(&data), sizeof(data));

		AssetHandle materialHandle = AssetManager::CreateNew<Material>(uuid, filepath.string());
		Material& material = AssetManager::Get<Material>(materialHandle);
		material.m_Name = name;
		material.m_RenderMode = static_cast<RenderMode>(data.RenderMode);
		material.m_AlbedoColor = data.AlbedoColor;
		material.m_Metallic = data.Metallic;
		material.m_EmissionColor = data.EmissionColor;
		material.m_EmissionIntensity = data.EmissionIntensity;
		material.m_Roughness = data.Roughness;
		material.m_TilingFactor = data.TilingFactor;
		material.m_Offset = data.Offset;
		material.m_HeightScale = data.HeightScale;
		material.m_AOScale = data.AOScale;
		material.m_CombinedRoughnessMetallnesMap = data.CombinedRoughnessMetallness;
		
		if (data.AlbedoTextureUUID != NULL_UUID)
		{
			const std::filesystem::path filepath = AssetRegistry::GetFilepath(data.AlbedoTextureUUID);
			AssetHandle textureHandle = NULL_HANDLE;
			if (!Serializer::Deserialize<Texture2D>(filepath, textureHandle))
				RLS_CORE_ERROR("[Serializer]: Albedo texture with path '{0}' for material '{1}' is missing or invalid.", filepath.string().c_str(), name.c_str());

			material.SetAlbedoTexture(textureHandle);
		}

		if (data.MetallicTextureUUID != NULL_UUID)
		{
			const std::filesystem::path filepath = AssetRegistry::GetFilepath(data.MetallicTextureUUID);
			AssetHandle textureHandle = NULL_HANDLE;
			if (!Serializer::Deserialize<Texture2D>(filepath, textureHandle))
				RLS_CORE_ERROR("[Serializer]: Metallic texture with path '{0}' for material '{1}' is missing or invalid.", filepath.string().c_str(), name.c_str());
			
			material.SetMetallicTexture(textureHandle);
		}

		if (data.RoughnessTextureUUID != NULL_UUID)
		{
			const std::filesystem::path filepath = AssetRegistry::GetFilepath(data.RoughnessTextureUUID);
			AssetHandle textureHandle = NULL_HANDLE;
			if (!Serializer::Deserialize<Texture2D>(filepath, textureHandle))
				RLS_CORE_ERROR("[Serializer]: Roughness texture with path '{0}' for material '{1}' is missing or invalid.", filepath.string().c_str(), name.c_str());

			material.SetRoughnessTexture(textureHandle);
		}

		if (data.NormalMapUUID != NULL_UUID)
		{
			const std::filesystem::path filepath = AssetRegistry::GetFilepath(data.NormalMapUUID);
			AssetHandle textureHandle = NULL_HANDLE;
			if (!Serializer::Deserialize<Texture2D>(filepath, textureHandle))
				RLS_CORE_ERROR("[Serializer]: Normal map with path '{0}' for material '{1}' is missing or invalid.", filepath.string().c_str(), name.c_str());

			material.SetNormalMap(textureHandle);
		}

		if (data.HeightMapUUID != NULL_UUID)
		{
			const std::filesystem::path filepath = AssetRegistry::GetFilepath(data.HeightMapUUID);
			AssetHandle textureHandle = NULL_HANDLE;
			if (!Serializer::Deserialize<Texture2D>(filepath, textureHandle))
				RLS_CORE_ERROR("[Serializer]: Height map with path '{0}' for material '{1}' is missing or invalid.", filepath.string().c_str(), name.c_str());

			material.SetHeightMap(textureHandle);
		}

		if (data.AmbientOcclusionTextureUUID != NULL_UUID)
		{
			const std::filesystem::path filepath = AssetRegistry::GetFilepath(data.AmbientOcclusionTextureUUID);
			AssetHandle textureHandle = NULL_HANDLE;
			if (!Serializer::Deserialize<Texture2D>(filepath, textureHandle))
				RLS_CORE_ERROR("[Serializer]: Ambient Occlusion texture with path '{0}' for material '{1}' is missing or invalid.", filepath.string().c_str(), name.c_str());

			material.SetAmbientOcclusionTexture(textureHandle);
		}

		if (data.EmissionTextureUUID != NULL_UUID)
		{
			const std::filesystem::path filepath = AssetRegistry::GetFilepath(data.EmissionTextureUUID);
			AssetHandle textureHandle = NULL_HANDLE;
			if (!Serializer::Deserialize<Texture2D>(filepath, textureHandle))
				RLS_CORE_ERROR("[Serializer]: Emission texture with path '{0}' for material '{1}' is missing or invalid.", filepath.string().c_str(), name.c_str());

			material.SetEmissionTexture(textureHandle);
		}

		RLS_CORE_INFO("Deserialized material \"{0}\" with GUID: {1}", material.GetName(), ConvertUUIDToString(uuid));

		outHandle = materialHandle;
		return true;
	}

	[[nodiscard]] RassetHeader_1 Serializer::DeserializeRAssetHeaderVersion1(std::ifstream& ifstream) noexcept
	{
		RLS_ASSERT(ifstream.is_open(), "File stream is not open."); 

		constexpr uint32_t Signature = 'R' << 24 | 'A' << 16 | 'S' << 8 | 'S';

		RassetHeader_1 rassetHeader = {};
		ifstream.read(reinterpret_cast<char*>(&rassetHeader), sizeof(RassetHeader_1));
		//RLS_ASSERT(rassetHeader.Signature == Signature, "RAsset header is invalid or corrupt");
		RLS_ASSERT(rassetHeader.Version == 1, "Rasset Header version is not 1 (version {0} detected)", rassetHeader.Version);

		return rassetHeader;
	}

	std::vector<std::string> Serializer::DeserializeAssetTags(std::ifstream& ifstream, uint32_t tagsByteSize) noexcept
	{
		if (tagsByteSize == 0)
			return {};

		RLS_ASSERT(ifstream.is_open(), "File stream is not open.");

		std::vector<char> buffer(tagsByteSize);
		ifstream.read(buffer.data(), tagsByteSize);

		std::string data(buffer.begin(), buffer.end());

		std::vector<std::string> tags;
		std::istringstream iss(data);
		std::string tag;

		while (std::getline(iss, tag, ';')) 
		{
			if (!tag.empty()) 
			{
				tags.push_back(tag);
			}
		}

		return tags;
	}

	std::pair<uint32_t, uint8_t> Serializer::DeserializeHeaderSignatureAndVersion(std::ifstream& ifstream) noexcept
	{
		RLS_ASSERT(ifstream.is_open(), "File stream is not open.");

		uint32_t signature = 0u;
		uint8_t version = 0u;

		ifstream.read(reinterpret_cast<char*>(&signature), sizeof(uint32_t));
		ifstream.read(reinterpret_cast<char*>(&version), sizeof(uint8_t));

		ifstream.clear();
		ifstream.seekg(0, std::ios::beg);

		return {signature, version};
	}

	void Serializer::SerializeRassetHeader(const AssetHandle& assetHandle, std::ofstream& outFileStream) noexcept
	{
		RLS_ASSERT(outFileStream.is_open(), "[Serializer]: File stream is not open.");
		RLS_ASSERT(assetHandle.IsValid(), "[Serializer]: Asset handle is invalid");

		const AssetMetaData metaData = AssetRegistry::GetMetaData(assetHandle);

		LatestRassetHeaderVersion rassetHeader; 
		rassetHeader.AssetType = metaData.AssetType;
		rassetHeader.UUID = metaData.Uuid;
		strcpy_s(rassetHeader.Name, Rules::Limits::ASSET_NAME_LENGTH, metaData.Name.c_str());
		strcpy_s(rassetHeader.SourcePath, Rules::Limits::ASSET_SOURCE_PATH_LENGTH, metaData.SourcePath.string().c_str());
		rassetHeader.AssetFlags = metaData.AssetFlags;
		rassetHeader.ModificationDateAndTime = metaData.ModificationDateAndTime;
		
		rassetHeader.TagsByteSize = 0u;
		for (auto& tag : metaData.Tags)
		{
			rassetHeader.TagsByteSize += (tag.length() * sizeof(char)) + 1; // includes delimiter
		}

		outFileStream.write(reinterpret_cast<char*>(&rassetHeader), sizeof(LatestRassetHeaderVersion));
	}


	void Serializer::SerializeAssetTags(const AssetHandle& assetHandle, std::ofstream& outFileStream) noexcept
	{
		RLS_ASSERT(outFileStream.is_open(), "[Serializer]: File stream is not open.");
		RLS_ASSERT(assetHandle.IsValid(), "[Serializer]: Asset handle is invalid");

		const std::vector<std::string> assetTags = AssetRegistry::GetMetaData(assetHandle).Tags;

		constexpr const char delimiter = ';';
		for (auto& tag : assetTags)
		{
			outFileStream.write(tag.c_str(), tag.length() * sizeof(char));
			outFileStream.write(&delimiter, 1);
		}
	}

}