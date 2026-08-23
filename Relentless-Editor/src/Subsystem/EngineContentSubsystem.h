#pragma once 
#include <Relentless.h>

namespace Relentless
{
	enum class EEngineAsset : uint32
	{
		#define RLS_ENGINE_ASSET(InId, InType, InPath) InId,
		#include "EngineContent.inl"
		#undef RLS_ENGINE_ASSET
		Count
	};

	template<EEngineAsset Id> struct EngineAssetTraits;

	#define RLS_ENGINE_ASSET(InId, InType, InPath)                 \
        template<> struct EngineAssetTraits<EEngineAsset::InId>    \
        {                                                          \
            using AssetType = InType;                              \
            static constexpr const char* Path = InPath;            \
        };
		#include "EngineContent.inl"
		#undef RLS_ENGINE_ASSET

	struct EngineAssetEntry
	{
		AssetHandle Handle = AssetHandle::INVALID;
		Ref<IAsset> Asset = nullptr;
	};

	class EngineContentSubsystem : public ISubsystem
	{
	public:
		template<EEngineAsset Id>
		NO_DISCARD Ref<typename EngineAssetTraits<Id>::AssetType> GetAsset() const noexcept
		{
			using T = typename EngineAssetTraits<Id>::AssetType;
			return std::static_pointer_cast<T>(m_Assets[(uint32)Id].Asset);
		}

		NO_DISCARD const AssetHandle& GetAssetHandle(EEngineAsset aAsset) const noexcept
		{
			return m_Assets[(uint32)aAsset].Handle;
		}

		NO_DISCARD bool IsLoading() const noexcept;

		NO_DISCARD bool OnLoad(MAYBE_UNUSED ISystemManager* aSystemManager) noexcept override;

		static bool ShouldCreateSubsystem(ISystemManager* aSystemManager) noexcept;
	private:
		template<typename AssetType>
		void RequestAsyncLoad(const String& aPath, EngineAssetEntry& aOutAssetEntry) noexcept;
	private:
		std::array<EngineAssetEntry, (uint32)EEngineAsset::Count> m_Assets;

		uint32 m_NumAssetsLoading = 0u;
	};

	template<typename AssetType>
	void EngineContentSubsystem::RequestAsyncLoad(const String& aPath, EngineAssetEntry& aOutAssetEntry) noexcept
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