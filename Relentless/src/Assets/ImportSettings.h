#pragma once
#include "AssetMeta.h"
#include "Scene/Scene.h"
#include "Mesh/Mesh.h"

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
		ETextureCompressionType TextureCompressionType{ ETextureCompressionType::Uncompressed };
		bool ImportLights{ false };
		bool ImportMaterialsAndTextures{ true };
		bool GenerateColliders{ true };
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
	struct AssetTypeInfo<TextureEx>
	{
		using Settings = TextureImportSettings;
	};

	template<>
	struct AssetTypeInfo<Mesh>
	{
		using Settings = MeshImportSettings;
	};
}