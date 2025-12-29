#pragma once
#include "Utility/Rules.h"

#include <StaticTypeInfo/type_index.h>

namespace Relentless
{
	inline constexpr char ASSET_EXTENSION[] = ".rasset";
	inline constexpr uint32_t NULL_INDEX = std::numeric_limits<uint32_t>::max();
	inline constexpr UUID NULL_UUID = UUID{ 0 };
	inline constexpr uint32_t RASSET_SIGNATURE = 'R' << 24 | 'A' << 16 | 'S' << 8 | 'S';

	using namespace static_type_info;
	using TypeIndex = static_type_info::TypeIndex;

	enum class AssetType : uint8_t 
	{
		Undefined = 0u,
		Material,
		Texture2D,
		Mesh,
		Scene,
		Count
	};

	inline constexpr AssetType NULL_ASSET_TYPE = AssetType::Undefined;
	
#pragma pack(push, 1)
	struct RassetHeader_1
	{
		const uint32_t Signature = RASSET_SIGNATURE;
		uint8_t Version{1u};
		AssetType AssetType = AssetType::Undefined;
		UUID UUID = NULL_UUID;
		char Name[Rules::Limits::ASSET_NAME_LENGTH] = "\0";
		char SourcePath[Rules::Limits::ASSET_SOURCE_PATH_LENGTH] = "\0";
		uint64_t ModificationDateAndTime = 0u;
		uint16_t TagsByteSize = 0u;
	};
#pragma pack(pop)

	using LatestRassetHeaderVersion = RassetHeader_1;

	struct AssetHandle
	{
		AssetHandle(const TypeIndex& aTypeIndex = {}, const UUID& aUUID = NULL_UUID, uint32_t aSparseIndex = NULL_INDEX) noexcept;
		~AssetHandle() noexcept;

		AssetHandle(const AssetHandle& otherHandle) noexcept;
		AssetHandle& operator=(const AssetHandle& otherHandle) noexcept;

		bool operator==(const AssetHandle& other) const noexcept
		{
			return Uuid == other.Uuid && Index == other.Index;
		}

		bool operator!=(const AssetHandle& other) const noexcept
		{
			return !(*this == other);
		}

		[[nodiscard]] inline bool IsValid() const noexcept
		{
			return Uuid != NULL_UUID && Index != NULL_INDEX;
		}

		void Invalidate() noexcept
		{
			Type = {};
			Uuid = NULL_UUID;
			Index = NULL_INDEX;
		}

		TypeIndex Type;
		UUID Uuid;
		uint32_t Index;

		static const AssetHandle INVALID;
	};

	inline const AssetHandle NULL_HANDLE = AssetHandle(TypeIndex{}, NULL_UUID, NULL_INDEX);
	inline const AssetHandle AssetHandle::INVALID{TypeIndex{}, NULL_UUID, NULL_INDEX };
}