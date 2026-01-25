#include "AssetMeta.h"
#include "AssetManager.h"
#include "Serialization/Archive.h"

namespace Relentless
{
	const AssetHandle AssetHandle::INVALID{ TypeIndex{}, NULL_UUID };

	const AssetHandle NULL_HANDLE{ TypeIndex{}, NULL_UUID };

	AssetHandle::AssetHandle(const TypeIndex& aTypeIndex, const UUID& aUUID) noexcept
		: Type{ aTypeIndex },
		  Uuid{ aUUID }
	{
	}

	bool AssetHandle::Serialize(IArchive& archive)
	{
		if (archive.IsSaving())
			return OnSave(archive);
		else if (archive.IsLoading())
			return OnLoad(archive);

		return false;
	}

	bool AssetHandle::OnLoad(IArchive& archive) noexcept
	{
		UUID persistentType = NULL_UUID;

		if (!archive.Process(persistentType))
			return false;

		Type = persistentType != NULL_UUID ? AssetManager::PersistentTypeToRuntimeType(persistentType) : INVALID_TYPE::StaticType();

		return archive.Process(Uuid);
	}

	bool AssetHandle::OnSave(IArchive& archive) noexcept
	{
		UUID persistentType = AssetManager::RuntimeTypeToPersistentType(Type);
		return archive.Process(persistentType) && archive.Process(Uuid);
	}

	AssetHandle::~AssetHandle() noexcept
	{
	}

	AssetHandle::AssetHandle(const AssetHandle& otherHandle) noexcept
		: Type{otherHandle.Type},
		  Uuid{otherHandle.Uuid}
	{
	}

	AssetHandle& AssetHandle::operator=(const AssetHandle& otherHandle) noexcept
	{
		if (this == &otherHandle)
			return *this;
			
		Type = otherHandle.Type;
		Uuid = otherHandle.Uuid;

		return *this;
	}
}