#pragma once
namespace Relentless
{
	inline constexpr char ASSET_EXTENSION[] = ".rasset";
	inline constexpr uint32_t NULL_INDEX = std::numeric_limits<uint32_t>::max();
	inline constexpr UUID NULL_UUID = UUID{ 0 };

	enum class AssetType : uint8_t 
	{
		Undefined = 0u,
		Material,
		Texture2D,
		Mesh,
		Scene
	};

	inline constexpr AssetType NULL_ASSET_TYPE = AssetType::Undefined;

	struct AssetMetaData
	{
		UUID Uuid;
		AssetType AssetType;
	};

#pragma pack(push, 1)
	struct RassetHeader
	{
		static const uint32_t MagicNumber = 'R' << 24 | 'A' << 16 | 'S' << 8 | 'S';
		uint8_t Version{1u};
		AssetType AssetType;
		UUID UUID;
	};
#pragma pack(pop)

	struct AssetHandle
	{
		AssetHandle(AssetType assetType = NULL_ASSET_TYPE, UUID uuid = NULL_UUID, uint32_t index = NULL_INDEX) noexcept;
		~AssetHandle() noexcept;

		AssetHandle(const AssetHandle& otherHandle) noexcept;
		AssetHandle& operator=(const AssetHandle& otherHandle) noexcept;

		bool operator==(const AssetHandle& other) const noexcept
		{
			return Type == other.Type && Uuid == other.Uuid && Index == other.Index;
		}

		bool operator!=(const AssetHandle& other) const noexcept
		{
			return !(*this == other);
		}

		[[nodiscard]] inline bool IsValid() const noexcept
		{
			return Type != NULL_ASSET_TYPE && Uuid != NULL_UUID && Index != NULL_INDEX;
		}

		AssetType Type;
		UUID Uuid;
		uint32_t Index;
	};

	inline const AssetHandle NULL_HANDLE = AssetHandle(NULL_ASSET_TYPE, NULL_UUID, NULL_INDEX);
}