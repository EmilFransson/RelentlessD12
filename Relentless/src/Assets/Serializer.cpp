#include "Assets/AssetManager.h"
#include "Core/Application.h"
#include "Graphics/Resources/Texture.h"
#include "Graphics/Renderer/RenderCommand.h"
#include "ImportSettings.h"
#include "Mesh/Mesh.h"
#include "Mesh/Vertex.h"
#include "Serializer.h"
#include "Utility/Common.h"
#include "Utility/SerializeUtilities.h"
namespace Relentless
{
	struct Texture2DSerializationContext
	{
		std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> StagingBuffers;
		std::string Path = "?";
	};

	struct Texture2DDeserializationContext
	{
		Microsoft::WRL::ComPtr<ID3D12Resource> pUploadBuffer = nullptr;
	};

	struct Data
	{
		std::unordered_map<UUID, Texture2DSerializationContext> UUIDToStagingResources;
		std::unordered_map<UUID, Texture2DDeserializationContext> UUIDToUploadBuffer;
	};

	static Data s_Data;

	#pragma pack(push, 1)
	struct MaterialData
	{
		char Name[30];
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
	void Serializer::Serialize<Material>(const AssetHandle& assetHandle, const std::string& path) noexcept
	{
		Material& material = AssetManager::Get<Material>(assetHandle);
		RLS_ASSERT(material.GetName().length() <= 30, "[Serializer]: Unable to serialize full material name");
		
#if defined RLS_DEBUG
		if (std::filesystem::path(path).has_extension())
		{
			RLS_ASSERT(std::filesystem::path(path).extension().string() == ASSET_EXTENSION, "Extension provided not supported.");
		}
#endif

		RassetHeader rassetHeader;
		rassetHeader.AssetType = AssetType::Material;
		rassetHeader.UUID = assetHandle.Uuid;

		MaterialData data{};
		strcpy_s(data.Name, sizeof(data.Name), material.GetName().c_str());
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

		std::filesystem::path sanitizedPath = path;
		if (!sanitizedPath.has_filename())
		{
			sanitizedPath.append(material.GetName());
		}
		if (!sanitizedPath.has_extension())
		{
			sanitizedPath += ASSET_EXTENSION;
		}

		std::ofstream outFile(sanitizedPath, std::ios::binary);
		RLS_ASSERT(outFile.is_open(), "[Serializer]: Failed to open output file.");

		outFile.write(reinterpret_cast<char*>(&rassetHeader), sizeof(rassetHeader));
		outFile.write(reinterpret_cast<char*>(&data), sizeof(data));

		outFile.close();

		if (!AssetManager::IsLoaded(sanitizedPath.string()))
		{
			AssetManager::Link(sanitizedPath.string(), rassetHeader.UUID);
		}
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

	static void FinalizeTexture2DSerialization(const AssetHandle& handle) noexcept 
	{
		using namespace Microsoft::WRL;

		const Texture2D& texture = AssetManager::Get<Texture2D>(handle);
		const ComPtr<ID3D12Resource> gpuTextureInterface = texture.GetInterface();

		const D3D12_RESOURCE_DESC textureDescriptor = gpuTextureInterface->GetDesc();

		const uint32_t nrOfMips = textureDescriptor.MipLevels;

		std::ofstream outFile(s_Data.UUIDToStagingResources[handle.Uuid].Path, std::ios::binary);
		RLS_ASSERT(outFile.is_open(), "Failed to open file for writing.");

		RassetHeader rassetHeader
		{
			.AssetType = AssetType::Texture2D,
			.UUID = handle.Uuid
		};
		outFile.write(reinterpret_cast<char*>(&rassetHeader), sizeof(rassetHeader));

		TextureHeader textureHeader
		{
			.BaseWidth = texture.GetWidth(),
			.BaseHeight = texture.GetHeight(),
			.NrOfMips = nrOfMips,
			.Samples = texture.GetMultiSampleCount(),
			.Format = texture.GetFormat(),
			.IsSRGB = true
		};
		outFile.write(reinterpret_cast<char*>(&textureHeader), sizeof(textureHeader));

		// Process each mip level
		for (uint32_t mipLevel = 0; mipLevel < nrOfMips; ++mipLevel)
		{
			ComPtr<ID3D12Resource> pStagingResource = s_Data.UUIDToStagingResources[handle.Uuid].StagingBuffers[mipLevel];

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
	}

	template<>
	void Serializer::Serialize<Texture2D>(const AssetHandle& assetHandle, const std::string& path) noexcept
	{
		//Split into two parts, as a texture must be read back from GPU memory to CPU to be accessible,
		//However, this requires waiting for the GPU to finish; one can't assume it is fast enough.

		using namespace Microsoft::WRL;

		Texture2D& texture = AssetManager::Get<Texture2D>(assetHandle);
		const ComPtr<ID3D12Resource> gpuInterface = texture.GetInterface();
		const D3D12_RESOURCE_DESC textureDescriptor = gpuInterface->GetDesc();

		const uint32_t mipLevels = textureDescriptor.MipLevels;

		//Transition resource to be eligible as readback/copy source:
		RenderCommand::TransitionResource(texture, D3D12_RESOURCE_STATE_COPY_SOURCE);

		Texture2DSerializationContext context = {};
		context.Path = path;
		for (uint32_t mipLevel = 0u; mipLevel < mipLevels; ++mipLevel)
		{
			ComPtr<ID3D12Resource> pStagingResource = nullptr;

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
				IID_PPV_ARGS(&pStagingResource)));

			// Copy the mip level to the staging resource
			D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
			srcLocation.pResource = gpuInterface.Get();
			srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
			srcLocation.SubresourceIndex = mipLevel;

			D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
			dstLocation.pResource = pStagingResource.Get();
			dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
			dstLocation.PlacedFootprint = footprint;
		
			DXCall_STD(D3D12Core::GetCommandList()->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr));
	
			context.StagingBuffers.push_back(pStagingResource);
		}

