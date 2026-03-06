#pragma once
#include "IModule.h"

namespace Relentless
{
	class RenderBakeService;

	class RLS_API RenderModule : public IModule
	{
	public:
		RenderModule() noexcept;
		~RenderModule() noexcept;

		NO_DISCARD const UniquePtr<RenderBakeService>& GetRenderBakeService() const noexcept;
	protected:
		virtual void OnLoad() override;
	private:
		UniquePtr<RenderBakeService> m_pRenderBakeService;
	};
}