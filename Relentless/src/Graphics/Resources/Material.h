#pragma once

namespace std
{
	template<>
	struct hash<UUID>
	{
		std::size_t operator()(const UUID& gUid) const
		{
			const uint64_t* half = reinterpret_cast<const uint64_t*>(&gUid);
			return half[0] ^ half[1];
		}
	};
}

namespace Relentless
{
	typedef UUID ResourceID;
	typedef ResourceID MaterialHandle;
	#define NULL_RESOURCEID UUID(0)

	class Texture2D;
	class Material
	{
	public:
		explicit Material() noexcept;
		~Material() noexcept = default;
		void SetAlbedoTexture(const ResourceID& albedoTextureHandle) noexcept;
		void SetMetallicTexture(const ResourceID& metallicTextureHandle) noexcept;
		void SetRoughnessTexture(const ResourceID& roughnessTextureHandle) noexcept;
		void SetNormalMap(const ResourceID& normalMapHandle) noexcept;
		void SetName(const std::string& materialName) noexcept { m_Name = materialName; }
		[[nodiscard]] bool HasAlbedoTexture() const noexcept;
		[[nodiscard]] bool HasMetallicTexture() const noexcept;
		[[nodiscard]] bool HasRoughnessTexture() const noexcept;
		[[nodiscard]] bool HasNormalMap() const noexcept;
		[[nodiscard]] bool ShouldUseAlbedoTexture() const noexcept;
		[[nodiscard]] bool ShouldUseMetallicTexture() const noexcept;
		[[nodiscard]] bool ShouldUseRoughnessTexture() const noexcept;
		[[nodiscard]] bool ShouldUseNormalMap() const noexcept;
		[[nodiscard]] Texture2D* GetAlbedoTexture() const noexcept;
		[[nodiscard]] Texture2D* GetMetallicTexture() const noexcept;
		[[nodiscard]] Texture2D* GetRoughnessTexture() const noexcept;
		[[nodiscard]] Texture2D* GetNormalMap() const noexcept;
		[[nodiscard]] const std::string& GetName() const noexcept { return m_Name; }
		[[nodiscard]] size_t GetConstantBufferIndex() const noexcept;
		static void UploadToGPU(const MaterialHandle& materialHandle) noexcept;
		void ToggleAlbedoTextureUsage() noexcept;
		void ToggleMetallicTextureUsage() noexcept;
		void ToggleRoughnessTextureUsage() noexcept;
		void ToggleNormalMapUsage() noexcept;

	public:
		friend class MaterialManager;
		
		DirectX::XMFLOAT3 m_AlbedoColor;
		float m_Metallic;
		float m_Roughness;
	private:
		uint32_t m_AlbedoTextureIndex;
		uint32_t m_MetallicTextureIndex;
		uint32_t m_RoughnessTextureIndex;
		uint32_t m_NormalMapIndex;

		std::string m_Name;
		ResourceID m_AlbedoTextureHandle;
		ResourceID m_MetallicTextureHandle;
		ResourceID m_RoughnessTextureHandle;
		ResourceID m_NormalMapHandle;

		bool m_UseAlbedoTexture;
		bool m_UseMetallicTexture;
		bool m_UseRoughnessTexture;
		bool m_UseNormalMap;

		size_t m_ConstantBufferID;
	};

	
	class MaterialManager
	{
	public:
		explicit MaterialManager() noexcept = default;
		~MaterialManager() noexcept = default;
		[[nodiscard]] Material& Get(const MaterialHandle& materialHandle) noexcept;
		[[nodiscard]] MaterialHandle Create(const std::string& name = std::string("Unnamed Material"), const Material & = Material()) noexcept;
		__forceinline void Upload(const MaterialHandle& materialHandle) noexcept;
	private:
		std::unordered_map<MaterialHandle, Material> m_IDToMaterialMap;
	};
}