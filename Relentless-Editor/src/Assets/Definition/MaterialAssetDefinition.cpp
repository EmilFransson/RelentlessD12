#include "MaterialAssetDefinition.h"

#include "Assets/Factory/TextureFactory.h"

#include "Module/UIModule.h"

#include "Panels/MaterialEditorPanel.h"

namespace Relentless
{
	bool MaterialAssetDefinition::RequestGenerateThumbnail(MAYBE_UNUSED const AssetData& aAssetData, const Callback<void(const Ref<Texture2D>&)>& aOnThumbnailGeneratedCallback) noexcept
	{
		if (!m_pThumbnail)
		{
			std::vector<AssetImportTask> importTasks;

			AssetImportTask& task = importTasks.emplace_back();
			task.FilePath = FilepathUtils::Combine(ENGINE_ASSET_DIRECTORY, "Textures/MaterialThumbnail.jpg");

			Ref<TextureFactory> pFactory = RLS_NEW TextureFactory();
			pFactory->SetImportAsSRGB(true);
			pFactory->SetGenerateMipmaps(true);
			task.pFactory = pFactory;

			AssetToolsModule& assetTools = ModuleManager::LoadModuleChecked<AssetToolsModule>();
			std::vector<AssetImportResult> result = assetTools.Import(importTasks);
			m_pThumbnail = AssetManager::Get<Texture2D>(result[0].Handle);
			m_pThumbnail->CreateResource();
		}

		aOnThumbnailGeneratedCallback(m_pThumbnail);
		return true;
	}

	std::vector<String> MaterialAssetDefinition::GetAssetCategories() const noexcept
	{
		return { "Material/Material" };
	}

	Color MaterialAssetDefinition::GetAssetColor() const noexcept
	{
		return Colors::Green;
	}

	String MaterialAssetDefinition::GetAssetDisplayName() const noexcept
	{
		return "Material";
	}

	TypeIndex MaterialAssetDefinition::GetSupportedAssetType() const noexcept
	{
		return Material::StaticType();
	}

	Ref<ThumbnailInfo> MaterialAssetDefinition::GetThumbnailInfo(const AssetData& aAssetData) const noexcept
	{
		Ref<ThumbnailInfo> pThumbnailInfo = RLS_NEW ThumbnailInfo();
		pThumbnailInfo->DisplayName = GetAssetDisplayName();
		pThumbnailInfo->Label = aAssetData.Name;
		pThumbnailInfo->TypeColor = GetAssetColor();
		pThumbnailInfo->Width = 256u;
		pThumbnailInfo->Height = 256u;
		
		return pThumbnailInfo;
	}

	bool MaterialAssetDefinition::OpenAssets(const std::vector<AssetHandle>& someAssets) noexcept
	{
		if (someAssets.size() != 1u)
			return false;

		for (const AssetHandle& assetHandle : someAssets)
		{
			if (!SupportsAsset(assetHandle))
				return false;
		}

		ModuleManager::LoadModuleChecked<UIModule>().OpenPanel<MaterialEditorPanel>(std::move(someAssets));

		return true;
	}

	bool MaterialAssetDefinition::SupportsAsset(IAsset* aAsset) const noexcept
	{
		return aAsset->GetStaticType() == Material::StaticType();
	}

	bool MaterialAssetDefinition::SupportsAsset(AssetData* aAssetData) const noexcept
	{
		return aAssetData->Type == Material::StaticType();
	}

	bool MaterialAssetDefinition::SupportsAsset(const AssetHandle& aAssetHandle) const noexcept
	{
		return aAssetHandle.Type == Material::StaticType();
	}

	bool MaterialAssetDefinition::SupportsCreateNew() const noexcept
	{
		return true;
	}
}
