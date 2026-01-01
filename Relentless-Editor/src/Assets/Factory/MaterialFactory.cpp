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
			return std::unexpected{std::format("File with name '{}' already exists at path '{}'", aName, fullPackagePath.string())};

		if (!FilepathUtils::CreateDirectoryTree(fullPackagePath))
			return std::unexpected{ std::format("Failed to create directories for path '{}'", fullPackagePath.string()) };

		Ref<Material> pMaterial = new Material();
		pMaterial->SetName(aName);

		std::ofstream outFile(assetFilePath, std::ios::binary);
		if (!outFile)
			return std::unexpected{ std::format("Failed to create file '{}'", assetFilePath.string()) };

		const TimeStamp timeStamp = Time::GetCurrentTimeStamp();
		
		AssetFileContent content{};
		content.AssetUUID = pMaterial->GetUUID();
		content.PersistentID = pMaterial->GetPersistentType();
		content.ModificationDateAndTime = timeStamp;
		//rest is correct default values

		outFile.write(reinterpret_cast<const char*>(&content), sizeof(content));

		FilepathUtils::SetFileHidden(assetFilePath);

		const AssetHandle handle = AssetManager::RegisterAsset<Material>(pMaterial);

		AssetData data{};
		data.Name = aName;
		data.PackagePath = aPackagePath;
		data.Type = Material::StaticType();
		data.Uuid = handle.Uuid;
		data.ModificationDateAndTime = timeStamp;

		AssetRegistryModule& assetRegistry = ModuleManager::LoadModuleChecked<AssetRegistryModule>();
		assetRegistry.AssetCreated(data);

		

		return handle;
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