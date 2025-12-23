#include "MaterialFactory.h"

#include "Assets/AssetManager.h"
#include "Graphics/Resources/Material.h"

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

	AssetHandle MaterialFactory::CreateNew(const String& aName, const Path& aPackagePath) noexcept
	{
		Ref<Material> pMaterial = new Material();
		pMaterial->SetName(aName);

		return {};
	}

	String MaterialFactory::GetDefaultNewAssetName() const noexcept
	{
		return "NewMaterial";
	}

	bool MaterialFactory::DoesSupportAsset(IAsset* aAsset) const noexcept
	{
		return dynamic_cast<Material*>(aAsset) != nullptr;
	}

}