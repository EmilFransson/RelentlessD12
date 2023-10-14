#pragma once
#include "IResource.h"
namespace Relentless
{
	struct TextureSpecification
	{
		uint32_t Width;
		uint32_t Height;
		DXGI_FORMAT Format;
		DirectX::XMFLOAT4 ClearColor;
	};

	struct RenderTextureSpecification : public TextureSpecification
	{
		DXGI_FORMAT Format;
		uint8_t MultiSampleCount;
		bool CreateSRV;
		bool isSRGB{true};
		D3D12_RESOURCE_FLAGS Flags = D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	};

	class Texture : public IResource
	{
	public:
		[[nodiscard]] constexpr const uint32_t GetWidth() const noexcept { return m_Width; }
		[[nodiscard]] constexpr const uint32_t GetHeight() const noexcept { return m_Height; }
		[[nodiscard]] constexpr const std::pair<uint32_t, uint32_t>& GetDimensions() const noexcept { return { m_Width, m_Height }; }
		[[nodiscard]] constexpr const DXGI_FORMAT GetFormat() const noexcept { return m_Format; }
		[[nodiscard]] constexpr const uint8_t GetMultiSampleCount() const noexcept { return m_MSAACount; }
		[[nodiscard]] constexpr const DirectX::XMFLOAT4& GetClearColor() const noexcept { return m_ClearColor; }
		[[nodiscard]] constexpr const DescriptorHandle& GetSRVDescriptorHandle() const noexcept { return m_SRVDescriptorHandle; }
	protected:
		Texture(const RenderTextureSpecification& textureSpecification, const std::string& name = "?") noexcept;
		Texture() noexcept = default;
		virtual ~Texture() noexcept override = default;
		Texture(Texture&& other) noexcept = default;
		Texture& operator=(Texture&& other) noexcept = default;
	protected:
		DescriptorHandle m_SRVDescriptorHandle;
	private:
		uint32_t m_Width;
		uint32_t m_Height;
		DXGI_FORMAT m_Format;
		DirectX::XMFLOAT4 m_ClearColor;
		uint8_t m_MSAACount;
	};

	class RenderTexture : public Texture
	{
	public:
		RenderTexture(const RenderTextureSpecification& textureSpecification, const std::string& name = "?") noexcept;
		virtual ~RenderTexture() noexcept override final = default;
		[[nodiscard]] static std::shared_ptr<RenderTexture> Create(RenderTextureSpecification& textureSpecification, const std::string& name = "?") noexcept;
		[[nodiscard]] constexpr const DescriptorHandle& GetRTVDescriptorHandle() const noexcept { return m_RTVDescriptorHandle; }
	private:
		DescriptorHandle m_RTVDescriptorHandle;
	};

	class Texture2D : public Texture 
	{
	public:
		explicit Texture2D(const std::string& fileName, bool srgb = false) noexcept;
		Texture2D() noexcept {}
		virtual ~Texture2D() noexcept override final = default;
		Texture2D(Texture2D&& other) noexcept = default;
		Texture2D& operator=(Texture2D&& other) noexcept = default;
		[[nodiscard]] static std::shared_ptr<Texture2D> Create(const std::string& fileName, bool srgb = false) noexcept;
		[[nodiscard]] static std::shared_ptr<Texture2D> CreateFromPath(const std::filesystem::path& filePath, bool srgb = false) noexcept;
		[[nodiscard]] static Texture2D LoadFromFile(const std::filesystem::path& filePath) noexcept { return {}; }
	};
}