#include "EnvironmentFactory.h"

namespace Relentless
{
	bool EnvironmentFactory::CanCreateNew() const noexcept
	{
		return true;
	}

	bool EnvironmentFactory::CanImport(MAYBE_UNUSED const Path& aPath) const noexcept
	{
		return false;
	}

	Ref<IFactory> EnvironmentFactory::Clone() noexcept
	{
		return RLS_NEW EnvironmentFactory();
	}

	FactoryCreateResult EnvironmentFactory::CreateNew(MAYBE_UNUSED const TypeIndex& aType, const String& aName, const UUID& aUUID) noexcept
	{
		Ref<Environment> pEnvironment = RLS_NEW Environment(aUUID);
		pEnvironment->SetName(aName);

		return pEnvironment;
	}

	String EnvironmentFactory::GetDefaultNewAssetName() const noexcept
	{
		return "NewEnvironment";
	}

	bool EnvironmentFactory::DoesSupportAsset(IAsset* aAsset) const noexcept
	{
		return aAsset->GetStaticType() == Environment::StaticType();
	}

	String EnvironmentFactory::GetDisplayName() const noexcept
	{
		return "EnvironmentFactory";
	}
}