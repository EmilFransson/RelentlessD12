#pragma once
namespace Relentless
{
	//Forward declarations:
	class Mesh;
	class Material;
	class Scene;
	class Texture2D;
	struct AssetHandle;

#pragma pack(push, 1)
	struct MeshDataHeader
	{
		uint64_t VertexBufferSizeInBytes;
		uint64_t IndexBufferSizeInBytes;
		uint32_t VertexCount;
		uint32_t IndexCount;
	};
#pragma pack(pop)

	struct RassetHeader_1;

	class Serializer
	{
	public:
	private:
		template<typename>
		struct always_false : std::false_type {};

	public:
		template<typename AssetType>
		static [[nodiscard]] bool Serialize(const AssetHandle& assetHandle, const std::filesystem::path& filepath) noexcept
		{
			static_assert(always_false<AssetType>::value, "This operation is not supported by the type, or the type does not exist.");
		}

		template<typename AssetType>
		static [[nodiscard]] bool Deserialize(const std::filesystem::path& filepath, AssetHandle& outHandle) noexcept
		{
			static_assert(always_false<AssetType>::value, "This operation is not supported by the type, or the type does not exist.");
		}

		template<typename AssetType>
		static [[nodiscard]] bool Deserialize(const std::filesystem::path& filepath) noexcept
		{
			AssetHandle tempHandle;
			return Deserialize<AssetType>(filepath, tempHandle);
		}

		static bool Serialize(const std::filesystem::path& filepath, const AssetHandle& assetHandle) noexcept;
		static bool Deserialize(const std::filesystem::path& filepath, AssetHandle& outHandle) noexcept;
		static [[nodiscard]] std::pair<uint32_t, uint8_t> DeserializeSignatureAndVersion(std::ifstream& ifstream) noexcept;
		static [[nodiscard]] RassetHeader_1 DeserializeRAssetHeaderVersion1(std::ifstream& ifstream) noexcept;
		static [[nodiscard]] std::vector<std::string> DeserializeAssetTags(std::ifstream& ifstream, uint32_t tagsByteSize) noexcept;
	private:
		static [[nodiscard]] std::pair<uint32_t, uint8_t> DeserializeHeaderSignatureAndVersion(std::ifstream& ifstream) noexcept;
		static void SerializeRassetHeader(const AssetHandle& assetHandle, std::ofstream& outFileStream) noexcept;
		static void SerializeAssetTags(const AssetHandle& assetHandle, std::ofstream& outFileStream) noexcept;
	};

	template<>
	bool Serializer::Serialize<Material>(const AssetHandle& assetHandle, const std::filesystem::path& filepath) noexcept;

	template<>
	bool Serializer::Serialize<Texture2D>(const AssetHandle& assetHandle, const std::filesystem::path& filepath) noexcept;

	template<>
	bool Serializer::Serialize<Mesh>(const AssetHandle& assetHandle, const std::filesystem::path& filepath) noexcept;

	template<>
	bool Serializer::Deserialize<Mesh>(const std::filesystem::path& filepath, AssetHandle& outHandle) noexcept;

	template<>
	bool Serializer::Deserialize<Texture2D>(const std::filesystem::path& filepath, AssetHandle& outHandle) noexcept;

	template<>
	bool Serializer::Deserialize<Material>(const std::filesystem::path& filepath, AssetHandle& outHandle) noexcept;
}