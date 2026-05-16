#pragma once

#include "Graphics/RHI/Texture.h"
#include "Graphics/RHI/PipelineState.h"

namespace Relentless
{
	struct MipRange
	{
		uint32 FirstMip = 0;
		uint32 MipCount = 1;

		NO_DISCARD uint32 LastMip() const noexcept { return FirstMip + MipCount - 1; }
		NO_DISCARD bool   IsValid() const noexcept { return MipCount > 0; }
	};

	struct IrradianceRequest
	{
		Texture* SourceEnvironmentCubemap = nullptr;
		Texture* TargetIrradianceMap = nullptr;
		uint32 NumSamples = 8u;
	};

	struct RadianceRequest
	{
		Texture* SourceEnvironmentCubemap = nullptr;
		Texture* TargetRadianceMap = nullptr;
		MipRange Mips = {};
	};

	struct LowerHemisphereCubemapBlendRequest
	{
		Texture* SrcEnvironmentMap = nullptr;
		Texture* DstEnvironmentMap = nullptr;
		Color LowerHemisphereColor = Colors::Black;
	};

	class GraphicsDevice;
	class RenderJobHandle;

	class IBLGenerationService
	{
	public:
		IBLGenerationService(GraphicsDevice* aGraphicsDevice) noexcept;

		RenderJobHandle RequestIrradiance(const IrradianceRequest& aRequest) noexcept;
		RenderJobHandle RequestRadiance(const RadianceRequest& aRequest) noexcept;
		RenderJobHandle RequestLowerHemisphereBlend(const LowerHemisphereCubemapBlendRequest& aRequest) noexcept;
	private:
		void ConvolveRadiance(CommandContext& aCommandContext, Texture* aSourceCubemap, Texture* aTargetRadianceMap, uint32 aMip) noexcept;

		void ResampleCubemap(CommandContext& aCommandContext, Texture* aSourceCubemap, Texture* aTargetRadianceMap) noexcept;
	private:
		Ref<PipelineState> m_pIrradiancePSO = nullptr;
		Ref<PipelineState> m_pRadiancePSO = nullptr;
		Ref<PipelineState> m_pCubemapResamplePSO = nullptr;
		Ref<PipelineState> m_pLowerHemisphereCubemapBlendPSO = nullptr;

		GraphicsDevice* m_pGraphicsDevice = nullptr;
	};
}