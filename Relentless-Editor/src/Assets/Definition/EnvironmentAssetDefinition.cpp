#include "EnvironmentAssetDefinition.h"

#include "Assets/Factory/TextureFactory.h"

#include "Module/UIModule.h"

#include "Panels/EnvironmentViewportPanel.h"

namespace Relentless
{
	bool EnvironmentAssetDefinition::RequestGenerateThumbnail(MAYBE_UNUSED const AssetData& aAssetData, const Callback<void(const Ref<Texture2D>&)>& aOnThumbnailGeneratedCallback) noexcept
	{
		if (!m_pThumbnail)
		{
			std::vector<AssetImportTask> importTasks;

			AssetImportTask& task = importTasks.emplace_back();
			task.FilePath = FilepathUtils::Combine(ENGINE_ASSET_DIRECTORY, "Textures/EnvironmentThumbnail.png");

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

	std::vector<String> EnvironmentAssetDefinition::GetAssetCategories() const noexcept
	{
		return { "Environment/Environment" };
	}

	Color EnvironmentAssetDefinition::GetAssetColor() const noexcept
	{
		return Colors::Yellow;
	}

	String EnvironmentAssetDefinition::GetAssetDisplayName() const noexcept
	{
		return "Environment";
	}

	TypeIndex EnvironmentAssetDefinition::GetSupportedAssetType() const noexcept
	{
		return Environment::StaticType();
	}

	Ref<ThumbnailInfo> EnvironmentAssetDefinition::GetThumbnailInfo(const AssetData& aAssetData) const noexcept
	{
		Ref<ThumbnailInfo> pThumbnailInfo = RLS_NEW ThumbnailInfo();
		pThumbnailInfo->DisplayName = GetAssetDisplayName();
		pThumbnailInfo->Label = aAssetData.Name;
		pThumbnailInfo->TypeColor = GetAssetColor();
		pThumbnailInfo->Width = 256u;
		pThumbnailInfo->Height = 256u;

		return pThumbnailInfo;
	}

	bool EnvironmentAssetDefinition::OpenAssets(const std::vector<Ref<IAsset>>& someAssets) noexcept
	{
		//TODO: Think on this rule...
		if (someAssets.size() != 1u)
			return false;

		std::vector<Ref<Environment>> environments;
		environments.reserve(someAssets.size());
		for (const Ref<IAsset>& asset : someAssets)
		{
			if (!SupportsAsset(asset))
				return false;

			environments.push_back(Ref<Environment>(static_cast<Environment*>(asset.Get())));
		}

		ModuleManager::LoadModuleChecked<UIModule>().OpenPanel<EnvironmentViewportPanel>(std::move(environments));

		return true;
	}

	bool EnvironmentAssetDefinition::SupportsAsset(IAsset* aAsset) const noexcept
	{
		return aAsset->GetStaticType() == Environment::StaticType();
	}

	bool EnvironmentAssetDefinition::SupportsAsset(AssetData* aAssetData) const noexcept
	{
		return aAssetData->Type == Environment::StaticType();
	}

	bool EnvironmentAssetDefinition::SupportsAsset(const AssetHandle& aAssetHandle) const noexcept
	{
		return aAssetHandle.Type == Environment::StaticType();
	}

	bool EnvironmentAssetDefinition::SupportsCreateNew() const noexcept
	{
		return true;
	}

}