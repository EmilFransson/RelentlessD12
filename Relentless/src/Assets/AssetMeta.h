#pragma once

namespace Relentless
{
	inline constexpr char ASSET_EXTENSION_EX[] = ".rasset";
	inline constexpr uint32_t NULL_INDEX_EX = std::numeric_limits<uint32_t>::max();
	inline constexpr UUID NULL_UUID_EX = UUID{ 0 };

	enum class AssetType : uint8_t 
	{
		Undefined = 0u,
		Material,
		Texture2D,
		Scene
	};

	inline constexpr AssetType NULL_ASSET_TYPE = AssetType::Undefined;

	struct RassetHeader
	{
		static const uint32_t MagicNumber = 'R' << 24 | 'A' << 16 | 'S' << 8 | 'S';
		uint8_t Version;
		AssetType AssetType;
		UUID UUID;
	};

	struct AssetHandle_EX
	{
		AssetHandle_EX(AssetType assetType = NULL_ASSET_TYPE, UUID uuid = NULL_UUID_EX, uint32_t index = NULL_INDEX_EX) noexcept;
		~AssetHandle_EX() noexcept;

		AssetHandle_EX(const AssetHandle_EX& otherHandle) noexcept;
		AssetHandle_EX& operator=(const AssetHandle_EX& otherHandle) noexcept;

		bool operator==(const AssetHandle_EX& other) const noexcept
		{
			return Type == other.Type && Uuid == other.Uuid && Index == other.Index;
		}

		bool operator!=(const AssetHandle_EX& other) const noexcept
		{
			return !(*this == other);
		}

		AssetType Type;
		UUID Uuid;
		uint32_t Index;
	};

	inline const AssetHandle_EX NULL_HANDLE_EX = AssetHandle_EX(NULL_ASSET_TYPE, NULL_UUID_EX, NULL_INDEX_EX);

}