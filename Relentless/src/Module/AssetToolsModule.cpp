#include "AssetToolsModule.h"
#include "Core/Application.h"
#include "Threading/ThreadPool.h"

namespace Relentless
{
	void AsyncImportTasks::Add(std::future<void> aFuture) noexcept
	{
		m_Futures.emplace_back(std::move(aFuture));
	}

	bool AsyncImportTasks::IsComplete() const noexcept
	{
		for (const auto& future : m_Futures)
		{
			if (future.wait_for(std::chrono::seconds(0)) != std::future_status::ready)
				return false;
		}

		return true;
	}

	void AsyncImportTasks::Wait() noexcept
	{
		for (auto& future : m_Futures)
			future.wait();
	}

	AssetHandle AssetToolsModule::CreateAndRegisterAssetData(const Ref<IAsset>& aAsset, const Path& aPackagePath, TimeStamp aTimeStamp) noexcept
	{
		AssetData data{};
		data.Name = aAsset->GetName();
		data.Type = aAsset->GetStaticType();
		data.Uuid = aAsset->GetUUID();
		data.PackagePath = aPackagePath;
		data.ModificationDateAndTime = aTimeStamp;

		AssetRegistryModule& assetRegistry = ModuleManager::LoadModuleChecked<AssetRegistryModule>();
		assetRegistry.AssetCreated(std::move(data));

		return AssetManager::RegisterAsset(aAsset);
	}

	AssetHandle AssetToolsModule::CreateAsset(TypeIndex aType, const String& aName, const Path& aPackagePath, const Ref<IFactory>& aFactory /*= nullptr*/, bool aShouldSave /*= true*/) noexcept
	{
		const Ref<IFactory>& pFactory = aFactory ? aFactory : GetSupportingFactory(aType);
		if (!pFactory)
		{
			RLS_CORE_WARN("Failed to find factory for asset with name: {0}", aName);
			return AssetHandle::INVALID;
		}

		if (!pFactory->CanCreateNew())
		{
			RLS_CORE_WARN("Factory '{0}' for type '{1}' cannot create new assets.", pFactory->GetDisplayName(), pFactory->GetDefaultNewAssetName());
			return AssetHandle::INVALID;
		}

		const Path fullPackagePath = FilepathUtils::Combine(Project::GetAssetDirectory(), aPackagePath);
		const Path assetFilePath = FilepathUtils::Combine(fullPackagePath, aName + ".rasset");
		if (aShouldSave && File::Exists(assetFilePath))
		{
			RLS_CORE_WARN("File with name '{0}' already exists at path '{1}'", aName, fullPackagePath.string());
			return AssetHandle::INVALID;
		}

		if (!FilepathUtils::CreateDirectoryTree(fullPackagePath))
		{
			RLS_CORE_WARN("Failed to create directories for path '{0}'", fullPackagePath.string());
			return AssetHandle::INVALID;
		}

		FactoryCreateResult result = pFactory->CreateNew(aType, aName);
		if (!result)
		{
			RLS_CORE_INFO("Failed to create asset '{0}' of type '{1}' with error:\n'{2}'", aName, pFactory->GetDefaultNewAssetName(), result.error());
			return AssetHandle::INVALID;
		}

		Ref<IAsset> pCreatedAsset = result.value();
		const TimeStamp timeStamp = Time::GetCurrentTimeStamp();

		if (aShouldSave && !SerializeAsset(pCreatedAsset, assetFilePath, timeStamp))
		{
			RLS_CORE_INFO("Failed to create asset file '{}'", assetFilePath.string());
			return AssetHandle::INVALID;
		}

		if (aShouldSave)
			return CreateAndRegisterAssetData(pCreatedAsset, fullPackagePath, timeStamp);
		else
			return AssetManager::RegisterAsset(pCreatedAsset);
	}

