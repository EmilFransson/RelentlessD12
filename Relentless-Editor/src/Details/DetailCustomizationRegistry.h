#pragma once
#include <Relentless.h>

#include "UI/Views/Details/Customizations/IDetailCustomization.h"

namespace Relentless
{
	class DetailCustomizationRegistry
	{
	public:
		template<typename T>
		std::vector<UniquePtr<IDetailCustomization>> GetCustomizations() const noexcept;

		template<typename T, typename U>
		void Register() noexcept;
	private:
		struct Entry
		{
			std::unordered_set<TypeIndex> OwningTypes;
			std::vector<Callback<UniquePtr<IDetailCustomization>()>> Factories;
		};

		std::vector<UniquePtr<IDetailCustomization>> Create(const Entry& aEntry) const noexcept;
	private:
		std::unordered_map<TypeIndex, Entry> m_Customizations;
	};

	template<typename T>
	std::vector<UniquePtr<IDetailCustomization>> DetailCustomizationRegistry::GetCustomizations() const noexcept
	{
		const TypeIndex owningTypeIndex = getTypeIndex<T>();

		if (auto it = m_Customizations.find(owningTypeIndex); it != m_Customizations.end())
			return Create(it->second);

		return {};
	}

	template<typename T, typename U>
	void DetailCustomizationRegistry::Register() noexcept
	{
		const TypeIndex owningTypeIndex = getTypeIndex<T>();
		const TypeIndex customTypeIndex = getTypeIndex<U>();

		Entry& entry = m_Customizations[owningTypeIndex];
		if (entry.OwningTypes.contains(customTypeIndex))
			return;

		entry.OwningTypes.insert(customTypeIndex);
		entry.Factories.push_back([]() { return MakeUnique<U>(); });
	}
}