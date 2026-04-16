#include "IBLGenerationService.h"

#include "Graphics/Renderer/Renderer.h"
#include "Graphics/RHI/Device.h"
#include "Graphics/RHI/CommandContext.h"
#include "Graphics/Shaders/Interop/ShaderInterop.h"

namespace Relentless
{
	IBLGenerationService::IBLGenerationService(GraphicsDevice* aGraphicsDevice) noexcept
		: m_pGraphicsDevice(aGraphicsDevice)
	{
		m_pIrradiancePSO = m_pGraphicsDevice->CreateComputePipeline(m_pGraphicsDevice->GetGlobalRootSignature(), "IrradianceConvolutionComputeShader", "cs_main");
		m_pRadiancePSO = m_pGraphicsDevice->CreateComputePipeline(m_pGraphicsDevice->GetGlobalRootSignature(), "RadianceConvolutionShader", "cs_main");
		m_pCubemapResamplePSO = m_pGraphicsDevice->CreateComputePipeline(m_pGraphicsDevice->GetGlobalRootSignature(), "CubemapResampleShader", "cs_main");
		m_pLowerHemisphereCubemapBlendPSO = m_pGraphicsDevice->CreateComputePipeline(m_pGraphicsDevice->GetGlobalRootSignature(), "CubeMapLowerHemisphereBlendShader", "cs_main");
	}

