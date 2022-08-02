#pragma once
#include "IResource.h"
namespace Relentless
{
	class RenderTexture : public IResource
	{
	public:
		RenderTexture(const uint32_t width, const uint32_t height) noexcept;
		virtual ~RenderTexture() noexcept override = default;
		[[nodiscard]] static std::shared_ptr<RenderTexture> Create(const uint32_t width, const uint32_t height) noexcept;
		[[nodiscard]] constexpr const DescriptorHandle& GetRTVDescriptorHandle() const noexcept { return m_RTVDescriptorHandle; }
		[[nodiscard]] constexpr const DescriptorHandle& GetSRVDescriptorHandle() const noexcept { return m_SRVDescriptorHandle; }
		uint32_t m_Width;
		uint32_t m_Height;
	private:
		DXGI_FORMAT m_Format;
		DescriptorHandle m_RTVDescriptorHandle;
		DescriptorHandle m_SRVDescriptorHandle;
	};

	class RenderTextureMSAA : public IResource
	{
	public:
		RenderTextureMSAA(const uint32_t width, const uint32_t height, const uint8_t multiSampleCount) noexcept;
		virtual ~RenderTextureMSAA() noexcept override = default;
		[[nodiscard]] static std::shared_ptr<RenderTextureMSAA> Create(const uint32_t width, const uint32_t height, const uint8_t multiSampleCount) noexcept;
		[[nodiscard]] constexpr const DescriptorHandle& GetRTVDescriptorHandle() const noexcept { return m_RTVDescriptorHandle; }
		[[nodiscard]] constexpr const DescriptorHandle& GetSRVDescriptorHandle() const noexcept { return m_SRVDescriptorHandle; }
		uint32_t m_Width;
		uint32_t m_Height;
	private:
		DXGI_FORMAT m_Format;
		DescriptorHandle m_RTVDescriptorHandle;
		DescriptorHandle m_SRVDescriptorHandle;
		uint8_t m_MultiSampleCount;
	};
}