#pragma once

#include "Graphics/RHI/Texture.h"
#include "Graphics/RHI/PipelineState.h"

namespace Relentless
{
	class RenderJobHandle;

	struct RLS_API EquirectangularToCubemapSpecification
	{
		Ref<Texture> EquirectangularTexture	= nullptr;
		uint32 CubeFaceDimension			= 512u;
	};

	class RLS_API RenderBakeService
	{
	public:
		RenderBakeService() noexcept;
		~RenderBakeService() noexcept;
		RenderJobHandle RequestEquirectangularToCubemapConversion(EquirectangularToCubemapSpecification& aSpecification, Ref<Texture>& aOutCubemap) noexcept;
	private:
		Ref<PipelineState> m_pEquirectToCubemapPSO = nullptr;
	};
}