#pragma once

#include "Assets/AssetManager.h"
#include "Assets/AssetMeta.h"
#include "Assets/Factory/IFactory.h"

#include "Core/DLLExport.h"
#include "Core/Time.h"

#include "File/File.h"

namespace Relentless
{
	class AsyncImportTasks
	{
	public:
		void Add(std::future<void> aFuture) noexcept;
		NO_DISCARD bool IsComplete() const noexcept;
		void Wait() noexcept;
	private:
		std::vector<std::future<void>> m_Futures;
	};

	struct FeedbackContext;

	struct AssetImportResult
	{
		AssetHandle Handle = AssetHandle::INVALID;
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

	class RLS_API AssetToolsModule : public IModule
	{
	public:
		template<typename AssetType>
		NO_DISCARD AssetHandle CreateAsset(const String& aName, const Path& aPackagePath, const Ref<IFactory>& aFactory = nullptr, bool aShouldSave = true) noexcept;

		std::vector<AssetImportResult> Import(Span<AssetImportTask> someImportTasks) noexcept;
		AsyncImportTasks ImportAsync(Span<AssetImportTask> someImportTasks, AssetImportTasksCompletedCallback aImportCompletedCallback) noexcept;

		template<typename SupportedAssetType>
		void RegisterFactory(const Ref<IFactory>& aFactory) noexcept;

		template<typename FactoryType>
		void RegisterCompositeFactory(const Ref<IFactory>& aFactory) noexcept;

		Ref<IFactory> GetSupportingFactory(const Path& aPath) const noexcept;
		Ref<IFactory> GetSupportingFactory(const TypeIndex& aType) const noexcept;
	private:
		template<typename AssetType>
		NO_DISCARD AssetHandle CreateAndRegisterAssetData(const Ref<IAsset>& aAsset, const Path& aPackagePath, TimeStamp aTimeStamp) noexcept;

		template<typename SupportedAssetType>
		Ref<IFactory> GetSupportingFactory() const noexcept;

		NO_DISCARD bool SerializeAsset(const Ref<IAsset>& aAsset, const Path& aAssetFilePath, TimeStamp aTimeStamp) noexcept;
	private:
		std::unordered_map<TypeIndex, Ref<IFactory>> m_RegisteredFactories;
		std::unordered_map<uint32, Ref<AsyncAssetImportResult>> m_PendingImportBatches;
		uint32 m_NextImportBatchID = 0;
		std::mutex m_PendingImportBatchesMutex;
		mutable std::mutex m_FactoryMutex;
	};

	template<typename AssetType>
	AssetHandle AssetToolsModule::CreateAsset(const String& aName, const Path& aPackagePath, const Ref<IFactory>& aFactory, bool aShouldSave) noexcept
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

		const Path fullPackagePath = FilepathUtils::Combine(Project::GetAssetDirectory(), aPackagePath);
		const Path assetFilePath = FilepathUtils::Combine(fullPackagePath, aName + ".rasset");
		if (File::Exists(assetFilePath))
		{
			RLS_CORE_WARN("File with name '{0}' already exists at path '{1}'", aName, fullPackagePath.string());
			return AssetHandle::INVALID;
		}

		if (!FilepathUtils::CreateDirectoryTree(fullPackagePath))
		{
			RLS_CORE_WARN("Failed to create directories for path '{0}'", fullPackagePath.string());
			return AssetHandle::INVALID;
		}

		FactoryCreateResult result = pFactory->CreateNew(AssetType::StaticType(), aName);
		if (!result)
		{
			RLS_CORE_INFO("Failed to create asset '{0}' of type '{1}' with error:\n'{2}'", aName, TypeName::GetPrettyTypeName<AssetType>(), result.error());
			return AssetHandle::INVALID;
		}

		Ref<IAsset> pCreatedAsset = result.value();
		const TimeStamp timeStamp = Time::GetCurrentTimeStamp();

		if (aShouldSave && !SerializeAsset(pCreatedAsset, assetFilePath, timeStamp))
		{
			RLS_CORE_INFO("Failed to create asset file '{}'", assetFilePath.string());
			return AssetHandle::INVALID;
		}

		return CreateAndRegisterAssetData<AssetType>(pCreatedAsset, aPackagePath, timeStamp);
	}

	template<typename AssetType>
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

		return AssetManager::RegisterAsset<AssetType>(aAsset);
	}

	template<typename SupportedAssetType>
	Ref<IFactory> AssetToolsModule::GetSupportingFactory() const noexcept
	{
		std::lock_guard<std::mutex> guard(m_FactoryMutex);

		static constexpr TypeIndex ID = getTypeIndex<SupportedAssetType>();

		if (!m_RegisteredFactories.contains(ID))
			return nullptr;

		return m_RegisteredFactories.at(ID)->Clone();
	}

	template<typename SupportedAssetType>
	void AssetToolsModule::RegisterFactory(const Ref<IFactory>& aFactory) noexcept
	{
		std::lock_guard<std::mutex> guard(m_FactoryMutex);

		static constexpr TypeIndex ID = getTypeIndex<SupportedAssetType>();
		if (m_RegisteredFactories.contains(ID))
			return;

		m_RegisteredFactories.emplace(ID, aFactory);

		AssetManager::RegisterStorage<SupportedAssetType>();
	}

	template<typename FactoryType>
	void AssetToolsModule::RegisterCompositeFactory(const Ref<IFactory>& aFactory) noexcept
	{
		std::lock_guard<std::mutex> guard(m_FactoryMutex);

		static constexpr TypeIndex ID = getTypeIndex<FactoryType>();
		if (m_RegisteredFactories.contains(ID))
			return;

		m_RegisteredFactories.emplace(ID, aFactory);
	}

}