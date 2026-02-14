#include "DetailCustomizationRegistry.h"

namespace Relentless
{
	std::vector<UniquePtr<IDetailCustomization>> DetailCustomizationRegistry::Create(const Entry& aEntry) const noexcept
	{
		std::vector<UniquePtr<IDetailCustomization>> customizations;
		customizations.reserve(aEntry.Factories.size());

		for (const auto& factory : aEntry.Factories)
			customizations.push_back(factory());

		return customizations;
	}

}