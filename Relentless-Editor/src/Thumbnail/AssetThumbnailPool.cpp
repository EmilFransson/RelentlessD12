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

		m_pDefaultThumbnailTexture = CreateTextureFromArchive(archive);
		RLS_VERIFY(m_pDefaultThumbnailTexture, "[AssetThumbnailPool::AssetThumbnailPool]: Failed to load default thumbnail file.");
		
		m_ResidentBytes += archive.ProcessedBytes();
	}

	Ref<Texture> AssetThumbnailPool::GetAssetThumbnail(const AssetData& aAssetData) noexcept
	{
		if (Ref<Texture> pThumbnailTexture = TryFindThumbnail(aAssetData))
			return pThumbnailTexture;

		if (Ref<Texture> pThumbnailTexture = TryLoadThumbnail(aAssetData))
			return pThumbnailTexture;

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
		
		Application::Get().SubmitToMainThread([this, aAssetData, pThumbnailTexture = aTexture2D->GetResource(), residentBytes, aGeneration]()
			{
				if (m_ThumbnailGenerations[aAssetData.Uuid] != aGeneration)
					return;

				CreateThumbnailEntry(aAssetData, pThumbnailTexture, residentBytes); // Will naturally overwrite
				OnThumbnailRegenerated(aAssetData, pThumbnailTexture);
			});
	}

	Ref<Texture> AssetThumbnailPool::CreateTextureFromArchive(LoadArchive& aArchive)
	{
		DirectX::TexMetadata meta{};

		aArchive.Process(meta.width);
		aArchive.Process(meta.height);
		aArchive.Process(meta.depth);
		aArchive.Process(meta.arraySize);
		aArchive.Process(meta.mipLevels);
		aArchive.Process(meta.format);
		aArchive.Process(meta.dimension);
		aArchive.Process(meta.miscFlags);
		aArchive.Process(meta.miscFlags2);

		uint64 pixelByteSize = 0u;
		aArchive.Process(pixelByteSize);

		RLS_ASSERT(pixelByteSize != 0u, "[AssetThumbnailPool::CreateFromArchive]: Thumbnail texture content is missing.");

		DirectX::ScratchImage scratchImage{};

		if (FAILED(scratchImage.Initialize(meta)))
			return nullptr;

		uint8* pixels = scratchImage.GetPixels();
		if (!pixels)
			return nullptr;

		if (!aArchive.ProcessRaw(pixels, static_cast<size_t>(pixelByteSize)))
			return nullptr;

		if (!aArchive.IsValid())
			return nullptr;

		const String name = FilepathUtils::ExtractFilenameWithoutExtension(aArchive.GetSourcePath()) + "_thumbnail";
		return Application::Get().GetGraphicsDevice()->CreateTexture(TextureDesc::Create2D(meta.width, meta.height, D3D::ConvertFormat(meta.format), meta.mipLevels, TextureFlag::ShaderResource), name.c_str(), scratchImage);
	}

	void AssetThumbnailPool::CreateThumbnailEntry(const AssetData& aAssetData, const Ref<Texture>& aThumbnailTexture, uint64 aResidentBytes) noexcept
	{
		ThumbnailEntry& entry = m_Thumbnails[aAssetData.Uuid];

		//If Already existing
		if (entry.ResidentBytes != 0)
			m_ResidentBytes -= entry.ResidentBytes;

		entry.Texture = aThumbnailTexture;
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
			const UUID toEvict = m_LRU.back();
			
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

	Ref<Texture> AssetThumbnailPool::TryFindThumbnail(const AssetData& aAssetData) noexcept
	{
		if (auto it = m_Thumbnails.find(aAssetData.Uuid); it != m_Thumbnails.end())
		{
			TouchLRU(aAssetData.Uuid, it->second);
			return it->second.Texture;
		}

		return nullptr;
	}

	Ref<Texture> AssetThumbnailPool::GenerateDefaultForAsset(const AssetData& aAssetData) noexcept
	{
		//Default resource shared, hence don't increase resident bytes (already done once)
		CreateThumbnailEntry(aAssetData, m_pDefaultThumbnailTexture, 0u);
		return m_pDefaultThumbnailTexture;
	}

	Path AssetThumbnailPool::GenerateThumbnailPath(const AssetData& aAssetData) noexcept
	{
		const String file = std::format("{:016X}.rthumb", static_cast<uint64_t>(std::hash<UUID>{}(aAssetData.Uuid)));
		const Path thumbnailPath = FilepathUtils::Combine(Project::GetThumbnailCacheDirectory(), file);

		return thumbnailPath;
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

	Ref<Texture> AssetThumbnailPool::TryLoadThumbnail(const AssetData& aAssetData) noexcept
	{
		const Path path = GenerateThumbnailPath(aAssetData);

		if (!File::Exists(path))
			return nullptr;

		LoadArchive archive(path, EArchiveFormat::Binary);
		if (!archive.IsValid())
			return nullptr;

		Ref<Texture> pThumbnailTexture = CreateTextureFromArchive(archive);
		if (!pThumbnailTexture)
			return nullptr;

		CreateThumbnailEntry(aAssetData, pThumbnailTexture, archive.ProcessedBytes());

		return pThumbnailTexture;
	}
}
