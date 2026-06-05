#pragma once
#include <Relentless.h>

#include "IAssetFilter.h"

namespace Relentless
{
	class AssetFilterCollection
	{
	public:
		template<typename T, typename... Args> 
		T* Add(Args&&...) noexcept;
		
		template<typename T> 
		T* Get() noexcept;

		NO_DISCARD bool PassesAll(const AssetData& aAssetData) const;

		Broadcaster<void()> OnFilterChanged;
	private:
		std::unordered_map<TypeIndex, UniquePtr<IAssetFilter>> m_Filters;
	};

	template<typename T, typename... Args>
	T* AssetFilterCollection::Add(Args&&... args) noexcept
	{
		static_assert(std::is_base_of_v<IAssetFilter, T>, "[AssetFilterCollection::Add]: Filter Type must inherit from IAssetFilter");

		auto pFilter = MakeUnique<T>(std::forward<Args>(args)...);
		pFilter->OnChanged.Connect([this]() { OnFilterChanged(); });

		T* pRaw = pFilter.get();
		m_Filters[getTypeIndex<T>()] = std::move(pFilter);
		return pRaw;
	}

	template<typename T>
	T* AssetFilterCollection::Get() noexcept
	{
		auto it = m_Filters.find(getTypeIndex<T>());
		return it != m_Filters.end() ? static_cast<T*>(it->second.get()) : nullptr;
	}
}