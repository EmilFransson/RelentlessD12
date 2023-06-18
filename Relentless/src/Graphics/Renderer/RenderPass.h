#pragma once
#include "Pipeline.h"
namespace Relentless
{
	struct RenderPassSpecification
	{
		std::string DebugName{ "?" };
		std::shared_ptr<Pipeline> RenderPipeline;
	};
	
	class RenderPass
	{
	public:
		explicit RenderPass(const RenderPassSpecification& renderPassDescriptor) noexcept;
		~RenderPass() noexcept = default;
		static [[nodiscard]] std::shared_ptr<RenderPass> Create(const RenderPassSpecification& renderPassSpecification) noexcept;
		[[nodiscard]] const RenderPassSpecification& GetDescriptor() const noexcept { return m_RenderPassSpecification; }
		[[nodiscard]] const std::shared_ptr<Pipeline>& GetPipeline() const noexcept { return m_RenderPassSpecification.RenderPipeline; }
	private:
		RenderPassSpecification m_RenderPassSpecification;
	};
}