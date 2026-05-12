#include "IAsset.h"

#include "Core/Time.h"

#include "Module/AssetRegistryModule.h"
#include "Module/ModuleManager.h"

#include "Utility/FilepathUtils.h"

namespace Relentless
{
	IAsset::IAsset(const UUID& aUUID) noexcept
		: m_UUID(aUUID)
	{
	}

	const String& IAsset::GetName() const noexcept
	{
		return m_Name;
	}

	const UUID& IAsset::GetUUID() const noexcept
	{
		return m_UUID;
	}

	bool IAsset::IsDirty() const noexcept
	{
		return m_IsDirty;
	}

	bool IAsset::Load()
	{
		PreLoad();

		AssetRegistryModule& assetRegistry = ModuleManager::LoadModuleChecked<AssetRegistryModule>();
		const AssetData* pAssetdata = assetRegistry.FindAsset(GetUUID());
		if (!pAssetdata)
			return false;

		Path targetLoadPath = FilepathUtils::Combine(pAssetdata->PackagePath, pAssetdata->Name);
		FilepathUtils::SetExtension(targetLoadPath, ".rasset");

		LoadArchive coreArchive(targetLoadPath, EArchiveFormat::Binary);
		if (!coreArchive.IsValid())
			return false;

		AssetFileContent content{};
		if (!coreArchive.Process(content))
			return false;

		if (!SerializeCore(coreArchive))
			return false;

		if (content.BulkDataSize > 0)
		{
			FilepathUtils::SetExtension(targetLoadPath, ".rbulk");
			LoadArchive bulkArchive(targetLoadPath, EArchiveFormat::Binary);
			if (!bulkArchive.IsValid())
				return false;

			if (!SerializeBulk(bulkArchive))
				return false;
		}

		PostLoad();

		return true;
	}

	void IAsset::MarkDirty() noexcept
	{
		m_IsDirty = true;
	}

	bool IAsset::Save()
	{
		if (!m_IsDirty)
			return true;

		OnSave(this);

		PreSave();

		AssetRegistryModule& assetRegistry = ModuleManager::LoadModuleChecked<AssetRegistryModule>();
		const AssetData* pAssetdata = assetRegistry.FindAsset(GetUUID());
		if (!pAssetdata)
			return false;

		Path bulkDataFilePath = pAssetdata->PackagePath;
		bulkDataFilePath = FilepathUtils::Combine(bulkDataFilePath, pAssetdata->Name);

		FilepathUtils::SetExtension(bulkDataFilePath, ".rbulk");

		SaveArchive bulkArchive(bulkDataFilePath, EArchiveFormat::Binary);
		if (!bulkArchive.IsValid())
			return false;

		bulkArchive.SetFileHiddenOnDone(true);

		if (!SerializeBulk(bulkArchive))
			return false;

		if (!bulkArchive.Flush())
			return false;

		Path coreDataFilepath = bulkDataFilePath;
		FilepathUtils::SetExtension(coreDataFilepath, ".rasset");

		SaveArchive archive(coreDataFilepath, EArchiveFormat::Binary);
		if (!archive.IsValid())
			return false;

		archive.SetFileHiddenOnDone(true);

		AssetFileContent content{};
		content.AssetUUID = GetUUID();
		content.PersistentID = GetPersistentType();
		content.BulkDataSize = bulkArchive.ProcessedBytes();
		content.ModificationDateAndTime = Time::GetCurrentTimeStamp();

		if (!archive.Process(content))
			return false;

		if (!SerializeCore(archive))
			return false;

		if (!archive.Flush())
			return false;

		if (!bulkArchive.IsValid() || !archive.IsValid())
			return false;

		MarkClean();
		PostSave();

		OnSaved(this);

		return true;
	}

	void IAsset::SetName(const String& aName) noexcept
	{
		m_Name = aName;
	}

	void IAsset::MarkClean() noexcept
	{
		m_IsDirty = false;
	}

}