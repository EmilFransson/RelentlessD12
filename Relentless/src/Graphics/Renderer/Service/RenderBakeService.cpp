#include "RenderBakeService.h"

#include "Core/Application.h"

#include "Graphics/Renderer/Renderer.h"
#include "Graphics/RHI/CommandContext.h"
#include "Graphics/Shaders/Interop/ShaderInterop.h"

namespace Relentless
{
	RenderBakeService::RenderBakeService(GraphicsDevice* aGraphicsDevice) noexcept
		: m_pGraphicsDevice{aGraphicsDevice}
	{
		m_pEquirectToCubemapPSO = m_pGraphicsDevice->CreateComputePipeline(m_pGraphicsDevice->GetGlobalRootSignature(), "EquirectangularToCubemapComputeShader", "cs_main");
		m_pCubeMipGenPSO = m_pGraphicsDevice->CreateComputePipeline(m_pGraphicsDevice->GetGlobalRootSignature(), "CubeMipGenerationShader", "cs_main");
	}

	RenderBakeService::~RenderBakeService() noexcept = default;

	RenderJobHandle RenderBakeService::RequestEquirectangularToCubemapConversion(EquirectangularToCubemapSpecification& aSpecification, Ref<Texture>& aOutCubemap) noexcept
	{
		return Renderer::SubmitComputeJob([this, aSpecification, &aOutCubemap](CommandContext& aCommandContext) mutable
			{
				const String name = aSpecification.EquirectangularTexture->GetName() + "_CubeMap";
				const uint32 dimension = aSpecification.CubeFaceDimension;
				const uint32 numMips = static_cast<uint32>(std::floor(Math::Log2f(static_cast<float>(dimension)) + 1.0f));

				Ref<Texture> pTextureCube = m_pGraphicsDevice->CreateTexture(TextureDesc::CreateCube(dimension, dimension, ResourceFormat::RGBA32_FLOAT, numMips, TextureFlag::UnorderedAccess | TextureFlag::ShaderResource), name.c_str());
				
				aCommandContext.InsertResourceBarrier(pTextureCube, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
				aCommandContext.InsertResourceBarrier(aSpecification.EquirectangularTexture, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

				struct
				{
					uint32 EquirectangularTextureIndex = ShaderInterop::INVALID_DESCRIPTOR_INDEX;
					uint32 OutputCubeIndex = ShaderInterop::INVALID_DESCRIPTOR_INDEX;
					uint32 CubeDimensions = 0u;
					float Padding = 0.0f;
				} passData;

				passData.EquirectangularTextureIndex = aSpecification.EquirectangularTexture->GetSRVIndex();
				passData.OutputCubeIndex = pTextureCube->GetUAVIndex();
				passData.CubeDimensions = aSpecification.CubeFaceDimension;

				aCommandContext.SetComputeRootSignature(m_pGraphicsDevice->GetGlobalRootSignature());
				aCommandContext.SetPipelineState(m_pEquirectToCubemapPSO);
				aCommandContext.BindRootCBV(BindingSlot::PerInstance, (const void*)&passData, sizeof(passData));
				aCommandContext.Dispatch(ComputeUtils::GetNumThreadGroups(aSpecification.CubeFaceDimension, 32, aSpecification.CubeFaceDimension, 32, 6, 1));
				
				aCommandContext.InsertUAVBarrier();

				for (uint32 mip = 0u; mip < numMips - 1u; ++mip)
				{
					const uint32 srcMip = mip;
					const uint32 dstMip = mip + 1u;

					struct
					{
						uint32 SrcTextureIndex;
						uint32 DstTextureIndex;
						uint32 SrcTextureDimension;
						uint32 DstTextureDimension;
					} passData2;

					passData2.SrcTextureIndex = pTextureCube->GetArraySRVIndex(srcMip);
					passData2.SrcTextureDimension = Math::Max(1u, dimension >> srcMip);
					passData2.DstTextureIndex = pTextureCube->GetUAVIndex(dstMip);
					passData2.DstTextureDimension = Math::Max(1u, dimension >> dstMip);

					aCommandContext.SetComputeRootSignature(m_pGraphicsDevice->GetGlobalRootSignature());
					aCommandContext.SetPipelineState(m_pCubeMipGenPSO);
					aCommandContext.BindRootCBV(BindingSlot::PerInstance, (const void*)&passData2, sizeof(passData2));
					aCommandContext.Dispatch(ComputeUtils::GetNumThreadGroups(passData2.DstTextureDimension, 32, passData2.DstTextureDimension, 32, 6, 1));

					if (mip < numMips - 2u)
						aCommandContext.InsertUAVBarrier();
				}
				aOutCubemap = pTextureCube;
			});
	}
}