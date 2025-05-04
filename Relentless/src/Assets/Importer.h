#pragma once
#include "ECS/ECSCommon.h"
#include "Graphics/RHI/RHI.h"
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

	enum class ExtensionType : uint8_t { UNKNOWN = 0, TGA, JPG, JPEG, PNG, BMP, DDS, TIFF, HDR, EXR, FBX, OBJ, GLTF };

	//struct ModelDependencies
	//{
	//	std::set<std::filesystem::path> UniqueTextures;
	//	std::set<std::filesystem::path> UniqueMeshes;
	//};

	//using AssetImportSettingsVariant = std::variant<TextureImportSettings, MeshImportSettings>;

	struct ImporterFeedbackContext : public RefCounted<ImporterFeedbackContext>
	{
		Broadcaster<void(const std::string& status)> OnStatusChanged;
		Broadcaster<void(float partialProgress)> OnProgressUpdated;
	};

	struct AssetImportBatch
	{
		std::vector<std::future<void>> Futures;

		void Wait() noexcept
		{
			for (auto& future : Futures)
				future.wait();
		}

		[[nodiscard]] bool IsComplete() const noexcept
		{
			for (auto& future : Futures)
			{
				if (future.wait_for(std::chrono::seconds(0)) != std::future_status::ready)
					return false;
			}

			return true;
		}
	};

	//struct ImportRequest
	//{
	//	std::filesystem::path Filepath;
	//	std::optional<AssetImportSettingsVariant> ImportSettings;
	//	mutable Broadcaster<void(const AssetHandle& handle, bool success)> OnAssetImported;
	//};

	class Importer
	{
	public:
		static Ref<IFactory> CreateDefaultFactory(ExtensionType extensionType) noexcept;

		static AssetImportBatch RequestAsyncLoad(Span<AssetImportTask> importTasks, Ref<ImporterFeedbackContext> pFeedbackContext = nullptr) noexcept;

		//static std::future<void> RequestAsyncLoadFromFile(const std::filesystem::path& filepath, const std::filesystem::path& dstAssetDirectorPath, const std::optional<AssetImportSettingsVariant>& optionalImportSettings = std::nullopt) noexcept;
		//static [[nodiscard]] bool ImportModel(const std::filesystem::path& fullPath, const std::filesystem::path& dstAssetDirectorPath, bool isABlockingOperation, const MeshImportSettings& importSettings = {}) noexcept;

		//static [[nodiscard]] bool ImportModelEx(GraphicsDevice* pDevice, const ImportRequest& request) noexcept;
		//static [[nodiscard]] bool ImportTextureEx(GraphicsDevice* pDevice, const ImportRequest& request) noexcept;
		//
		//template<typename T>
		//struct always_false : std::false_type{};

		//template<typename AssetType>
		//static [[nodiscard]] typename std::enable_if<!std::is_same<AssetType, Mesh>::value, AssetType>::type
		//Import(const std::filesystem::path& fullPath, const typename AssetTypeInfo<AssetType>::Settings& importSettings = {}) noexcept
		//{
		//	static_assert(always_false<AssetType>::value, "This operation is not supported by the type.");
		//}

		//template<typename AssetType>
		//[[nodiscard]] static typename std::enable_if<std::is_same<AssetType, Mesh>::value, void>::type
		//Import(const std::filesystem::path& fullPath, const std::string& destinationDirectory = std::string::empty(), const typename AssetTypeInfo<AssetType>::Settings& importSettings = {}) noexcept
		//{
		//	static_assert(always_false<AssetType>::value, "This operation is not supported by the type.");
		//}

		static [[nodiscard]] ExtensionType GetExtensionTypeFromPath(const std::filesystem::path& fullPath) noexcept;
	private:
		//static [[nodiscard]] DXGI_FORMAT GetCompressedDXGITextureFormat(const TextureImportSettings& importSettings) noexcept;
		
		//static bool ImportAssimpMeshEx(GraphicsDevice* pDevice, aiMesh* pMesh, AssetHandle& outHandle) noexcept;
	};
}