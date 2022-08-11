#pragma once
#include "IResource.h"
namespace Relentless
{
	class DepthStencil : public IResource
	{
	public:
		DepthStencil(const uint32_t width, const uint32_t height, const uint8_t multiSampleCount, const std::string& name = "DepthStencil") noexcept;
		virtual ~DepthStencil() noexcept override final = default;
		[[nodiscard]] static std::shared_ptr<DepthStencil> Create(const uint32_t width, const uint32_t height, const uint8_t multiSampleCount, const std::string& name = "DepthStencil") noexcept;
		[[nodiscard]] constexpr const DescriptorHandle& GetDSVDescriptorHandle() const { return m_DSVDescriptorHandle; }
	private:
		uint32_t m_Width;
		uint32_t m_Height;
		DXGI_FORMAT m_Format;
		uint8_t m_MultiSampleCount;
		DescriptorHandle m_DSVDescriptorHandle;
	};
}