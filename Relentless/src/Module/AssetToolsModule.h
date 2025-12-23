#pragma once
#include "IModule.h"

#include "Assets/AssetMeta.h"
#include "Callback/Callback.h"

#include "Assets/ImportSettings.h"

namespace Relentless
{
	struct AssetImportBatchEx
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

	class FeedbackContext;
	class IFactory;

	struct AssetImportResult
	{
		AssetHandle Handle;
		Path FilePath;
	};
	
	using AssetImportTasksCompletedCallback = Callback<void(const std::vector<AssetImportResult>& someAssetImportResults)>;

	struct AsyncAssetImportResult : public RefCounted<AsyncAssetImportResult>
	{
		std::vector<AssetImportResult> ImportResults;
		AssetImportTasksCompletedCallback CompletionCallback;
		std::atomic_int32_t Remaining{ 0 };
		std::mutex Mutex;
	};

	class AssetToolsModule : public IModule
	{
	public:
		std::vector<AssetImportResult> Import(Span<AssetImportTask> someImportTasks) noexcept;
		AssetImportBatchEx ImportAsync(Span<AssetImportTask> someImportTasks, AssetImportTasksCompletedCallback aImportCompletedCallback) noexcept;
	protected:
		virtual void OnLoad() override;
	private:
		Ref<IFactory> GetSupportingFactory(const Path& aPath) const noexcept;
	private:
		std::vector<Ref<IFactory>> m_RegisteredFactories;
		std::unordered_map<uint32, Ref<AsyncAssetImportResult>> m_PendingImportBatches;
		uint32 m_NextImportBatchID = 0;
		std::mutex m_PendingImportBatchesMutex;
	};
}