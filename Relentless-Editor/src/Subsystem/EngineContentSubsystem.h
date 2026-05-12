#pragma once 
#include <Relentless.h>

namespace Relentless
{
	template<typename AssetType>
	struct EngineAssetEntry
	{
		AssetHandle Handle		= AssetHandle::INVALID;
		Ref<AssetType> Asset	= nullptr;
	};

	class EngineContentSubsystem : public ISubsystem
	{
	public:
		NO_DISCARD const AssetHandle& GetCubeMeshHandle() const noexcept;
		NO_DISCARD const AssetHandle& GetSphereMeshHandle() const noexcept;
		NO_DISCARD const AssetHandle& GetMaterialPreviewCubemapHandle() const noexcept;
		NO_DISCARD const AssetHandle& GetNoneTexture2DHandle() const noexcept;

		NO_DISCARD bool IsLoading() const noexcept;

		NO_DISCARD bool OnLoad(MAYBE_UNUSED ISystemManager* aSystemManager) noexcept override;

		static bool ShouldCreateSubsystem(ISystemManager* aSystemManager) noexcept;
	private:
		template<typename AssetType>
		void RequestAsyncLoad(const String& aPath, EngineAssetEntry<AssetType>& aOutAssetEntry) noexcept;
	private:
		EngineAssetEntry<TextureCube> m_MaterialPreviewEnvironment;
		EngineAssetEntry<Texture2D> m_NoneThumbnail;
		EngineAssetEntry<Mesh> m_SphereMesh;
		EngineAssetEntry<Mesh> m_CubeMesh;

		uint32 m_NumAssetsLoading = 0u;
	};

	template<typename AssetType>
	void EngineContentSubsystem::RequestAsyncLoad(const String& aPath, EngineAssetEntry<AssetType>& aOutAssetEntry) noexcept
	{
		m_NumAssetsLoading++;

		AssetManager::LoadAssetAsync(aPath, [this, &aOutAssetEntry, aPath](const AssetHandle& aAssetHandle)
			{
				RLS_VERIFY(aAssetHandle.IsValid(), std::format("[EngineContentSubsystem::OnLoad]: Failed to load asset '{}'", aPath));
				aOutAssetEntry.Handle = aAssetHandle;
				aOutAssetEntry.Asset = AssetManager::Get<AssetType>(aAssetHandle);
				m_NumAssetsLoading--;
			});
	}
}