	RenderJobHandle IBLGenerationService::RequestIrradiance(const IrradianceRequest& aRequest) noexcept
	{
		return Renderer::SubmitRenderJob([this, aRequest](CommandContext& aCommandContext)
			{
				struct
				{
					uint32 RadianceMapTextureIndex;
					uint32 IrradianceMapTextureIndex;
					uint32 Dimensions;
					uint32 Samples;
					Vector4 LowerHemisphereColor;
				} passData;

				passData.RadianceMapTextureIndex = aRequest.SourceEnvironmentCubemap->GetSRVIndex();
				passData.IrradianceMapTextureIndex = aRequest.TargetIrradianceMap->GetUAVIndex();
				passData.Dimensions = aRequest.TargetIrradianceMap->GetWidth();
				passData.Samples = aRequest.NumSamples;

				aCommandContext.InsertResourceBarrier(aRequest.SourceEnvironmentCubemap, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
				aCommandContext.InsertResourceBarrier(aRequest.TargetIrradianceMap, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

				aCommandContext.SetComputeRootSignature(m_pGraphicsDevice->GetGlobalRootSignature());
				aCommandContext.SetPipelineState(m_pIrradiancePSO);

				aCommandContext.BindRootCBV(BindingSlot::PerInstance, (const void*)&passData, sizeof(passData));
				aCommandContext.Dispatch(ComputeUtils::GetNumThreadGroups(passData.Dimensions, 32u, passData.Dimensions, 32u, 6u, 1u));

				aCommandContext.InsertResourceBarrier(aRequest.SourceEnvironmentCubemap, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
				aCommandContext.InsertResourceBarrier(aRequest.TargetIrradianceMap, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			});
	}

	RenderJobHandle IBLGenerationService::RequestRadiance(const RadianceRequest& aRequest) noexcept
	{
		return Renderer::SubmitRenderJob([this, aRequest](CommandContext& aCommandContext)
			{
				aCommandContext.InsertResourceBarrier(aRequest.SourceEnvironmentCubemap, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
				aCommandContext.InsertResourceBarrier(aRequest.TargetRadianceMap, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

				const uint32 totalMips = aRequest.TargetRadianceMap->GetMipLevels();
				const uint32 lastMip = Math::Min(aRequest.Mips.LastMip(), totalMips - 1u);

				for (uint32 mip = aRequest.Mips.FirstMip; mip <= lastMip; ++mip)
				{
					if (mip == 0u)
						ResampleCubemap(aCommandContext, aRequest.SourceEnvironmentCubemap, aRequest.TargetRadianceMap, mip);
					else
						ConvolveRadiance(aCommandContext, aRequest.SourceEnvironmentCubemap, aRequest.TargetRadianceMap, mip);
				}

				aCommandContext.InsertResourceBarrier(aRequest.SourceEnvironmentCubemap, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
				aCommandContext.InsertResourceBarrier(aRequest.TargetRadianceMap, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			});
	}

	RenderJobHandle IBLGenerationService::RequestLowerHemisphereBlend(const LowerHemisphereCubemapBlendRequest& aRequest) noexcept
	{
		return Renderer::SubmitRenderJob([this, aRequest](CommandContext& aCommandContext)
			{
				RLS_ASSERT(aRequest.SrcEnvironmentMap->GetMipLevels() == aRequest.DstEnvironmentMap->GetMipLevels(), "[IBLGenerationService::RequestLowerHemisphereBlend]: Source & Target cubemap mip count mismatch.");
				RLS_ASSERT(aRequest.SrcEnvironmentMap->GetWidth() == aRequest.DstEnvironmentMap->GetWidth(), "[IBLGenerationService::RequestLowerHemisphereBlend]: Source & Target cubemap size mismatch.");

				struct
				{
					Vector4 LowerHemisphereColor;
					uint32 SrcMapTextureIndex;
					uint32 DstMapTextureIndex;
					uint32 Size;
					float Padding;
				} passData;

				passData.LowerHemisphereColor = aRequest.LowerHemisphereColor.ToVector4();

				const uint32 numMips = aRequest.SrcEnvironmentMap->GetMipLevels();
				const uint32 size = aRequest.SrcEnvironmentMap->GetWidth();

				aCommandContext.InsertResourceBarrier(aRequest.SrcEnvironmentMap, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
				aCommandContext.InsertResourceBarrier(aRequest.DstEnvironmentMap, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

				for (uint32 mip = 0u; mip < numMips; ++mip)
				{
					passData.SrcMapTextureIndex = aRequest.SrcEnvironmentMap->GetArraySRVIndex(mip);
					passData.DstMapTextureIndex = aRequest.DstEnvironmentMap->GetUAVIndex(mip);
					passData.Size = Math::Max(1u, size >> mip);

					aCommandContext.SetComputeRootSignature(m_pGraphicsDevice->GetGlobalRootSignature());
					aCommandContext.SetPipelineState(m_pLowerHemisphereCubemapBlendPSO);

					aCommandContext.BindRootCBV(BindingSlot::PerInstance, (const void*)&passData, sizeof(passData));
					aCommandContext.Dispatch(ComputeUtils::GetNumThreadGroups(passData.Size, 8, passData.Size, 8, 6u, 1u));
				}

				aCommandContext.InsertResourceBarrier(aRequest.SrcEnvironmentMap, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
				aCommandContext.InsertResourceBarrier(aRequest.DstEnvironmentMap, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			});
	}

	void IBLGenerationService::ConvolveRadiance(CommandContext& aCommandContext, Texture* aSourceCubemap, Texture* aTargetRadianceMap, uint32 aMip) noexcept
	{
		struct
		{
			uint32 InputTextureIndex;
			uint32 OutputTextureIndex;
			uint32 InputDimensions;
			uint32 OutputDimensions;
			Vector4 LowerHemisphereColor;
			float Roughness;
			float Padding[3] = {};
		} passData;

		passData.InputTextureIndex = aSourceCubemap->GetSRVIndex();
		passData.OutputTextureIndex = aTargetRadianceMap->GetUAVIndex(aMip);
		passData.InputDimensions = aSourceCubemap->GetWidth();
		passData.OutputDimensions = aTargetRadianceMap->GetWidth() >> aMip;
		passData.Roughness = static_cast<float>(aMip) / static_cast<float>(aTargetRadianceMap->GetMipLevels() - 1u);

		aCommandContext.SetComputeRootSignature(m_pGraphicsDevice->GetGlobalRootSignature());
		aCommandContext.SetPipelineState(m_pRadiancePSO);

		aCommandContext.BindRootCBV(BindingSlot::PerInstance, (const void*)&passData, sizeof(passData));
		aCommandContext.Dispatch(ComputeUtils::GetNumThreadGroups(passData.OutputDimensions, 32u, passData.OutputDimensions, 32u, 6u, 1u));
	}

	void IBLGenerationService::ResampleCubemap(CommandContext& aCommandContext, Texture* aSourceCubemap, Texture* aTargetRadianceMap, uint32 aMip) noexcept
	{
		struct
		{
			uint32 SrcTextureIndex;
			uint32 DstTextureIndex;
			uint32 SrcSize;
			uint32 DstSize;
		} passData;

		passData.SrcTextureIndex = aSourceCubemap->GetSRVIndex();
		passData.DstTextureIndex = aTargetRadianceMap->GetUAVIndex(0);
		passData.SrcSize = aSourceCubemap->GetWidth();
		passData.DstSize = aTargetRadianceMap->GetWidth();

		aCommandContext.SetComputeRootSignature(m_pGraphicsDevice->GetGlobalRootSignature());
		aCommandContext.SetPipelineState(m_pCubemapResamplePSO);

		aCommandContext.BindRootCBV(BindingSlot::PerInstance, (const void*)&passData, sizeof(passData));
		aCommandContext.Dispatch(ComputeUtils::GetNumThreadGroups(passData.DstSize, 8, passData.DstSize, 8, 6, 1));
	}
}