#include "MaterialFactory.h"

#include "Assets/AssetManager.h"
#include "File/File.h"
#include "Graphics/Resources/Material.h"
#include "Project/Project.h"
#include "Utility/FilepathUtils.h"

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

	FactoryResult MaterialFactory::CreateNew(const String& aName, const Path& aPackagePath) noexcept
	{
		const Path fullPackagePath = FilepathUtils::Combine(Project::GetAssetDirectory(), aPackagePath);
		const Path assetFilePath = FilepathUtils::Combine(fullPackagePath, aName + ".rasset");
		if (File::Exists(assetFilePath))
			return std::unexpected{std::format("File with name '{}' already exists at path '{}'", aName, aPackagePath.string())};

		Ref<Material> pMaterial = new Material();
		pMaterial->SetName(aName);

		return AssetManager::RegisterAsset<Material>(pMaterial);
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