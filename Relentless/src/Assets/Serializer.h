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
		uint32_t VertexBufferSizeInBytes;
		uint32_t IndexBufferSizeInBytes;
		uint32_t VertexCount;
		uint32_t IndexCount;
	};
#pragma pack(pop)

	class Serializer
	{
	public:
	private:
		template<typename>
		struct always_false : std::false_type {};

	public:
		template<typename AssetType>
		static [[nodiscard]] void Serialize(const AssetHandle& assetHandle, const std::string& path) noexcept
		{
			static_assert(always_false<AssetType>::value, "This operation is not supported by the type, or the type does not exist.");
		}

		template<typename AssetType>
		static [[nodiscard]] AssetHandle Deserialize(const std::string& filepath) noexcept
		{
			static_assert(always_false<AssetType>::value, "This operation is not supported by the type, or the type does not exist.");
		}
	};

	template<>
	void Serializer::Serialize<Material>(const AssetHandle& assetHandle, const std::string& path) noexcept;

	template<>
	void Serializer::Serialize<Texture2D>(const AssetHandle& assetHandle, const std::string& path) noexcept;

	template<>
	void Serializer::Serialize<Mesh>(const AssetHandle& assetHandle, const std::string& directoryPath) noexcept;

	template<>
	AssetHandle Serializer::Deserialize<Mesh>(const std::string& filepath) noexcept;

	template<>
	AssetHandle Serializer::Deserialize<Texture2D>(const std::string& filepath) noexcept;

	template<>
	AssetHandle Serializer::Deserialize<Material>(const std::string& filepath) noexcept;
}