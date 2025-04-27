#pragma once
#include "Assets/AssetMeta.h"
#include "Assets/ImportSettings.h"
#include "Callback/Broadcaster.h"
#include "Core/IAsset.h"
#include "IFactory.h"

#include "../../../vendor/includes/Assimp/Importer.hpp"
#include "../../../vendor/includes/Assimp/postprocess.h"
#include "../../../vendor/includes/Assimp/scene.h"

namespace Relentless
{
	class GraphicsDevice;

	struct ImportedAsset
	{
		IAsset* pAsset = nullptr;
		AssetHandle Handle = NULL_HANDLE;
		AssetType Type = AssetType::Undefined;
	};

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

		Broadcaster<void(const std::vector<ImportedAsset>& importedAssets, bool success)> OnDone;
		Broadcaster<void(float progress)> OnProgressIncreased;
	private:
		virtual void Execute(const Path& filePath, GraphicsDevice* pDevice) noexcept override;
		void Finalize(bool succeeded) noexcept;
		void ImportMaterials() noexcept;
		void ImportModel() noexcept;
		void ImportTextures() noexcept;
		[[nodiscard]] bool ImportTexture(const Path& absolutePath, bool srgb, Ref<Texture>& pOutTexture, AssetHandle& outAssetHandle);
		void ImportMeshes() noexcept;
		[[nodiscard]] bool ImportMesh(const aiMesh* pMesh, Ref<Mesh>& pOutMesh, AssetHandle& outHandle) noexcept;
		void IncreaseProgress() noexcept;
		[[nodiscard]] bool InitializeImporter() noexcept;
		bool ParseModel() noexcept;
		void ParseMaterialsAndTextures() noexcept;
		void ParseMeshes() noexcept;
		void ResolveSceneNodeHierarchy() noexcept;
		void SetProgress(float progress) noexcept;
		void StoreImportedAsset(const ImportedAsset& asset) noexcept;
	private:
		std::vector<ImportedAsset> m_ImportedAssets;
		
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
	};
}