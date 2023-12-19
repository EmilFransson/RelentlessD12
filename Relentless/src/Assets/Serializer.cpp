#include "Assets/AssetManager.h"
#include "Graphics/Resources/Texture.h"
#include "ImportSettings.h"
#include "Mesh/Mesh.h"
#include "Mesh/Vertex.h"
#include "Serializer.h"
#include "Utility/Common.h"
#include "Utility/SerializeUtilities.h"
namespace Relentless
{
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

	template<>
	void Serializer::Serialize<Material>(const AssetHandle& assetHandle, const std::string& path) noexcept
	{
		Material& material = AssetManager::Get<Material>(assetHandle);
		RLS_ASSERT(material.GetName().length() <= 30, "[Serializer]: Unable to serialize full material name");
		RLS_ASSERT(std::filesystem::path(path).extension().string() == ".rasset", "[Serializer]: File not of correct rasset format.");

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

		std::ofstream outFile(path, std::ios::binary);
		RLS_ASSERT(outFile.is_open(), "[Serializer]: Failed to open output file.");

		outFile.write(reinterpret_cast<char*>(&rassetHeader), sizeof(rassetHeader));
		outFile.write(reinterpret_cast<char*>(&data), sizeof(data));

		outFile.close();

		if (!AssetManager::IsLoaded(path))
		{
			AssetManager::Link(path, rassetHeader.UUID);
		}
	}

	template<>
	void Serializer::Serialize<Mesh>(const AssetHandle& assetHandle, const std::string& path) noexcept
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

		RLS_CORE_INFO("Loaded mesh '{0}' with GUID: {1}", meshName, ConvertUUIDToString(meshHandle.Uuid));
	
		return meshHandle;
	}

	template<>
	AssetHandle Serializer::Deserialize<Material>(const std::string& filepath) noexcept
	{
		RLS_ASSERT(std::filesystem::exists(filepath), "[Serializer]: Material file does not exist.");
		RLS_ASSERT(std::filesystem::path(filepath).extension() == ".rasset", "[Serializer]: File not of correct rasset format.");

		if (AssetManager::IsLoaded(filepath))
		{
			RLS_CORE_INFO("[Serializer]: Material already loaded - returning handle.");
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

		RLS_CORE_INFO("Deserialized material \"{0}\"", material.GetName());

		return materialHandle;
	}
}