		RenderCommand::TransitionResource(texture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

		s_Data.UUIDToStagingResources[assetHandle.Uuid] = context;

		//Copying will take time, step two is deferred until the point where it is complete:
		Application::Get().SubmitToMainThread([assetHandle]()
			{
				MasterRenderer::WaitAndSyncAllFramesInFlight();
				FinalizeTexture2DSerialization(assetHandle);
			});
	}

	template<>
	void Serializer::Serialize<Mesh>(const AssetHandle& assetHandle, const std::string& directoryPath) noexcept
	{
		RLS_ASSERT(assetHandle != NULL_HANDLE, "Mesh asset handle is invalid.");

		const Mesh& mesh = AssetManager::Get<Mesh>(assetHandle);
		const std::unique_ptr<VertexBuffer>& pVertexBuffer = mesh.GetVertexBuffer();
		const std::unique_ptr<IndexBuffer>& pIndexBuffer = mesh.GetIndexBuffer();

		VertexBuffer::Specification& vertexBufferSpecification = pVertexBuffer->GetSpecification();
		IndexBuffer::Specification& indexBufferSpecification = pIndexBuffer->GetSpecification();

		RassetHeader rassetHeader{};
		rassetHeader.AssetType = AssetType::Mesh;
		rassetHeader.UUID = assetHandle.Uuid;

		MeshDataHeader meshDataHeader
		{
			.VertexBufferSizeInBytes = vertexBufferSpecification.TotalSizeInBytes,
			.IndexBufferSizeInBytes = indexBufferSpecification.TotalSizeInBytes,
			.VertexCount = vertexBufferSpecification.NrOfVertices,
			.IndexCount = indexBufferSpecification.NrOfIndices
		};

		const std::string path = directoryPath + mesh.GetName() + ASSET_EXTENSION;
		std::ofstream outFile(path, std::ios::binary);

		//Write the rasset header:
		outFile.write(reinterpret_cast<char*>(&rassetHeader), sizeof(rassetHeader));

		//Write the mesh header:
		outFile.write(reinterpret_cast<char*>(&meshDataHeader), sizeof(meshDataHeader));

		// Write the vertex and index data
		outFile.write(static_cast<char*>(vertexBufferSpecification.pBuffer), meshDataHeader.VertexBufferSizeInBytes);
		outFile.write(static_cast<char*>(indexBufferSpecification.pBuffer), meshDataHeader.IndexBufferSizeInBytes);

		outFile.close();

		//Sanity check:
#if defined RLS_DEBUG
		std::ifstream inFile(path, std::ios::binary);

		RassetHeader readRassetHeader;
		MeshDataHeader readMeshHeader;

		inFile.read(reinterpret_cast<char*>(&readRassetHeader), sizeof(readRassetHeader));
		RLS_ASSERT(memcmp(&readRassetHeader, &rassetHeader, sizeof(readRassetHeader)) == 0, "Rasset Data is not equal");

		inFile.read(reinterpret_cast<char*>(&readMeshHeader), sizeof(readMeshHeader));
		RLS_ASSERT(memcmp(&readMeshHeader, &meshDataHeader, sizeof(readMeshHeader)) == 0, "Mesh header Data is not equal");

		std::vector<char> readVertexData(meshDataHeader.VertexBufferSizeInBytes);
		std::vector<char> readIndexData(meshDataHeader.IndexBufferSizeInBytes);

		inFile.read(reinterpret_cast<char*>(readVertexData.data()), meshDataHeader.VertexBufferSizeInBytes);
		RLS_ASSERT(memcmp(readVertexData.data(), vertexBufferSpecification.pBuffer, meshDataHeader.VertexBufferSizeInBytes) == 0, "Data is not equal");

		inFile.read(reinterpret_cast<char*>(readIndexData.data()), meshDataHeader.IndexBufferSizeInBytes);
		RLS_ASSERT(memcmp(readIndexData.data(), indexBufferSpecification.pBuffer, meshDataHeader.IndexBufferSizeInBytes) == 0, "Data is not equal");
#endif
	}

