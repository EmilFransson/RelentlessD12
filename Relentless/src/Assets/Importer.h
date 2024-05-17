#pragma once
#include "ECS/ECSCommon.h"
#include "ImportSettings.h"

struct aiNode;
struct aiScene;
struct aiMesh;

namespace DirectX
{
	class ScratchImage;
}

namespace Relentless
{
	//Forward declares:
	class Mesh;
	class Texture2D;
	class Material;
	class Scene;
	struct AssetHandle;
	struct TextureImportSettings;
	struct MeshImportSettings;

	enum class ExtensionType : uint8_t { UNKNOWN = 0, TGA, JPG, PNG, BMP, DDS, TIFF, FBX, OBJ, GLTF };

	struct ModelDependencies
	{
		std::set<std::filesystem::path> UniqueTextures;
		std::set<std::filesystem::path> UniqueMeshes;
	};

	using AssetImportSettingsVariant = std::variant<TextureImportSettings, MeshImportSettings>;

	class Importer
	{
	public:
		static void RequestAsyncLoadFromFile(const std::filesystem::path& filepath, const std::filesystem::path& dstAssetDirectorPath, const std::optional<AssetImportSettingsVariant>& optionalImportSettings = std::nullopt) noexcept;
		static [[nodiscard]] bool ImportTexture(const std::filesystem::path& fullPath, const std::filesystem::path& dstAssetDirectorPath, const TextureImportSettings& importSettings = {}) noexcept;
		static [[nodiscard]] bool ImportModel(const std::filesystem::path& fullPath, const std::filesystem::path& dstAssetDirectorPath, const MeshImportSettings& importSettings = {}) noexcept;

		template<typename T>
		struct always_false : std::false_type{};

		template<typename AssetType>
		static [[nodiscard]] typename std::enable_if<!std::is_same<AssetType, Mesh>::value, AssetType>::type
		Import(const std::filesystem::path& fullPath, const typename AssetTypeInfo<AssetType>::Settings& importSettings = {}) noexcept
		{
			static_assert(always_false<AssetType>::value, "This operation is not supported by the type.");
		}

		template<typename AssetType>
		[[nodiscard]] static typename std::enable_if<std::is_same<AssetType, Mesh>::value, void>::type
		Import(const std::filesystem::path& fullPath, const std::string& destinationDirectory = std::string::empty(), const typename AssetTypeInfo<AssetType>::Settings& importSettings = {}) noexcept
		{
			static_assert(always_false<AssetType>::value, "This operation is not supported by the type.");
		}

		static [[nodiscard]] ExtensionType GetExtensionTypeFromPath(const std::filesystem::path& fullPath) noexcept;
	private:
		static [[nodiscard]] DXGI_FORMAT GetCompressedDXGITextureFormat(const TextureImportSettings& importSettings) noexcept;
		static [[nodiscard]] Microsoft::WRL::ComPtr<ID3D12Resource> CreateAndUploadTexture2DFromImage(const DirectX::ScratchImage& image) noexcept;
		//static void ProcessAssimpNode(aiNode* pNode, const aiScene* pAssimpScene, const MeshImportSettings& importSettings, const DirectX::XMFLOAT4X4& transform, const std::filesystem::path& workingDirectory, const std::string& destinationDirectory = "", entity parent = NULL_ENTITY) noexcept;
		//static AssetHandle ProcessMaterial(aiMesh* pMesh, const aiScene* pAssimpScene, const std::filesystem::path& workingDirectory, const std::string& destinationDirectory = "") noexcept;
		//static AssetHandle ProcessMesh(aiMesh* pMesh, const std::filesystem::path& workingDirectory, const std::string& destinationDirectory = "") noexcept;
	
		template<typename T>
		static T CastToImportSettings(const std::optional<AssetImportSettingsVariant>& optionalImportSettings) noexcept
		{
			return optionalImportSettings.has_value() ?
				std::get_if<T>(&(*optionalImportSettings)) ? *std::get_if<T>(&(*optionalImportSettings)) : T()
				: T();
		}
	private:
		static std::unordered_map<AssetType, std::function<bool(const std::filesystem::path&, const std::filesystem::path&, const std::optional<AssetImportSettingsVariant>&)>> m_LoadFuncs;
	};

	//template<>
	//void Importer::Import<Mesh>(const std::filesystem::path& fullPath, const std::string& destinationDirectory, const AssetTypeInfo<Mesh>::Settings& importSettings) noexcept;
}