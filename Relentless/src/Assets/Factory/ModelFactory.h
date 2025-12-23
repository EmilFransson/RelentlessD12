#pragma once
#include "Callback/Broadcaster.h"
#include "Graphics/RHI/RHI.h"
#include "IFactory.h"

#include "../../../vendor/includes/Assimp/Importer.hpp"
#include "../../../vendor/includes/Assimp/postprocess.h"
#include "../../../vendor/includes/Assimp/scene.h"

namespace Relentless
{
	class Texture2D;

	class ModelFactory : public IFactory
	{
	public:
		struct MeshImportInfo
		{
			const aiMesh* pMesh = nullptr;

			AssetHandle HandleToImportedMesh = NULL_HANDLE;
		};

		struct TextureImportInfo
		{
			Path AbsolutePath;
			aiTextureType Type;
			bool IsSRGB = false;

			AssetHandle HandleToImportedTexture = NULL_HANDLE;
		};

		struct MaterialImportInfo
		{
			const aiMaterial* pMaterial = nullptr;
			std::vector<TextureImportInfo> TextureDependencies;

			String Name;
			Vector4 DiffuseColor = Vector4::One;
			float Opacity = 1.0f;
			bool TwoSided = false;

			AssetHandle HandleToImportedMaterial = NULL_HANDLE;
		};

		~ModelFactory();

		void SetGenerateCollisionMeshes(bool enable) noexcept;
		void SetGenerateTextureMipmaps(bool enable) noexcept;
		void SetOptimizeMeshes(bool enable) noexcept;
		void SetImportMaterialsAndTextures(bool enable) noexcept;
		void SetTextureCompressionType(ETextureCompressionType compressionType) noexcept;

		NO_DISCARD bool CanCreateNew() const noexcept override;
		NO_DISCARD bool CanImport(const Path& aPath) const noexcept override;
		virtual Ref<IFactory> Clone() noexcept override;

		NO_DISCARD bool DoesSupportAsset(IAsset* aAsset) const noexcept override;

		NO_DISCARD std::vector<String> GetSupportedFileExtensions() const noexcept override;
		NO_DISCARD std::vector<String> GetFormats() const noexcept override;

		virtual const FactoryImportResult& ImportFromFile(const Path& aPath, const Path& aPackagePath, const String& aName, Ref<FeedbackContext> aFeedbackContext = nullptr) noexcept override;

		void SetGraphicsDevice(GraphicsDevice* aGraphicsDevice) noexcept;	
		NO_DISCARD bool SupportsFileExtension(const std::string_view aFileExtension) const noexcept override;
	private:
		//virtual void Execute(const Path& filePath, GraphicsDevice* pDevice) noexcept override;
		void Finalize(bool succeeded) noexcept;
		void ImportMaterials() noexcept;
		void ImportModel() noexcept;
		void ImportTextures() noexcept;
		[[nodiscard]] bool ImportTexture(const Path& absolutePath, bool srgb, Ref<Texture2D>& pOutTexture, AssetHandle& outAssetHandle) noexcept;
		void ImportMeshes() noexcept;
		[[nodiscard]] bool ImportMesh(const aiMesh* pMesh, Ref<Mesh>& pOutMesh, AssetHandle& outHandle) noexcept;
		void IncreaseProgress() noexcept;
		[[nodiscard]] bool InitializeImporter() noexcept;
		bool ParseModel() noexcept;
		void ParseMaterialsAndTextures() noexcept;
		void ParseMeshes() noexcept;
		void ResolveSceneNodeHierarchy() noexcept;
		void SetProgress(float progress) noexcept;
		void StoreImportedAsset(const FactoryImportResult& asset) noexcept;
	private:
		//std::vector<ImportedAsset> m_ImportedAssets;
		std::array<String, 4> m_SupportedExtensions = { ".fbx", ".gltf", ".glb", ".obj" };
		std::array<String, 4> m_SupportedFormats = { "FBX", "GLTF", "GLB", "OBJ" };

		//Embedded:
		std::vector<MaterialImportInfo> m_UniqueMaterials;
		std::vector<MeshImportInfo> m_UniqueMeshes;
		
		//External:
		std::vector<TextureImportInfo> m_UniqueTextures;
	
		std::mutex m_ImportAssetMutex;
		std::mutex m_ProgressionMutex;

		Path m_MainModelPath;

		UniquePtr<Assimp::Importer> m_pImporter = nullptr;
		const aiScene* m_pScene = nullptr;
		GraphicsDevice* m_pDevice = nullptr;

		float m_Progress = 0.0f;
		float m_ProgressPerAsset = 1.0f;

		ETextureCompressionType m_TextureCompressionType = ETextureCompressionType::Uncompressed;

		bool m_GenerateCollisionMeshes = true;
		bool m_OptimizeMeshes = true;
		bool m_ImportMaterialsAndTextures = true;
		bool m_GenerateTextureMipmaps = true;
		bool m_MainAssetDone = false;
	};
}