	template<>
	AssetHandle Serializer::Deserialize<Mesh>(const std::string& filepath) noexcept
	{
		RLS_ASSERT(std::filesystem::exists(filepath), "File does not exist.");

		if (AssetManager::IsLoaded(filepath))
		{
			return AssetManager::GetHandleByPath(filepath);
		}

		std::string meshName = std::filesystem::path(filepath).filename().stem().string();
	
		std::ifstream inFile(filepath, std::ios::binary);
	
		RassetHeader readRassetHeader;
		inFile.read(reinterpret_cast<char*>(&readRassetHeader), sizeof(readRassetHeader));

		RLS_ASSERT(readRassetHeader.AssetType == AssetType::Mesh, "Asset is not of type mesh.");

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
			.Name = meshName + std::string(" Vertex Buffer")
		};
	
		IndexBuffer::Specification ibSpec
		{
			.NrOfIndices = (uint32_t)readMeshHeader.IndexCount,
			.TotalSizeInBytes = (uint32_t)readMeshHeader.IndexBufferSizeInBytes,
			.Stride = sizeof(uint32_t),
			.pBuffer = (void*)readIndexData.data(),
			.Name = meshName + std::string(" Index Buffer")
		};
	
		AssetHandle meshHandle = AssetManager::CreateNew<Mesh>(readRassetHeader.UUID, filepath);
		Mesh& mesh = AssetManager::Get<Mesh>(meshHandle);
		mesh.SetName(meshName);
		mesh.SetVertexBuffer(std::make_unique<VertexBuffer>(&vbSpec));
		mesh.SetIndexBuffer(std::make_unique<IndexBuffer>(&ibSpec));

		RLS_CORE_INFO("[Serializer]: Loaded mesh '{0}' with GUID: {1}", meshName, ConvertUUIDToString(meshHandle.Uuid));
	
		return meshHandle;
	}

	template<>
	AssetHandle Serializer::Deserialize<Texture2D>(const std::string& filepath) noexcept
	{
		RLS_ASSERT(std::filesystem::exists(filepath), "[Serializer]: Filepath is invalid.");
		RLS_ASSERT(std::filesystem::path(filepath).extension() == ASSET_EXTENSION, "[Serializer]: File not of correct rasset format.");

		if (AssetManager::IsLoaded(filepath))
		{
			return AssetManager::GetHandleByPath(filepath);
		}

		std::ifstream inFile(filepath, std::ios::binary);
		RLS_ASSERT(inFile.is_open(), "[Serializer]: Unable to open filepath {0}", filepath);

		RassetHeader rassetHeader = {};
		inFile.read(reinterpret_cast<char*>(&rassetHeader), sizeof(RassetHeader));
		RLS_ASSERT(rassetHeader.AssetType == AssetType::Texture2D, "[Serializer]: Invalid asset type encountered.");

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

			DXCall_STD(D3D12Core::GetCommandList()->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr));
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

		RenderCommand::TransitionResource(pTextureResource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

		specification.pResource = pTextureResource;
		specification.Name = filepath;
		specification.Width = textureHeader.BaseWidth;
		specification.Height = textureHeader.BaseHeight;
		specification.Format = textureHeader.Format;
		specification.MipCount = textureHeader.NrOfMips;
		specification.IsSRGB = textureHeader.IsSRGB;
		specification.SampleCount = textureHeader.Samples;
		
		s_Data.UUIDToUploadBuffer[rassetHeader.UUID].pUploadBuffer = pUploadBuffer;
		
		const AssetHandle handle = AssetManager::CreateNew<Texture2D>(specification, rassetHeader.UUID, filepath);
		
		Application::Get().SubmitToMainThread([handle]()
			{
				MasterRenderer::WaitAndSyncAllFramesInFlight();
				s_Data.UUIDToUploadBuffer.erase(handle.Uuid);
			});

		return handle;
	}

