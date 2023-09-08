#include "TextureSerializer.h"
#include "AssetManager.h"
#include "../../Utility/SerializeUtilities.h"
namespace Relentless
{
	void TextureSerializer::Serialize(TextureHandle& textureHandle, const std::string& path, const std::string& toReplace) noexcept
	{
		RLS_ASSERT(textureHandle != NULL_HANDLE, "Texture handle is invalid.");

		Texture2D& texture = AssetManager::Get<Texture2D>(textureHandle);

		YAML::Emitter out;
		out << YAML::BeginMap;

		out << YAML::Key << "Texture" << YAML::Value << YAML::BeginMap;
		out << YAML::Key << "GUID" << YAML::Value << textureHandle.UUID;
		out << YAML::Key << "Name" << YAML::Value << texture.GetName();

		out << YAML::EndMap;
		out << YAML::EndMap;

		std::string pathWithExtension = path + ASSET_EXTENSION;
		std::ofstream outFile(pathWithExtension);
		RLS_ASSERT(outFile.is_open(), "Unable to open/create texture asset file to write to.");

		outFile << out.c_str();
		outFile.close();
		::SetFileAttributesA(pathWithExtension.c_str(), GetFileAttributesA(pathWithExtension.c_str()) | FILE_ATTRIBUTE_HIDDEN);

		if (!toReplace.empty())
		{
			RLS_ASSERT(std::filesystem::exists(toReplace + ASSET_EXTENSION), ".rasset file to replace does not exist.");
			std::filesystem::remove(toReplace + ASSET_EXTENSION);
		}
	}

	UUID TextureSerializer::SerializeDefault(const std::string& path) noexcept
	{
		RLS_ASSERT(std::filesystem::exists(path), "Path is invalid.");
		RLS_ASSERT(std::filesystem::path(path).extension().string() == ".jpg" ||
			std::filesystem::path(path).extension().string() == ".png", "File is not of type texture.");

		UUID uuid = CreateUUID();
		const std::string textureName = std::filesystem::path(path).filename().string();
		
		YAML::Emitter out;
		out << YAML::BeginMap;

		out << YAML::Key << "Texture" << YAML::Value << YAML::BeginMap;
		out << YAML::Key << "GUID" << YAML::Value << uuid;
		out << YAML::Key << "Name" << YAML::Value << textureName;

		out << YAML::EndMap;
		out << YAML::EndMap;

		std::string pathWithExtension = path + ASSET_EXTENSION;
		std::ofstream outFile(pathWithExtension);
		RLS_ASSERT(outFile.is_open(), "Unable to open/create texture asset file to write to.");

		outFile << out.c_str();
		outFile.close();
		::SetFileAttributesA(pathWithExtension.c_str(), GetFileAttributesA(pathWithExtension.c_str()) | FILE_ATTRIBUTE_HIDDEN);

		return uuid;
	}

	TextureHandle TextureSerializer::Deserialize(const std::string& fullPath) noexcept
	{
		RLS_ASSERT(std::filesystem::exists(fullPath), "Path is not valid for texture deserialization.");
		const std::string rassetFilePath = fullPath + ASSET_EXTENSION;
		RLS_ASSERT(std::filesystem::exists(rassetFilePath), "Texture .rasset path is not valid for texture deserialization.");
		 
		std::ifstream inFile(rassetFilePath);
		RLS_ASSERT(inFile.is_open(), "Unable to open material file.");

		std::stringstream sstream;
		sstream << inFile.rdbuf();
		inFile.close();
		YAML::Node textureData = YAML::Load(sstream.str());

		RLS_ASSERT(textureData["Texture"], "Texture node not present in YAML buffer.");

		auto texture = textureData["Texture"];
		std::string GUIDAsString = texture["GUID"].as<std::string>();
		std::wstring GUIDAsWString = ConvertStringToWstring(GUIDAsString);
		GUID uuid;
		IIDFromString(GUIDAsWString.c_str(), &uuid);
		std::string textureName = texture["Name"].as<std::string>();

		//TODO: Load various texture types!
		const TextureHandle textureHandle = AssetManager::LoadWithUUID<Texture2D>(uuid, fullPath);

		RLS_CORE_INFO("Deserialized texture \"{0}\"", textureName);

		return textureHandle;
	}
}