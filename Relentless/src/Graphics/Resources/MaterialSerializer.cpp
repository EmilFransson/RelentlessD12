#include "Material.h"
#include "MaterialSerializer.h"
#include "AssetManager.h"
#include "../../Utility/SerializeUtilities.h"

namespace Relentless
{
	void MaterialSerializer::Serialize(MaterialHandle& materialHandle, const std::string& path, const std::string& toReplace) noexcept
	{
		RLS_ASSERT(materialHandle != NULL_HANDLE, "Material handle is invalid.");

		Material& material = AssetManager::Get<Material>(materialHandle);

		YAML::Emitter out;
		out << YAML::BeginMap;
		
		out << YAML::Key << "Material" << YAML::Value << YAML::BeginMap;
		out << YAML::Key << "Name" << YAML::Value << material.GetName();
		out << YAML::Key << "Render Mode" << YAML::Value << (uint32_t)material.GetRenderMode();
		out << YAML::Key << "Albedo Color" << YAML::Value << material.m_AlbedoColor;
		out << YAML::Key << "Metallic" << YAML::Value << material.m_Metallic;
		out << YAML::Key << "Emission Color" << YAML::Value << material.m_EmissionColor;
		out << YAML::Key << "Emission Intensity" << YAML::Value << material.m_EmissionIntensity;
		out << YAML::Key << "Roughness" << YAML::Value << material.m_Roughness;
		out << YAML::Key << "Tiling Factor" << YAML::Value << material.m_TilingFactor;
		out << YAML::Key << "Offset" << YAML::Value << material.m_Offset;
		out << YAML::Key << "Heightmap Scale" << YAML::Value << material.m_HeightScale;
		out << YAML::Key << "AO Scale" << YAML::Value << material.m_AOScale;
		out << YAML::Key << "Combined RoughnessMetalness" << YAML::Value << (bool)material.m_CombinedRoughnessMetallnesMap;

		//Maps:
		out << YAML::Key << "Maps" << YAML::Value << YAML::BeginMap;
		out << YAML::Key << "Albedo" << YAML::Value << material.m_AlbedoTextureHandle.UUID;
		out << YAML::Key << "Metallic" << YAML::Value << material.m_MetallicTextureHandle.UUID;
		out << YAML::Key << "Roughness" << YAML::Value << material.m_RoughnessTextureHandle.UUID;
		out << YAML::Key << "Normal" << YAML::Value << material.m_NormalMapHandle.UUID;
		out << YAML::Key << "Height" << YAML::Value << material.m_HeightMapHandle.UUID;
		out << YAML::Key << "AO" << YAML::Value << material.m_AmbientOcclusionTextureHandle.UUID;
		out << YAML::Key << "Emission" << YAML::Value << material.m_EmissionTextureHandle.UUID;

		out << YAML::EndMap;

		out << YAML::EndMap;
		out << YAML::EndMap;

		std::ofstream outFile(path);
		RLS_ASSERT(outFile.is_open(), "Unable to open/create material file to write to.");

		outFile << out.c_str();
		outFile.close();

		//Corresponding rasset-file, if it not already exists:
		//Otherwise, the guid is preserved and only the material is re-serealized.
		//(For materials, the rasset-file only contains the guid!)
		if (!std::filesystem::exists(path + ASSET_EXTENSION))
		{
			std::string pathWithExtension = path + ASSET_EXTENSION;
			YAML::Emitter outAsset;

			outAsset << YAML::BeginMap;
			outAsset << YAML::Key << "GUID" << YAML::Value << materialHandle.UUID;
			outAsset << YAML::EndMap;

			outFile.open(pathWithExtension);
			RLS_ASSERT(outFile.is_open(), "Unable to open/create material asset file to write to.");
			outFile << outAsset.c_str();
			outFile.close();

			::SetFileAttributesA(pathWithExtension.c_str(), GetFileAttributesA(pathWithExtension.c_str()) | FILE_ATTRIBUTE_HIDDEN);
		}

		if (!toReplace.empty())
		{
			RLS_ASSERT(std::filesystem::exists(toReplace), ".rmat file to replace does not exist.");
			RLS_ASSERT(std::filesystem::exists(toReplace + ASSET_EXTENSION), ".rasset file to replace does not exist.");
		
			std::filesystem::remove(toReplace);
			std::filesystem::remove(toReplace + ASSET_EXTENSION);
		}
	}

