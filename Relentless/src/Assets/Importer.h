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

	class Importer
	{
	public:
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
		Import(const std::filesystem::path& fullPath, const typename AssetTypeInfo<AssetType>::Settings& importSettings = {}) noexcept
		{
			static_assert(always_false<AssetType>::value, "This operation is not supported by the type.");
		}

	private:
		static [[nodiscard]] bool HasImageExtension(const std::string& fileExtension) noexcept;
		static [[nodiscard]] DXGI_FORMAT GetCompressedDXGITextureFormat(const TextureImportSettings& importSettings) noexcept;
		static [[nodiscard]] Microsoft::WRL::ComPtr<ID3D12Resource> CreateAndUploadTexture2DFromImage(const DirectX::ScratchImage& image) noexcept;
		static void ProcessAssimpNode(aiNode* pNode, const aiScene* pAssimpScene, const MeshImportSettings& importSettings, const DirectX::XMFLOAT4X4& transform, const std::filesystem::path& workingDirectory, entity parent = NULL_ENTITY) noexcept;
		static AssetHandle ProcessMaterial(aiMesh* pMesh, const aiScene* pAssimpScene, const std::filesystem::path& workingDirectory) noexcept;
		static AssetHandle ProcessMesh(aiMesh* pMesh, const std::filesystem::path& workingDirectory) noexcept;
	};

	template<>
	Texture2D Importer::Import<Texture2D>(const std::filesystem::path& fullPath, const AssetTypeInfo<Texture2D>::Settings& importSettings) noexcept;

	template<>
	void Importer::Import<Mesh>(const std::filesystem::path& fullPath, const AssetTypeInfo<Mesh>::Settings& importSettings) noexcept;
}