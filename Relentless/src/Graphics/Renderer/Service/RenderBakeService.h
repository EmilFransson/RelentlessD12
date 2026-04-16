#pragma once

#include "Graphics/RHI/Texture.h"
#include "Graphics/RHI/PipelineState.h"

namespace Relentless
{
	class GraphicsDevice;
	class RenderJobHandle;

	struct RLS_API EquirectangularToCubemapSpecification
	{
		Ref<Texture> EquirectangularTexture	= nullptr;
		uint32 CubeFaceDimension			= 512u;
	};

	class RLS_API RenderBakeService
	{
	public:
		RenderBakeService(GraphicsDevice* aGraphicsDevice) noexcept;
		~RenderBakeService() noexcept;
		RenderJobHandle RequestEquirectangularToCubemapConversion(EquirectangularToCubemapSpecification& aSpecification, Ref<Texture>& aOutCubemap) noexcept;
	private:
		GraphicsDevice* m_pGraphicsDevice = nullptr;
		Ref<PipelineState> m_pEquirectToCubemapPSO = nullptr;
		Ref<PipelineState> m_pCubeMipGenPSO = nullptr;
	};
}