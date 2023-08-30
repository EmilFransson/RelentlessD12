#pragma once
#include "../MemoryManager.h" //TODO: Perhaps remove?
#include "Material.h"
#include "../../Mesh/Mesh.h"
#include "TextureManager.h"

namespace Relentless
{
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
		};

		static Data s_Data;

		template<typename>
		struct always_false : std::false_type {};
	public:
		static void Initialize() noexcept;
		[[nodiscard]] static MaterialManager& GetMaterialManager() noexcept;
		[[nodiscard]] static MeshManager& GetMeshManager() noexcept;
		[[nodiscard]] static TextureManager& GetTextureManager() noexcept;

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
		static AssetHandle Load(const std::string& context) noexcept
		{
			static_assert(always_false<AssetType>::value, "This operation is not supported by the type, or the type does not exist.");
		}
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
	inline AssetHandle AssetManager::Load<Texture2D>(const std::string& context) noexcept
	{
		if (s_Data.TextureManager.Exists(context))
			return s_Data.TextureManager.GetTextureHandleByString(context);

		return s_Data.TextureManager.LoadTextureFromFile(context);
	}
}