#include "MaterialFactory.h"

namespace Relentless
{
	bool MaterialFactory::CanCreateNew() const noexcept
	{
		return true;
	}

	bool MaterialFactory::CanImport(MAYBE_UNUSED const Path& aPath) const noexcept
	{
		return false;
	}

	Ref<IFactory> MaterialFactory::Clone() noexcept
	{
		return new MaterialFactory();
	}

	FactoryCreateResult MaterialFactory::CreateNew(MAYBE_UNUSED const TypeIndex& aType, const String& aName, const UUID& aUUID) noexcept
	{
		Ref<Material> pMaterial = new Material(aUUID);
		pMaterial->SetName(aName);

		return pMaterial;
	}

	String MaterialFactory::GetDefaultNewAssetName() const noexcept
	{
		return "NewMaterial";
	}

	bool MaterialFactory::DoesSupportAsset(IAsset* aAsset) const noexcept
	{
		return aAsset->GetStaticType() == Material::StaticType();
	}

	String MaterialFactory::GetDisplayName() const noexcept
	{
		return "MaterialFactory";
	}
}