	std::vector<AssetImportResult> AssetToolsModule::Import(Span<AssetImportTask> someImportTasks) noexcept
	{
		ThreadPool& threadPool = Application::Get().GetThreadPool();

		std::vector<AssetImportResult> importedAssets;
		importedAssets.reserve(someImportTasks.GetSize());

		std::vector<std::future<void>> futures;
		futures.reserve(someImportTasks.GetSize());

		std::mutex importedAssetsMutex;

		for (const AssetImportTask& task : someImportTasks)
		{
			Ref<IFactory> pFactory = task.pFactory;
			if (!pFactory)
				pFactory = GetSupportingFactory(task.FilePath);

			if (!pFactory)
			{
				RLS_CORE_WARN("Failed to find factory for file with path: {0}", task.FilePath.string());
				continue;
			}

			futures.push_back(threadPool.Submit([this, pFactory, task, &importedAssetsMutex, &importedAssets]()
				{
					std::vector<FactoryResult> importResults;
					importResults.push_back(pFactory->ImportFromFile(task.FilePath, task.DestinationPath, task.FilePath.stem().string()));

					const std::vector<FactoryResult>& additionalImportedAssets = pFactory->GetAdditionalImportedAssets();
					importResults.insert(importResults.end(), additionalImportedAssets.begin(), additionalImportedAssets.end());

					std::lock_guard<std::mutex> lock(importedAssetsMutex);
					for (const FactoryResult& importResult : importResults)
					{
						if (importResult)
						{
							if (task.ShouldSave)
							{
								Ref<IAsset> pAsset = AssetManager::Get(importResult.value());
								Path destinationPath = task.DestinationPath;
								if (destinationPath.empty())
									destinationPath = task.FilePath;
								else
								{
									Path fullDestination = FilepathUtils::Combine(Project::GetProjectDirectory(), destinationPath);
									destinationPath = FilepathUtils::Combine(fullDestination, pAsset->GetName());
								}

								if (!SerializeAsset(pAsset, destinationPath, Time::GetCurrentTimePoint()))
									continue;
							}
							
							importedAssets.push_back({ importResult.value(), task.FilePath });
						}
						else
						{
							RLS_CORE_WARN("Failed to import asset from file '{0}' with error: '{1}'", task.FilePath.string(), importResult.error());
						}
					}
				}));
		}

		for (const auto& future : futures)
			future.wait();

		return importedAssets;
	}

	AsyncImportTasks AssetToolsModule::ImportAsync(Span<AssetImportTask> someImportTasks, AssetImportTasksCompletedCallback aImportCompletedCallback) noexcept
	{
		if (someImportTasks.GetSize() == 0)
			return {};

		const uint32 importBatchID = m_NextImportBatchID++;

		Ref<AsyncAssetImportResult> pAsyncImportResult = new AsyncAssetImportResult;
		pAsyncImportResult->CompletionCallback = std::move(aImportCompletedCallback);
		pAsyncImportResult->Remaining.store((int32)someImportTasks.GetSize(), std::memory_order_relaxed);

		{
			std::lock_guard<std::mutex> lock(m_PendingImportBatchesMutex);
			m_PendingImportBatches.emplace(importBatchID, pAsyncImportResult);
		}

		ThreadPool& threadPool = Application::Get().GetThreadPool();
		AsyncImportTasks importTasks;

		auto TryFinish = [this, importBatchID, pAsyncImportResult]()
			{
				if (pAsyncImportResult->Remaining.fetch_sub(1, std::memory_order_acq_rel) != 1)
					return;

				std::vector<AssetImportResult> results;
				{
					std::lock_guard<std::mutex> lock(pAsyncImportResult->Mutex);
					results.swap(pAsyncImportResult->ImportResults);
				}

				auto cb = std::make_shared<AssetImportTasksCompletedCallback>(std::move(pAsyncImportResult->CompletionCallback));

				Application::Get().SubmitToMainThread([cb = std::move(cb), results = std::move(results)]() mutable
					{
						(*cb)(results);
					});

				std::lock_guard<std::mutex> lock(m_PendingImportBatchesMutex);
				m_PendingImportBatches.erase(importBatchID);
			};

		for (const AssetImportTask& task : someImportTasks)
		{
			Ref<IFactory> pFactory = task.pFactory ? task.pFactory : GetSupportingFactory(task.FilePath);

			if (!pFactory)
			{
				RLS_CORE_WARN("Failed to find factory for file with path: {0}", task.FilePath.string());
				TryFinish();
				continue;
			}

			importTasks.Add(threadPool.Submit([this, pFactory, task, pAsyncImportResult, TryFinish]()
				{
					std::vector<FactoryResult> importResults;
					importResults.push_back(pFactory->ImportFromFile(task.FilePath, task.DestinationPath, task.FilePath.stem().string()));

					const std::vector<FactoryResult>& additionalImportedAssets = pFactory->GetAdditionalImportedAssets();
					importResults.insert(importResults.end(), additionalImportedAssets.begin(), additionalImportedAssets.end());

					{
						std::lock_guard<std::mutex> lock(pAsyncImportResult->Mutex);
						for (const FactoryResult& importResult : importResults)
						{
							if (importResult)
							{
								Ref<IAsset> pAsset = AssetManager::Get(importResult.value());
								Path destinationPath = task.DestinationPath;
								if (destinationPath.empty())
									destinationPath = task.FilePath;
								else
								{
									Path fullDestination = FilepathUtils::Combine(Project::GetProjectDirectory(), destinationPath);
									destinationPath = FilepathUtils::Combine(fullDestination, pAsset->GetName());
								}

								if (!SerializeAsset(pAsset, destinationPath, Time::GetCurrentTimePoint()))
									continue;

								pAsyncImportResult->ImportResults.push_back({ importResult.value(), task.FilePath });
							}
							else
							{
								RLS_CORE_WARN("Failed to import asset from file '{0}' with error: '{1}'", task.FilePath.string(), importResult.error());
							}
						}
					}

					TryFinish();
				}));
		}

		return importTasks;
	}

