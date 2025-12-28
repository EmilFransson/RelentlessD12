#pragma once
#include "IModule.h"

#include "Assets/AssetMeta.h"
#include "Callback/Callback.h"

#include "Assets/AssetManager.h"
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
		template<typename AssetType>
		NO_DISCARD AssetHandle CreateAsset(const String& aName, const Path& aPackagePath, const Ref<IFactory>& aFactory = nullptr) noexcept;

		std::vector<AssetImportResult> Import(Span<AssetImportTask> someImportTasks) noexcept;
		AssetImportBatchEx ImportAsync(Span<AssetImportTask> someImportTasks, AssetImportTasksCompletedCallback aImportCompletedCallback) noexcept;

		template<typename SupportedAssetType>
		void RegisterFactory(const Ref<IFactory>& aFactory) noexcept;
	protected:
		virtual void OnLoad() override;
	private:
		Ref<IFactory> GetSupportingFactory(const Path& aPath) const noexcept;

		template<typename SupportedAssetType>
		Ref<IFactory> GetSupportingFactory() const noexcept;
	private:
		std::unordered_map<TypeIndex, Ref<IFactory>> m_RegisteredFactories;
		std::unordered_map<uint32, Ref<AsyncAssetImportResult>> m_PendingImportBatches;
		uint32 m_NextImportBatchID = 0;
		std::mutex m_PendingImportBatchesMutex;
	};

	template<typename AssetType>
	AssetHandle AssetToolsModule::CreateAsset(const String& aName, const Path& aPackagePath, const Ref<IFactory>& aFactory) noexcept
	{
		const Ref<IFactory>& pFactory = aFactory ? aFactory : GetSupportingFactory<AssetType>();
		if (!pFactory)
		{
			RLS_CORE_WARN("Failed to find factory for asset type: {0}", TypeName::GetPrettyTypeName<AssetType>());
			return AssetHandle::INVALID;
		}

		if (!pFactory->CanCreateNew())
		{
			RLS_CORE_WARN("Factory '{0}' for type '{1}' cannot create new assets.", pFactory->GetDisplayName(), TypeName::GetPrettyTypeName<AssetType>());
			return AssetHandle::INVALID;
		}

		FactoryResult result = pFactory->CreateNew(aName, aPackagePath);
		if (result)
			return result.value();
		else
		{
			RLS_CORE_INFO("Failed to create asset '{0}' of type '{1}' with error:\n'{2}'", aName, TypeName::GetPrettyTypeName<AssetType>(), result.error());
			return AssetHandle::INVALID;
		}
	}

	template<typename SupportedAssetType>
	Ref<IFactory> AssetToolsModule::GetSupportingFactory() const noexcept
	{
		static constexpr TypeIndex ID = getTypeIndex<SupportedAssetType>();

		if (!m_RegisteredFactories.contains(ID))
			return nullptr;

		return m_RegisteredFactories.at(ID)->Clone();
	}

	template<typename SupportedAssetType>
	void AssetToolsModule::RegisterFactory(const Ref<IFactory>& aFactory) noexcept
	{
		static constexpr TypeIndex ID = getTypeIndex<SupportedAssetType>();
		if (m_RegisteredFactories.contains(ID))
			return;

		m_RegisteredFactories.emplace(ID, aFactory);

		AssetManager::RegisterStorage<SupportedAssetType>();
	}
}