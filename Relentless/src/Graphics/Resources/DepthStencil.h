#pragma once
#include "IResource.h"
namespace Relentless
{
	struct DepthStencilSpecification
	{
		uint32_t Width{800u};
		uint32_t Height{600u};
		DXGI_FORMAT Format{DXGI_FORMAT_D32_FLOAT};
		D3D12_RESOURCE_FLAGS Flags{D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL};
		uint8_t Samples{1u};
		bool CreateSRV{false};
	};

	class DepthStencil : public IResource
	{
	public:
		DepthStencil(const DepthStencilSpecification& depthStencilDescriptor, const std::string& name = "DepthStencil") noexcept;
		virtual ~DepthStencil() noexcept override final = default;
		[[nodiscard]] static std::shared_ptr<DepthStencil> Create(const DepthStencilSpecification& depthStencilSpecification, const std::string& name = "DepthStencil") noexcept;
		[[nodiscard]] constexpr const DescriptorHandle& GetDSVDescriptorHandle() const { return m_DSVDescriptorHandle; }
		[[nodiscard]] constexpr const DescriptorHandle& GetSRVDescriptorHandle() const { return m_SRVDescriptorHandle; }
	private:
		DepthStencilSpecification m_Specification;
		DescriptorHandle m_DSVDescriptorHandle;
		DescriptorHandle m_SRVDescriptorHandle;
	};
}