	UUID MaterialSerializer::SerializeDefault(const std::string& path) noexcept
	{
		//RLS_ASSERT(std::filesystem::exists(path), "Path is invalid.");
		//RLS_ASSERT(std::filesystem::path(path).extension().string() == ".rmat", "File is not of type material.");

		{
			Material material{};

			YAML::Emitter out;
			out << YAML::BeginMap;

			out << YAML::Key << "Material" << YAML::Value << YAML::BeginMap;
			out << YAML::Key << "Name" << YAML::Value << std::filesystem::path(path).filename().stem().string();
			out << YAML::Key << "Render Mode" << YAML::Value << (uint32_t)material.GetRenderMode();
			out << YAML::Key << "Albedo Color" << YAML::Value << material.m_AlbedoColor;
			out << YAML::Key << "Metallic" << YAML::Value << material.m_Metallic;
			out << YAML::Key << "Emission Color" << YAML::Value << material.m_EmissionColor;
			out << YAML::Key << "Emission Intensity" << YAML::Value << material.m_EmissionIntensity;
			out << YAML::Key << "Roughness" << YAML::Value << material.m_Roughness;
			out << YAML::Key << "Tiling Factor" << YAML::Value << material.m_TilingFactor;
			out << YAML::Key << "Offset" << YAML::Value << material.m_Offset;
			out << YAML::Key << "Heightmap Scale" << YAML::Value << material.m_HeightScale;
			out << YAML::Key << "AO Scale" << YAML::Value << material.m_AOScale;
			out << YAML::Key << "Combined RoughnessMetalness" << YAML::Value << (bool)material.m_CombinedRoughnessMetallnesMap;

			//Maps:
			out << YAML::Key << "Maps" << YAML::Value << YAML::BeginMap;
			out << YAML::Key << "Albedo" << YAML::Value << material.m_AlbedoTextureHandle.UUID;
			out << YAML::Key << "Metallic" << YAML::Value << material.m_MetallicTextureHandle.UUID;
			out << YAML::Key << "Roughness" << YAML::Value << material.m_RoughnessTextureHandle.UUID;
			out << YAML::Key << "Normal" << YAML::Value << material.m_NormalMapHandle.UUID;
			out << YAML::Key << "Height" << YAML::Value << material.m_HeightMapHandle.UUID;
			out << YAML::Key << "AO" << YAML::Value << material.m_AmbientOcclusionTextureHandle.UUID;
			out << YAML::Key << "Emission" << YAML::Value << material.m_EmissionTextureHandle.UUID;

			out << YAML::EndMap;

			out << YAML::EndMap;
			out << YAML::EndMap;

			std::ofstream outFile(path);
			RLS_ASSERT(outFile.is_open(), "Unable to open/create material file to write to.");

			outFile << out.c_str();
			outFile.close();
		}

		YAML::Emitter outAsset;
		UUID uuid = CreateUUID();

		outAsset << YAML::BeginMap;
		outAsset << YAML::Key << "GUID" << YAML::Value << uuid;
		outAsset << YAML::EndMap;

		std::string pathWithExtension = path + ASSET_EXTENSION;
		std::ofstream outFile(pathWithExtension);
		RLS_ASSERT(outFile.is_open(), "Unable to open/create material asset file to write to.");
		outFile << outAsset.c_str();
		outFile.close();

		::SetFileAttributesA(pathWithExtension.c_str(), GetFileAttributesA(pathWithExtension.c_str()) | FILE_ATTRIBUTE_HIDDEN);

		return uuid;
	}

