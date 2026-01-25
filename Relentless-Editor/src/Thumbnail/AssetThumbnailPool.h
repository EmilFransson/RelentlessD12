#pragma once
#include <Relentless.h>
#include "../UI/Widgets/Thumbnail.h"

namespace Relentless
{
	struct ThumbnailRegenerateRequest
	{
		AssetData Data;
		TimeStamp EnqueuedTimeStamp;
		uint32 Generation = 0;
	};

	struct ThumbnailEntry
	{
		Ref<Thumbnail> Thumbnail;
		bool Resident = false;
		std::list<UUID>::iterator LRUIterator;
		uint64 ResidentBytes = 0u;
	};

	class AssetThumbnailPool
	{
	public:
		constexpr static uint64 RESIDENT_BYTES_CAPACITY = 128u * 1024u * 1024u; //128 mb

		AssetThumbnailPool() noexcept;

		NO_DISCARD Ref<Thumbnail> GetAssetThumbnail(const AssetData& aAssetData) noexcept;
		
		void InvalidateAssetThumbnail(const AssetData& aAssetData) noexcept;

		void OnUpdate() noexcept;

		void RegenerateAssetThumbnail(const AssetData& aAssetData) noexcept;

		Broadcaster<void(const AssetData& aAsset, const Ref<Thumbnail>& aThumbnail)> OnThumbnailRegenerated;
	private:
		void CacheThumbnail(const AssetData& aAssetData, Ref<Texture2D> aTexture2D, uint32 aGeneration) noexcept;
		void CreateThumbnailEntry(const AssetData& aAssetData, const Ref<Thumbnail>& aThumbnail, uint64 aResidentBytes) noexcept;

		void EnqueueRequest(const AssetData& aAssetData) noexcept;
		void Evict() noexcept;

		NO_DISCARD Ref<Thumbnail> TryFindThumbnail(const AssetData& aAssetData) noexcept;
		NO_DISCARD Ref<Thumbnail> TryLoadThumbnail(const AssetData& aAssetData) noexcept;

		NO_DISCARD Ref<Thumbnail> GenerateDefaultForAsset(const AssetData& aAssetData) noexcept;
		NO_DISCARD Path GenerateThumbnailPath(const AssetData& aAssetData) noexcept;

		void ResolveThumbnailInfo(const Ref<Thumbnail>& aThumbnail, const AssetData& aAssetData) noexcept;

		void TouchLRU(const UUID& aUUID, ThumbnailEntry& aEntry) noexcept;
	private:
		std::unordered_map<UUID, ThumbnailEntry> m_Thumbnails;
		std::unordered_map<UUID, uint32> m_ThumbnailGenerations;

		std::list<UUID> m_LRU;

		inline static auto NewestFirst = [](const ThumbnailRegenerateRequest& a, const ThumbnailRegenerateRequest& b) noexcept
			{
				return a.EnqueuedTimeStamp < b.EnqueuedTimeStamp;
			};

		std::priority_queue<ThumbnailRegenerateRequest, std::vector<ThumbnailRegenerateRequest>, decltype(NewestFirst)> m_ThumbnailRegenerateRequests;

		Ref<Thumbnail> m_pDefaultThumbnail = nullptr;
		uint64 m_ResidentBytes = 0u;
	};
}