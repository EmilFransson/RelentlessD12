#include "AssetThumbnailPool.h"
#include "../Assets/Factory/TextureFactory.h"
#include "../Core/Editor.h"
#include "../Subsystem/AssetDefinitionRegistry.h"

namespace Relentless
{
	AssetThumbnailPool::AssetThumbnailPool() noexcept
	{
		const String file = "default_thumbnail.rthumb";
		const Path thumbnailPath = FilepathUtils::Combine(ENGINE_ASSET_DIRECTORY, "Textures\\" + file);

		LoadArchive archive(thumbnailPath, EArchiveFormat::Binary);
		RLS_VERIFY(archive.IsValid(), "[AssetThumbnailPool::AssetThumbnailPool]: Default thumbnail file (default_thumbnail.rthumb) is missing or invalid.");

		m_pDefaultThumbnail = new Thumbnail();
		RLS_VERIFY(m_pDefaultThumbnail->Load(archive), "[AssetThumbnailPool::AssetThumbnailPool]: Failed to load default thumbnail file.");
		
		m_ResidentBytes += archive.ProcessedBytes();
	}

	Ref<Thumbnail> AssetThumbnailPool::GetAssetThumbnail(const AssetData& aAssetData) noexcept
	{
		if (Ref<Thumbnail> pThumbnail = TryFindThumbnail(aAssetData))
			return pThumbnail;

		if (Ref<Thumbnail> pThumbnail = TryLoadThumbnail(aAssetData))
			return pThumbnail;

		EnqueueRequest(aAssetData);

		return GenerateDefaultForAsset(aAssetData);
	}

	void AssetThumbnailPool::InvalidateAssetThumbnail(const AssetData& aAssetData) noexcept
	{
		auto it = m_Thumbnails.find(aAssetData.Uuid);
		if (it == m_Thumbnails.end())
			return;

		ThumbnailEntry& entry = it->second;
		RLS_ASSERT(static_cast<int>(m_ResidentBytes) - static_cast<int>(entry.ResidentBytes) >= 0, "[AssetThumbnailPool::InvalidateAssetThumbnail]: UB detected - Resident bytes underflow");
		m_ResidentBytes -= entry.ResidentBytes;

		if (entry.Resident)
			m_LRU.erase(entry.LRUIterator);

		m_Thumbnails.erase(it);
	}

	void AssetThumbnailPool::OnUpdate() noexcept
	{
		if (m_ResidentBytes > RESIDENT_BYTES_CAPACITY)
			Evict();

		if (m_ThumbnailRegenerateRequests.empty())
			return;

		AssetDefinitionRegistry* pAssetDefinitionRegistry = Editor::Get()->GetSubsystem<AssetDefinitionRegistry>();
		RLS_ASSERT(pAssetDefinitionRegistry, "[AssetThumbnailPool::OnUpdate]: Asset Definition Registry is invalid.");

		const TimeStamp start = Time::GetCurrentTimePoint();
		constexpr float budgetMs = 8.0f;

		while (!m_ThumbnailRegenerateRequests.empty())
		{
			const float elapsedMs = Time::GetElapsedMsSince(start);
			if (elapsedMs > budgetMs)
				break;

			const ThumbnailRegenerateRequest& request = m_ThumbnailRegenerateRequests.top();
			if (const Ref<IAssetDefinition>& pAssetDefinition = pAssetDefinitionRegistry->GetDefinitionForAsset(request.Data))
			{
				pAssetDefinition->RequestGenerateThumbnail(request.Data, [this, data = request.Data, generation = request.Generation](const Ref<Texture2D>& aTexture)
					{
						ThreadPool& threadPool = Application::Get().GetThreadPool();
						threadPool.Submit([this, request = std::move(data), texture = aTexture, generation]()
							{
								CacheThumbnail(std::move(request), std::move(texture), generation);
							});
					});
			}
			
			m_ThumbnailRegenerateRequests.pop();
		}
	}

	void AssetThumbnailPool::RegenerateAssetThumbnail(const AssetData& aAssetData) noexcept
	{
		InvalidateAssetThumbnail(aAssetData);
		EnqueueRequest(aAssetData);
	}

	void AssetThumbnailPool::CacheThumbnail(const AssetData& aAssetData, Ref<Texture2D> aTexture2D, uint32 aGeneration) noexcept
	{
		if (!aTexture2D)
			return;

		const Path path = GenerateThumbnailPath(aAssetData);

		SaveArchive archive(path, EArchiveFormat::Binary);
		
		if (!archive.IsValid())
			return;

		if (!aTexture2D->SerializeBulk(archive))
			return;

		if (!archive.Flush())
			return;

		const uint64 residentBytes = archive.ProcessedBytes();
		Ref<Thumbnail> pThumbnail = RLS_NEW Thumbnail();
		pThumbnail->SetResource(aTexture2D->GetResource());

		Application::Get().SubmitToMainThread([this, aAssetData, pThumbnail, residentBytes, aGeneration]()
			{
				if (m_ThumbnailGenerations[aAssetData.Uuid] != aGeneration)
					return;

				CreateThumbnailEntry(aAssetData, pThumbnail, residentBytes); // Will naturally overwrite
				ResolveThumbnailInfo(pThumbnail, aAssetData);
				OnThumbnailRegenerated(aAssetData, pThumbnail);
			});
	}

