#pragma once
#include "Assets/Factory/IFactory.h"
#include "AssetMeta.h"
#include "Scene/Scene.h"
#include "Mesh/Mesh.h"

namespace Relentless
{
	class IFactory;

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
	
	struct AssetImportTask
	{
		Path FilePath;
		Ref<IFactory> pFactory = nullptr;
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
	struct AssetTypeInfo<Texture>
	{
		using Settings = TextureImportSettings;
	};

	template<>
	struct AssetTypeInfo<Mesh>
	{
		using Settings = MeshImportSettings;
	};
}