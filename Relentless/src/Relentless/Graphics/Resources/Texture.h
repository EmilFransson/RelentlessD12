#pragma once
#include "IResource.h"
namespace Relentless
{
	struct TextureSpecification
	{
		uint32_t Width;
		uint32_t Height;
		DXGI_FORMAT Format;
		uint8_t MultiSampleCount;
		DirectX::XMFLOAT4 ClearColor;
		bool CreateDescriptorHandles;
	};

	class Texture : public IResource
	{
	public:
		Texture(const TextureSpecification& textureSpecification, const std::string& name = "?") noexcept;
		Texture(const std::string& name) noexcept;
		Texture() noexcept;
		virtual ~Texture() noexcept override = default;
		[[nodiscard]] constexpr const uint32_t GetWidth() const noexcept { return m_TextureSpecification.Width; }
		[[nodiscard]] constexpr const uint32_t GetHeight() const noexcept { return m_TextureSpecification.Height; }
		[[nodiscard]] constexpr const std::pair<uint32_t, uint32_t>& GetDimensions() const noexcept { return std::make_pair(m_TextureSpecification.Width, m_TextureSpecification.Height); }
		[[nodiscard]] constexpr const DXGI_FORMAT GetFormat() const noexcept { return m_TextureSpecification.Format; }
		[[nodiscard]] constexpr const uint8_t GetMultiSampleCount() const noexcept { return m_TextureSpecification.MultiSampleCount; }
		[[nodiscard]] constexpr const DescriptorHandle& GetSRVDescriptorHandle() const noexcept { return m_SRVDescriptorHandle; }
	protected:
		TextureSpecification m_TextureSpecification;
		DescriptorHandle m_SRVDescriptorHandle;
	};

	class RenderTexture : public Texture
	{
	public:
		RenderTexture(const uint32_t width, const uint32_t height, const std::string& name = "?") noexcept;
		RenderTexture(const TextureSpecification& textureSpecification, const std::string& name = "?") noexcept;
		virtual ~RenderTexture() noexcept override final = default;
		[[nodiscard]] static std::shared_ptr<RenderTexture> Create(const uint32_t width, const uint32_t height, const std::string& name = "?") noexcept;
		[[nodiscard]] static std::shared_ptr<RenderTexture> Create(const TextureSpecification& textureSpecification, const std::string& name = "?") noexcept;
		[[nodiscard]] constexpr const DescriptorHandle& GetRTVDescriptorHandle() const noexcept { return m_RTVDescriptorHandle; }
	private:
		DescriptorHandle m_RTVDescriptorHandle;
	};
}