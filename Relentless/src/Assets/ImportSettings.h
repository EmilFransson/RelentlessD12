#pragma once
#include "AssetMeta.h"
#include "Scene/Scene.h"

namespace Relentless
{
	enum class ETextureCompressionType : uint8_t
	{
		Uncompressed = 0u, BC5, BC7, BC7_Quick
	};

	struct TextureImportSettings
	{
		bool GenerateMipMaps{ true };
		bool IsSRGB{ true };
		bool IsHDR{ false };
		ETextureCompressionType TextureCompressionType{ ETextureCompressionType::Uncompressed };
	};

	struct MeshImportSettings
	{
		bool OptimizeMesh{ true };
		bool ImportLights{ false };
		//bool ImportCameras{ true };
		bool ImportMaterialsAndTextures{ true };
		bool GenerateColliders{ true };
		std::shared_ptr<Scene> pScene{ nullptr };
	};

	template<typename AssetType>
	struct AssetImportSettings
	{
		//Empty for now...
	};

	template<typename AssetType>
	struct AssetTypeInfo
	{
		using Settings = AssetImportSettings<AssetType>;
	};

	template<>
	struct AssetTypeInfo<Texture2D>
	{
		using Settings = TextureImportSettings;
	};

	template<>
	struct AssetTypeInfo<Mesh>
	{
		using Settings = MeshImportSettings;
	};
}