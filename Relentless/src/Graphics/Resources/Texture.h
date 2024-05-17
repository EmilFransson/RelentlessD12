#pragma once
#include "IResource.h"
namespace Relentless
{
	struct TextureSpecification
	{
		uint32_t Width{ 0u };
		uint32_t Height{ 0u };
		DXGI_FORMAT Format{ DXGI_FORMAT::DXGI_FORMAT_UNKNOWN };
	};

	struct RenderTextureSpecification : public TextureSpecification
	{
		uint8_t MultiSampleCount;
		bool CreateSRV;
		bool isSRGB{true};
		D3D12_RESOURCE_FLAGS Flags{ D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET };
		DirectX::XMFLOAT4 ClearColor;
	};

	struct Texture2DSpecification : public TextureSpecification
	{
		Microsoft::WRL::ComPtr<ID3D12Resource> pResource{ nullptr };
		DescriptorHandle DescriptorHandleSRV;
		std::string Name{ "?" };
		bool IsSRGB{ true };
		uint32_t MipCount{0u};
		uint32_t SampleCount{0u};
	};

	class Texture : public IResource
	{
	public:
		[[nodiscard]] constexpr const uint32_t GetWidth() const noexcept { return m_Width; }
		[[nodiscard]] constexpr const uint32_t GetHeight() const noexcept { return m_Height; }
		[[nodiscard]] constexpr const std::pair<uint32_t, uint32_t>& GetDimensions() const noexcept { return { m_Width, m_Height }; }
		[[nodiscard]] constexpr const DXGI_FORMAT GetFormat() const noexcept { return m_Format; }
		[[nodiscard]] constexpr const uint8_t GetMultiSampleCount() const noexcept { return m_Samples; }
		
		[[nodiscard]] constexpr const DescriptorHandle& GetSRVDescriptorHandle() const noexcept { return m_SRVDescriptorHandle; }
		[[nodiscard]] constexpr const uint32_t GetMipCount() const noexcept { return m_MipCount; }
	protected:
		Texture(const RenderTextureSpecification& textureSpecification, const std::string& name = "?") noexcept;
		Texture();
		Texture(Texture& otherTexture) noexcept = delete;
		virtual ~Texture() noexcept override = default;
		Texture(Texture&& otherTexture) noexcept;
		Texture& operator=(Texture&& otherTexture) noexcept;
	protected:
		DescriptorHandle m_SRVDescriptorHandle;
		uint32_t m_Width;
		uint32_t m_Height;
		DXGI_FORMAT m_Format;
		uint8_t m_Samples;
		uint32_t m_MipCount;
	};

	class RenderTexture : public Texture
	{
	public:
		RenderTexture(const RenderTextureSpecification& textureSpecification, const std::string& name = "?") noexcept;
		virtual ~RenderTexture() noexcept override final = default;
		[[nodiscard]] static std::shared_ptr<RenderTexture> Create(RenderTextureSpecification& textureSpecification, const std::string& name = "?") noexcept;
		[[nodiscard]] constexpr const DescriptorHandle& GetRTVDescriptorHandle() const noexcept { return m_RTVDescriptorHandle; }
		[[nodiscard]] constexpr const DirectX::XMFLOAT4& GetClearColor() const noexcept { return m_ClearColor; }
	private:
		DescriptorHandle m_RTVDescriptorHandle;
		DirectX::XMFLOAT4 m_ClearColor;
	};

	class Texture2D : public Texture 
	{
	public:
		explicit Texture2D(const std::string& fileName, bool srgb = false) noexcept;
		virtual ~Texture2D() noexcept override final = default;
		Texture2D(Texture2D& otherTexture2D) noexcept = delete;
		Texture2D(Texture2D&& otherTexture2D) noexcept;
		explicit Texture2D(const Texture2DSpecification& specification) noexcept;
		Texture2D& operator=(Texture2D&& otherTexture) noexcept;
		[[nodiscard]] static std::shared_ptr<Texture2D> Create(const std::string& fileName, bool srgb = false) noexcept;
		[[nodiscard]] static Texture2D LoadFromFile(const std::filesystem::path& filePath) noexcept;
		[[nodiscard]] bool IsSRGB() const { return m_IsSRGB; }
	private:
		bool m_IsSRGB;
	};
}