	Ref<IFactory> AssetToolsModule::GetSupportingFactory(const Path& aPath) const noexcept
	{
		std::lock_guard<std::mutex> guard(m_FactoryMutex);

		for (const auto& [id, pFactory] : m_RegisteredFactories)
		{
			if (pFactory->CanImport(aPath))
				return pFactory->Clone();
		}

		return nullptr;
	}

	Ref<IFactory> AssetToolsModule::GetSupportingFactory(const TypeIndex& aType) const noexcept
	{
		std::lock_guard<std::mutex> guard(m_FactoryMutex);

		if (!m_RegisteredFactories.contains(aType))
			return nullptr;

		return m_RegisteredFactories.at(aType)->Clone();
	}

	bool AssetToolsModule::SerializeAsset(const Ref<IAsset>& aAsset, const Path& aAssetFilePath, TimeStamp aTimeStamp) noexcept
	{
		if (!aAsset)
			return false;

		Path bulkDataFilePath = aAssetFilePath;
		FilepathUtils::SetExtension(bulkDataFilePath, ".rbulk");

		if (!FilepathUtils::CreateDirectoryTree(aAssetFilePath))
			return false;

		SaveArchive bulkArchive(bulkDataFilePath, EArchiveFormat::Binary);
		if (!bulkArchive.IsValid())
			return false;

		bulkArchive.SetFileHiddenOnDone(true);

		if (!aAsset->SerializeBulk(bulkArchive))
			return false;

		if (!bulkArchive.Flush())
			return false;

		Path coreDataFilepath = bulkDataFilePath;
		FilepathUtils::SetExtension(coreDataFilepath, ".rasset");

		SaveArchive archive(coreDataFilepath, EArchiveFormat::Binary);
		if (!archive.IsValid())
			return false;

		archive.SetFileHiddenOnDone(true);

		AssetFileContent content{};
		content.AssetUUID = aAsset->GetUUID();
		content.PersistentID = aAsset->GetPersistentType();
		content.BulkDataSize = bulkArchive.ProcessedBytes();
		content.ModificationDateAndTime = aTimeStamp;

		if (!archive.Process(content))
			return false;

		if (!aAsset->SerializeCore(archive))
			return false;

		return bulkArchive.IsValid() && archive.IsValid();
	}
}
