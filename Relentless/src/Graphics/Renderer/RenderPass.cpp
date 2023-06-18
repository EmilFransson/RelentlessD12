#include "RenderPass.h"
namespace Relentless
{
	RenderPass::RenderPass(const RenderPassSpecification& renderPassSpecification) noexcept
		: m_RenderPassSpecification{ renderPassSpecification }
	{
		RLS_ASSERT(renderPassSpecification.RenderPipeline, "No valid pipeline submitted for render pass.");
	}

	std::shared_ptr<RenderPass> RenderPass::Create(const RenderPassSpecification& renderPassSpecification) noexcept
	{
		return std::make_shared<RenderPass>(renderPassSpecification);
	}
}