	template<>
	AssetHandle Serializer::Deserialize<Material>(const std::string& filepath) noexcept
	{
		RLS_ASSERT(std::filesystem::exists(filepath), "[Serializer]: Material file does not exist.");
		RLS_ASSERT(std::filesystem::path(filepath).extension() == ASSET_EXTENSION, "[Serializer]: File not of correct rasset format.");

		if (AssetManager::IsLoaded(filepath))
		{
			return AssetManager::GetHandleByPath(filepath);
		}

		std::ifstream inFile(filepath, std::ios_base::binary);
		RLS_ASSERT(inFile.is_open(), "[Serializer]: Unable to open material file.");

		RassetHeader rassetHeader{};

		inFile.read(reinterpret_cast<char*>(&rassetHeader), sizeof(rassetHeader));
		RLS_ASSERT(rassetHeader.AssetType == AssetType::Material, "[Serializer]: Asset type mismatch encountered.");

		MaterialData data{};
		inFile.read(reinterpret_cast<char*>(&data), sizeof(data));

		AssetHandle materialHandle = AssetManager::CreateNew<Material>(rassetHeader.UUID, filepath);
		Material& material = AssetManager::Get<Material>(materialHandle);
		material.m_Name = data.Name;
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
			const std::string path = AssetRegistry::GetFilepath(data.AlbedoTextureUUID);
			TextureImportSettings defaultImportSettings{};

			const AssetHandle textureHandle = AssetManager::LoadFromFile<Texture2D>(path, defaultImportSettings, data.AlbedoTextureUUID);
			material.SetAlbedoTexture(textureHandle);
		}

		if (data.MetallicTextureUUID != NULL_UUID)
		{
			const std::string path = AssetRegistry::GetFilepath(data.MetallicTextureUUID);
			TextureImportSettings importSettings
			{
				.IsSRGB = false
			};
			const AssetHandle textureHandle = AssetManager::LoadFromFile<Texture2D>(path, importSettings, data.MetallicTextureUUID);
			material.SetMetallicTexture(textureHandle);
		}

		if (data.RoughnessTextureUUID != NULL_UUID)
		{
			const std::string path = AssetRegistry::GetFilepath(data.RoughnessTextureUUID);
			TextureImportSettings importSettings
			{
				.IsSRGB = false
			};
			const AssetHandle textureHandle = AssetManager::LoadFromFile<Texture2D>(path, importSettings, data.RoughnessTextureUUID);
			material.SetRoughnessTexture(textureHandle);
		}

		if (data.NormalMapUUID != NULL_UUID)
		{
			const std::string path = AssetRegistry::GetFilepath(data.NormalMapUUID);
			TextureImportSettings importSettings
			{
				.IsSRGB = false
			};
			const AssetHandle textureHandle = AssetManager::LoadFromFile<Texture2D>(path, importSettings, data.NormalMapUUID);
			material.SetNormalMap(textureHandle);
		}

		if (data.HeightMapUUID != NULL_UUID)
		{
			const std::string path = AssetRegistry::GetFilepath(data.HeightMapUUID);
			TextureImportSettings importSettings
			{
				.IsSRGB = false
			};
			const AssetHandle textureHandle = AssetManager::LoadFromFile<Texture2D>(path, importSettings, data.HeightMapUUID);
			material.SetHeightMap(textureHandle);
		}

		if (data.AmbientOcclusionTextureUUID != NULL_UUID)
		{
			const std::string path = AssetRegistry::GetFilepath(data.AmbientOcclusionTextureUUID);
			TextureImportSettings importSettings
			{
				.IsSRGB = false
			};
			const AssetHandle textureHandle = AssetManager::LoadFromFile<Texture2D>(path, importSettings, data.AmbientOcclusionTextureUUID);
			material.SetAmbientOcclusionTexture(textureHandle);
		}

		if (data.EmissionTextureUUID != NULL_UUID)
		{
			const std::string path = AssetRegistry::GetFilepath(data.EmissionTextureUUID);
			TextureImportSettings importSettings
			{
				.IsSRGB = false
			};
			const AssetHandle textureHandle = AssetManager::LoadFromFile<Texture2D>(path, importSettings, data.EmissionTextureUUID);
			material.SetEmissionTexture(textureHandle);
		}

		RLS_CORE_INFO("Deserialized material \"{0}\" with GUID: {1}", material.GetName(), ConvertUUIDToString(rassetHeader.UUID));

		return materialHandle;
	}
}