	MaterialHandle MaterialSerializer::Deserialize(const std::string& fullPath) noexcept
	{
		RLS_ASSERT(std::filesystem::exists(fullPath), "Material file does not exist.");
		RLS_ASSERT(std::filesystem::path(fullPath).extension() == ".rmat", "File is not of type material.");
		RLS_ASSERT(std::filesystem::exists(fullPath + ASSET_EXTENSION), "Corresponding .rasset file is missing.");

		std::ifstream inFile(fullPath);
		RLS_ASSERT(inFile.is_open(), "Unable to open material file.");
		
		std::stringstream sstream;
		sstream << inFile.rdbuf();
		inFile.close();
		YAML::Node materialData = YAML::Load(sstream.str());

		inFile.open(fullPath + ASSET_EXTENSION);
		RLS_ASSERT(inFile.is_open(), "Unable to open material rasset file.");

		std::stringstream sstream2;
		sstream2 << inFile.rdbuf();
		inFile.close();
		YAML::Node materialRassetData = YAML::Load(sstream2.str());

		RLS_ASSERT(materialData["Material"], "Material node not present in YAML buffer.");
		RLS_ASSERT(materialRassetData["GUID"], "GUID node not present in YAML buffer.");

		std::string GUIDAsString = materialRassetData["GUID"].as<std::string>();
		std::wstring GUIDAsWString = ConvertStringToWstring(GUIDAsString);
		GUID uuid;
		IIDFromString(GUIDAsWString.c_str(), &uuid);

		MaterialManager& materialManager = AssetManager::GetMaterialManager();

		auto mat = materialData["Material"];
		std::string materialName = mat["Name"].as<std::string>();

		const MaterialHandle materialHandle = materialManager.CreateWithUUID(uuid, materialName);
		Material& material = materialManager.GetMaterial(materialHandle);
		material.SetRenderMode((RenderMode)mat["Render Mode"].as<uint32_t>());
		material.m_AlbedoColor = mat["Albedo Color"].as<DirectX::XMFLOAT4>();
		material.m_Metallic = mat["Metallic"].as<float>();
		material.m_EmissionColor = mat["Emission Color"].as<DirectX::XMFLOAT4>();
		material.m_EmissionIntensity = mat["Emission Intensity"].as<float>();
		material.m_Roughness = mat["Roughness"].as<float>();
		material.m_TilingFactor = mat["Tiling Factor"].as<DirectX::XMFLOAT2>();
		material.m_Offset = mat["Offset"].as<DirectX::XMFLOAT2>();
		material.m_HeightScale = mat["Heightmap Scale"].as<float>();
		material.m_AOScale = mat["AO Scale"].as<float>();
		material.m_CombinedRoughnessMetallnesMap = mat["Combined RoughnessMetalness"].as<bool>();
		
		uuid = ConvertStringToGUID(mat["Maps"]["Albedo"].as<std::string>());
		if (uuid != NULL_UUID)
		{
			const std::string path = AssetManager::GetAssetPath(uuid);
			material.SetAlbedoTexture(AssetManager::Load<Texture2D>(path));
		}
		
		uuid = ConvertStringToGUID(mat["Maps"]["Metallic"].as<std::string>());
		if (uuid != NULL_UUID)
		{
			const std::string path = AssetManager::GetAssetPath(uuid);
			material.SetMetallicTexture(AssetManager::Load<Texture2D>(path));
		}

		uuid = ConvertStringToGUID(mat["Maps"]["Roughness"].as<std::string>());
		if (uuid != NULL_UUID)
		{
			const std::string path = AssetManager::GetAssetPath(uuid);
			material.SetRoughnessTexture(AssetManager::Load<Texture2D>(path));
		}

		uuid = ConvertStringToGUID(mat["Maps"]["Normal"].as<std::string>());
		if (uuid != NULL_UUID)
		{
			const std::string path = AssetManager::GetAssetPath(uuid);
			material.SetNormalMap(AssetManager::Load<Texture2D>(path));
		}

		uuid = ConvertStringToGUID(mat["Maps"]["Height"].as<std::string>());
		if (uuid != NULL_UUID)
		{
			const std::string path = AssetManager::GetAssetPath(uuid);
			material.SetHeightMap(AssetManager::Load<Texture2D>(path));
		}

		uuid = ConvertStringToGUID(mat["Maps"]["AO"].as<std::string>());
		if (uuid != NULL_UUID)
		{
			const std::string path = AssetManager::GetAssetPath(uuid);
			material.SetAmbientOcclusionTexture(AssetManager::Load<Texture2D>(path));
		}

		uuid = ConvertStringToGUID(mat["Maps"]["Emission"].as<std::string>());
		if (uuid != NULL_UUID)
		{
			const std::string path = AssetManager::GetAssetPath(uuid);
			material.SetEmissionTexture(AssetManager::Load<Texture2D>(path));
		}

		RLS_CORE_INFO("Deserialized material \"{0}\"", materialName);

		return materialHandle;
	}
}