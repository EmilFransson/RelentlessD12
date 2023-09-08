#pragma once
#include "Material.h"
#include "../../Mesh/Mesh.h"
#include "TextureManager.h"
#include "TextureSerializer.h"
#include "MaterialSerializer.h"

namespace Relentless
{
	inline static std::mutex g_MainCreateMutex;

	class AssetManager
	{
	private:
		AssetManager() noexcept = default;
		~AssetManager() noexcept = default;

		struct Data
		{
			MaterialManager MaterialManager;
			MeshManager MeshManager;
			TextureManager TextureManager;
			std::unordered_map<UUID, std::string> UUIDToPathMap;
			std::unordered_map<std::string, UUID> PathToUUIDMap;
		};

		static Data s_Data;

		template<typename>
		struct always_false : std::false_type {};
	public:
		static void Initialize() noexcept;
		[[nodiscard]] static MaterialManager& GetMaterialManager() noexcept;
		[[nodiscard]] static MeshManager& GetMeshManager() noexcept;
		[[nodiscard]] static TextureManager& GetTextureManager() noexcept;
		[[nodiscard]] static const std::string& GetAssetPath(const AssetHandle& assetHandle) noexcept;
		static void MapGUIDToFilepath(const UUID& uuid, const std::string& path) noexcept;
		static void DeleteGUIDToFilepathMapping(const UUID& uuid, const std::string& path) noexcept;

		template<typename AssetType>
		[[nodiscard]] static AssetType& Get(const AssetHandle& AssetHandle) noexcept
		{
			static_assert(always_false<AssetType>::value, "This operation is not supported by the type, or the type does not exist.");
		}

		template<typename AssetType>
		[[nodiscard]] static bool Exists(const std::string& assetName) noexcept
		{
			static_assert(always_false<AssetType>::value, "This operation is not supported by the type, or the type does not exist.");
		}

		template<typename AssetType>
		static AssetHandle Create(const std::string& context) noexcept
		{
			static_assert(always_false<AssetType>::value, "This operation is not supported by the type, or the type does not exist.");
		}

		template<typename AssetType>
		static AssetHandle CreateWithUUID(const UUID& uuid, const std::filesystem::path& fullPath, const AssetType& assetType = AssetType()) noexcept
		{
			static_assert(always_false<AssetType>::value, "This operation is not supported by the type, or the type does not exist.");
		}

		template<typename AssetType>
		static AssetHandle Load(const std::filesystem::path& fullPath) noexcept
		{
			static_assert(always_false<AssetType>::value, "This operation is not supported by the type, or the type does not exist.");
		}

		template<typename AssetType>
		static AssetHandle LoadWithUUID(const UUID& uuid, const std::filesystem::path& fullPath) noexcept
		{
			static_assert(always_false<AssetType>::value, "This operation is not supported by the type, or the type does not exist.");
		}
	private:
		static void EstablishGUIDToFilepathMapping(const std::filesystem::path& startingDirectory) noexcept;
	};

	template<>
	inline Material& AssetManager::Get<Material>(const MaterialHandle& materialHandle) noexcept
	{
		return s_Data.MaterialManager.GetMaterial(materialHandle);
	}

	template<>
	inline Mesh& AssetManager::Get<Mesh>(const MeshHandle& materialHandle) noexcept
	{
		return s_Data.MeshManager.GetMesh(materialHandle);
	}

	template<>
	inline Texture2D& AssetManager::Get<Texture2D>(const TextureHandle& textureHandle) noexcept
	{
		return s_Data.TextureManager.GetTexture(textureHandle);
	}

	template<>
	inline bool AssetManager::Exists<Material>(const std::string& assetName) noexcept
	{
		return s_Data.MaterialManager.Exists(assetName);
	}

	template<>
	inline bool AssetManager::Exists<Mesh>(const std::string& assetName) noexcept
	{
		return s_Data.MeshManager.Exists(assetName);
	}

	template<>
	inline bool AssetManager::Exists<Texture2D>(const std::string& assetName) noexcept
	{
		return s_Data.TextureManager.Exists(assetName);
	}

	template<>
	inline AssetHandle AssetManager::Create<Material>(const std::string& name) noexcept
	{
		return s_Data.MaterialManager.Create(name);
	}

	template<>
	inline AssetHandle AssetManager::CreateWithUUID<Material>(const UUID& uuid, const std::filesystem::path& fullPath, const Material& material) noexcept
	{
		const std::lock_guard<std::mutex> lock(g_MainCreateMutex);

		RLS_ASSERT(std::filesystem::exists(fullPath), "Path is invalid.");
		s_Data.UUIDToPathMap[uuid] = fullPath.string();
		
		return s_Data.MaterialManager.CreateWithUUID(uuid, fullPath.filename().string(), material);
	}

	template<>
	inline AssetHandle AssetManager::Load<Texture2D>(const std::filesystem::path& fullPath) noexcept
	{
		const std::lock_guard<std::mutex> lock(g_MainCreateMutex);

		RLS_ASSERT(std::filesystem::exists(fullPath), "Path is invalid");

		if (s_Data.TextureManager.Exists(fullPath.string())) 
		{
			return s_Data.TextureManager.GetTextureHandleByString(fullPath.string());
		}
		else if (s_Data.PathToUUIDMap.contains(fullPath.string() + ".rasset"))
		{
			return TextureSerializer::Deserialize(fullPath.string());
		}

		UUID uuid = TextureSerializer::SerializeDefault(fullPath.string());
		TextureHandle textureHandle = s_Data.TextureManager.LoadTextureFromFile(fullPath.string(), uuid);
		s_Data.UUIDToPathMap[textureHandle.UUID] = fullPath.string() + ".rasset";
		s_Data.PathToUUIDMap[fullPath.string() + ".rasset"] = textureHandle.UUID;
		return textureHandle;
	}

	template<>
	inline AssetHandle AssetManager::Load<Material>(const std::filesystem::path& fullPath) noexcept
	{
		const std::lock_guard<std::mutex> lock(g_MainCreateMutex);

		if (s_Data.MaterialManager.Exists(fullPath.filename().stem().string()))
		{
			return s_Data.MaterialManager.GetMaterialHandleByName(fullPath.stem().string());
		}
		else if (s_Data.PathToUUIDMap.contains(fullPath.string() + ".rasset"))
		{
			return MaterialSerializer::Deserialize(fullPath.string());
		}

		//TODO: Generate the rmat in SerializeDefault
		UUID uuid = MaterialSerializer::SerializeDefault(fullPath.string());
		MaterialHandle materialHandle = s_Data.MaterialManager.CreateWithUUID(uuid, fullPath.filename().stem().string());
		s_Data.UUIDToPathMap[materialHandle.UUID] = fullPath.string() + ".rmat" + ".rasset";
		s_Data.PathToUUIDMap[fullPath.string() + ".rmat" + ".rasset"] = materialHandle.UUID;
		return materialHandle;
	}

	template<>
	inline AssetHandle AssetManager::LoadWithUUID<Texture2D>(const UUID& uuid, const std::filesystem::path& fullPath) noexcept
	{
		const std::lock_guard<std::mutex> lock(g_MainCreateMutex);

		RLS_ASSERT(std::filesystem::exists(fullPath), "Path is invalid");
		s_Data.UUIDToPathMap[uuid] = fullPath.string();

		if (s_Data.TextureManager.Exists(fullPath.string()))
		{
			return s_Data.TextureManager.GetTextureHandleByString(fullPath.string());
		}

		return s_Data.TextureManager.LoadTextureFromFile(fullPath.string(), uuid);
	}
}