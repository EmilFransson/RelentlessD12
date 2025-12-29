#include "AssetToolsModule.h"
#include "../Assets/Factory/MaterialFactory.h"
#include "../Assets/Factory/ModelFactory.h"
#include "../Assets/Factory/TextureFactory.h"

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

			futures.push_back(threadPool.Submit([pFactory, task, &importedAssetsMutex, &importedAssets]() 
				{
					std::vector<FactoryResult> importResults;
					importResults.push_back(pFactory->ImportFromFile(task.FilePath, "TODO", task.FilePath.stem().string()));

					const std::vector<FactoryResult>& additionalImportedAssets = pFactory->GetAdditionalImportedAssets();
					importResults.insert(importResults.end(), additionalImportedAssets.begin(), additionalImportedAssets.end());

					std::lock_guard<std::mutex> lock(importedAssetsMutex);
					for (const FactoryResult& importResult : importResults)
					{
						if (importResult)
							importedAssets.push_back({ importResult.value(), task.FilePath });
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
					importResults.push_back(pFactory->ImportFromFile(task.FilePath, "TODO", task.FilePath.stem().string()));

					const std::vector<FactoryResult>& additionalImportedAssets = pFactory->GetAdditionalImportedAssets();
					importResults.insert(importResults.end(), additionalImportedAssets.begin(), additionalImportedAssets.end());

					{
						std::lock_guard<std::mutex> lock(pAsyncImportResult->Mutex);
						for (const FactoryResult& importResult : importResults)
						{
							if (importResult)
								pAsyncImportResult->ImportResults.push_back({ importResult.value(), task.FilePath });
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

	void AssetToolsModule::OnLoad()
	{
		RegisterFactory<Material>(new MaterialFactory());
		RegisterFactory<Mesh>(new ModelFactory());
		RegisterFactory<Texture2D>(new TextureFactory());
	}

	Ref<IFactory> AssetToolsModule::GetSupportingFactory(const Path& aPath) const noexcept
	{
		for (const auto&[id, pFactory] : m_RegisteredFactories)
		{
			if (pFactory->CanImport(aPath))
				return pFactory->Clone();
		}

		return nullptr;
	}
}
