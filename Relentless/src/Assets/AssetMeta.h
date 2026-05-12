#pragma once
#include "Core/DLLExport.h"
#include "Utility/Rules.h"

#include <Core/StaticTypeInfo.h>

namespace Relentless
{
	inline constexpr char ASSET_EXTENSION[] = ".rasset";
	inline constexpr uint32_t NULL_INDEX = std::numeric_limits<uint32_t>::max();
	inline constexpr UUID NULL_UUID = UUID{0, 0, 0, {0}};

	inline static constexpr uint32 ASSET_FILE_MAGIC_NUMBER = 0x524C5354;

	#pragma pack(push, 1)
	struct AssetFileContent
	{
		uint32 Magic					= ASSET_FILE_MAGIC_NUMBER;
		uint32 Version					= 1u;
		uint32 Flags					= 0u;
		uint64 BulkDataSize				= 0u;
		uint64 ModificationDateAndTime	= 0u;
		UUID PersistentID				= NULL_UUID;
		UUID AssetUUID					= NULL_UUID;
		char SourcePath[260]			= { '\0' };
	};
	#pragma pack(pop)

	class IArchive;

	struct RLS_API AssetHandle
	{
		AssetHandle(const TypeIndex& aTypeIndex = {}, const UUID& aUUID = NULL_UUID) noexcept;
		~AssetHandle() noexcept;

		AssetHandle(const AssetHandle& otherHandle) noexcept;
		AssetHandle& operator=(const AssetHandle& otherHandle) noexcept;

		bool operator==(const AssetHandle& other) const noexcept
		{
			return Uuid == other.Uuid;
		}

		bool operator!=(const AssetHandle& other) const noexcept
		{
			return !(*this == other);
		}

		NO_DISCARD inline bool IsValid() const noexcept
		{
			return Uuid != NULL_UUID;
		}

		void Invalidate() noexcept
		{
			Type = {};
			Uuid = NULL_UUID;
		}

		bool Serialize(IArchive& archive);

		NO_DISCARD String ToString() const noexcept;

		TypeIndex Type = INVALID_TYPE::StaticType();
		UUID Uuid = NULL_UUID;

		static const AssetHandle INVALID;

	private:
		NO_DISCARD bool OnLoad(IArchive& archive) noexcept;
		NO_DISCARD bool OnSave(IArchive& archive) noexcept;
	};

	RLS_API extern const AssetHandle NULL_HANDLE;
}