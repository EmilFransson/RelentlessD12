#include "RenderModule.h"

#include "Graphics/Renderer/Service/RenderBakeService.h"

namespace Relentless
{
	RenderModule::RenderModule() noexcept = default;
	RenderModule::~RenderModule() noexcept = default;

	const UniquePtr<RenderBakeService>& RenderModule::GetRenderBakeService() const noexcept
	{
		return m_pRenderBakeService;
	}

	void RenderModule::OnLoad()
	{
		m_pRenderBakeService = MakeUnique<RenderBakeService>();
	}
}