	void AssetThumbnailPool::CreateThumbnailEntry(const AssetData& aAssetData, const Ref<Thumbnail>& aThumbnail, uint64 aResidentBytes) noexcept
	{
		ThumbnailEntry& entry = m_Thumbnails[aAssetData.Uuid];

		//If Already existing
		if (entry.ResidentBytes != 0)
			m_ResidentBytes -= entry.ResidentBytes;

		entry.Thumbnail = aThumbnail;
		entry.ResidentBytes = aResidentBytes;

		TouchLRU(aAssetData.Uuid, entry);

		m_ResidentBytes += aResidentBytes;
	}

	void AssetThumbnailPool::EnqueueRequest(const AssetData& aAssetData) noexcept
	{
		const uint32 gen = ++m_ThumbnailGenerations[aAssetData.Uuid];
		m_ThumbnailRegenerateRequests.push(ThumbnailRegenerateRequest{ .Data = aAssetData, .EnqueuedTimeStamp = Time::GetCurrentTimeStamp(), .Generation = gen });
	}

	void AssetThumbnailPool::Evict() noexcept
	{
		constexpr int MaxEvictPerTick = 32;
		int evicted = 0;

		// Target 90% of cap, but keep lenient (dependent on MaxEvictsPerTick, resident bytes of thumbs etc)
		const uint64 targetBudget = static_cast<uint64>(RESIDENT_BYTES_CAPACITY * 0.9f); 

		while (evicted < MaxEvictPerTick && m_ResidentBytes > targetBudget && !m_LRU.empty())
		{
			const UUID& toEvict = m_LRU.back();
			
			auto it = m_Thumbnails.find(toEvict);
			if (it == m_Thumbnails.end())
			{
				m_LRU.pop_back();
				continue;
			}

			m_ResidentBytes -= it->second.ResidentBytes;

			m_LRU.erase(it->second.LRUIterator);
			m_Thumbnails.erase(toEvict);

			++evicted;
		}
	}

	Ref<Thumbnail> AssetThumbnailPool::TryFindThumbnail(const AssetData& aAssetData) noexcept
	{
		if (auto it = m_Thumbnails.find(aAssetData.Uuid); it != m_Thumbnails.end())
		{
			TouchLRU(aAssetData.Uuid, it->second);
			return it->second.Thumbnail;
		}

		return nullptr;
	}

	Ref<Thumbnail> AssetThumbnailPool::GenerateDefaultForAsset(const AssetData& aAssetData) noexcept
	{
		//Default resource shared, hence don't increase resident bytes (already done once)

		Ref<Thumbnail> pDefault = new Thumbnail();
		pDefault->SetResource(m_pDefaultThumbnail->GetResource());

		CreateThumbnailEntry(aAssetData, pDefault, 0u);
		ResolveThumbnailInfo(pDefault, aAssetData);

		return pDefault;
	}

	Path AssetThumbnailPool::GenerateThumbnailPath(const AssetData& aAssetData) noexcept
	{
		const String file = std::format("{:016X}.rthumb", static_cast<uint64_t>(std::hash<UUID>{}(aAssetData.Uuid)));
		const Path thumbnailPath = FilepathUtils::Combine(Project::GetThumbnailCacheDirectory(), file);

		return thumbnailPath;
	}

	void AssetThumbnailPool::ResolveThumbnailInfo(const Ref<Thumbnail>& aThumbnail, const AssetData& aAssetData) noexcept
	{
		AssetDefinitionRegistry* pAssetDefinitionRegistry = Editor::Get()->GetSubsystem<AssetDefinitionRegistry>();
		RLS_ASSERT(pAssetDefinitionRegistry, "[AssetThumbnailPool::ResolveThumbnailInfo]: Asset definition registry is invalid.");

		const Ref<IAssetDefinition>& pAssetDefinition = pAssetDefinitionRegistry->GetDefinitionForAsset(aAssetData);
		if (!pAssetDefinition)
			return;

		aThumbnail->SetInfo(pAssetDefinition->GetThumbnailInfo(aAssetData));
	}

	void AssetThumbnailPool::TouchLRU(const UUID& aUUID, ThumbnailEntry& aThumbnailEntry) noexcept
	{
		if (!aThumbnailEntry.Resident)
		{
			m_LRU.push_front(aUUID);
			aThumbnailEntry.LRUIterator = m_LRU.begin();
			aThumbnailEntry.Resident = true;
		}

		if (aThumbnailEntry.LRUIterator != m_LRU.begin())
			m_LRU.splice(m_LRU.begin(), m_LRU, aThumbnailEntry.LRUIterator);
	}

	Ref<Thumbnail> AssetThumbnailPool::TryLoadThumbnail(const AssetData& aAssetData) noexcept
	{
		const Path path = GenerateThumbnailPath(aAssetData);

		if (!File::Exists(path))
			return nullptr;

		LoadArchive archive(path, EArchiveFormat::Binary);
		if (!archive.IsValid())
			return nullptr;

		Ref<Thumbnail> pThumbnail = RLS_NEW Thumbnail();
		if (!pThumbnail->Load(archive))
			return nullptr;

		CreateThumbnailEntry(aAssetData, pThumbnail, archive.ProcessedBytes());
		ResolveThumbnailInfo(pThumbnail, aAssetData);

		return pThumbnail;
	}
}
