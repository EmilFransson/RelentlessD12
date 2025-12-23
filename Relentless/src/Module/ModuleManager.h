#pragma once
#include "IModule.h"

namespace Relentless
{
	class ModuleManager
	{
	public:
		enum class EModuleStatus : uint8 { Loading = 0, Loaded };
		
		struct ModuleEntry : RefCounted<ModuleEntry>
		{
			UniquePtr<IModule> Module;
			EModuleStatus Status = EModuleStatus::Loading;
			std::condition_variable Condition;
			std::mutex Mutex;
		};

		template<typename ModuleType>
		static NO_DISCARD ModuleType& LoadModuleChecked() noexcept
		{
			const size_t moduleHash = typeid(ModuleType).hash_code();

			Ref<ModuleEntry> pEntry = nullptr;

			// Fast path: find entry under shared lock
			{
				std::shared_lock<std::shared_mutex> readlock(s_ModuleMutex);
				if (s_Modules.contains(moduleHash))
					pEntry = s_Modules.at(moduleHash);
			}

			// Create placeholder if missing
			bool thisThreadCreatedEntry = false;
			if (!pEntry)
			{
				std::unique_lock<std::shared_mutex> writeLock(s_ModuleMutex);
				auto& entry = s_Modules[moduleHash];
				if (!entry)
					entry = new ModuleEntry();  // state=Loading by default
				pEntry = entry;
				thisThreadCreatedEntry = true;
			}

			// If it's already loaded, return it
			if (pEntry->Module && pEntry->Status == EModuleStatus::Loaded)
				return static_cast<ModuleType&>(*pEntry->Module);

			// Now synchronize on this module's entry
			std::unique_lock<std::mutex> writeLock(pEntry->Mutex);

			// If another thread is loading it, wait
			while (!thisThreadCreatedEntry && pEntry->Status == EModuleStatus::Loading && !pEntry->Module)
				pEntry->Condition.wait(writeLock);

			// If it's already loaded, return it
			if (pEntry->Status == EModuleStatus::Loaded)
				return static_cast<ModuleType&>(*pEntry->Module);

			if (!pEntry->Module)
			{
				// Mark ourselves as the loader by creating the module object now
				pEntry->Module = MakeUnique<ModuleType>();

				// IMPORTANT: run OnLoad() without holding entry lock
				writeLock.unlock();
				
				pEntry->Module->OnLoad();
				writeLock.lock();
				pEntry->Status = EModuleStatus::Loaded;

				pEntry->Condition.notify_all();

				return static_cast<ModuleType&>(*pEntry->Module);
			}

			return static_cast<ModuleType&>(*pEntry->Module);
		}

		void ShutDown() noexcept
		{
			ShutdownAllModules();
		}

	private:
		static void ShutdownAllModules() noexcept
		{
			std::unique_lock<std::shared_mutex> writeLock(s_ModuleMutex);

			for (auto& [id, moduleEntry] : s_Modules)
			{
				const UniquePtr<IModule>& pModule = moduleEntry->Module;

				if (!pModule->SupportsAutomaticShutdown())
					pModule->OnUnload();
			}

			s_Modules.clear();
		}

	private:
		inline static std::unordered_map<size_t, Ref<ModuleEntry>> s_Modules;
		inline static std::shared_mutex s_ModuleMutex;
	};
}