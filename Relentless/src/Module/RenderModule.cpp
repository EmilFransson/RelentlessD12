#include "RenderModule.h"

#include "Core/Application.h"

#include "Graphics/Renderer/Service/IBLGenerationService.h"
#include "Graphics/Renderer/Service/RenderBakeService.h"

namespace Relentless
{
	RenderModule::RenderModule() noexcept = default;
	RenderModule::~RenderModule() noexcept = default;

	const UniquePtr<IBLGenerationService>& RenderModule::GetIBLGenerationService() const noexcept
	{
		return m_pIBLGenerationService;
	}

	const UniquePtr<RenderBakeService>& RenderModule::GetRenderBakeService() const noexcept
	{
		return m_pRenderBakeService;
	}

	void RenderModule::OnLoad()
	{
		GraphicsDevice* pGraphicsDevice = Application::Get().GetGraphicsDevice();

		m_pIBLGenerationService = MakeUnique<IBLGenerationService>(pGraphicsDevice);
		m_pRenderBakeService = MakeUnique<RenderBakeService>(pGraphicsDevice);
	}
}
