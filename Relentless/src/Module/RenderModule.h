#pragma once
#include "IModule.h"

namespace Relentless
{
	class IBLGenerationService;
	class RenderBakeService;

	class RLS_API RenderModule : public IModule
	{
	public:
		RenderModule() noexcept;
		~RenderModule() noexcept;

		NO_DISCARD const UniquePtr<IBLGenerationService>& GetIBLGenerationService() const noexcept;
		NO_DISCARD const UniquePtr<RenderBakeService>& GetRenderBakeService() const noexcept;
	protected:
		virtual void OnLoad() override;
	private:
		UniquePtr<IBLGenerationService> m_pIBLGenerationService;
		UniquePtr<RenderBakeService> m_pRenderBakeService;
	};
}