// #include "SkyLightRenderer.h"
// 
// #include "Assets/AssetManager.h"
// #include "Assets/CoreTypes/Environment.h"
// #include "Assets/CoreTypes/Texture2D.h"
// #include "Assets/CoreTypes/TextureCube.h"
// 
// #include "Graphics/RHI/CommandContext.h"
// 
// #include "Scene/Scene.h"
// 
// namespace Relentless
// {
// 	SkyLightRenderer::SkyLightRenderer(GraphicsDevice* aGraphicsDevice) noexcept
// 		: m_pDevice{aGraphicsDevice}
// 	{
// 		m_pIrradiancePSO = m_pDevice->CreateComputePipeline(m_pDevice->GetGlobalRootSignature(), "IrradianceConvolutionComputeShader", "cs_main");
// 		m_pIrradianceLowerHemisphereColorPSO = m_pDevice->CreateComputePipeline(m_pDevice->GetGlobalRootSignature(), "IrradianceConvolutionComputeShader", "cs_main", { "LOWER_HEMISPHERE_SOLID_COLOR" });
// 
// 		m_pRadiancePSO = m_pDevice->CreateComputePipeline(m_pDevice->GetGlobalRootSignature(), "RadianceConvolutionShader", "cs_main");
// 		m_pRadianceLowerHemisphereColorPSO = m_pDevice->CreateComputePipeline(m_pDevice->GetGlobalRootSignature(), "RadianceConvolutionShader", "cs_main", { "LOWER_HEMISPHERE_SOLID_COLOR" });
// 
// 		m_pCubemapResamplePSO = m_pDevice->CreateComputePipeline(m_pDevice->GetGlobalRootSignature(), "CubemapResampleShader", "cs_main");
// 	}
// 
// 	void SkyLightRenderer::Render(const RenderView& aRenderView) noexcept
// 	{
// 		Scene& scene = *aRenderView.pScene;
// 		const entity skyLightEntity = scene.GetActiveSkyLight();
// 		if (skyLightEntity == NULL_ENTITY)
// 			return;
// 
// 		SkyLightComponent& skyLightComponent = scene.GetEntityManager().Get<SkyLightComponent>(skyLightEntity);
// 		if (!ShouldProcessSkyLight(skyLightComponent, aRenderView.FrameIndex))
// 			return;
// 
// 		//Conditionally create resources:
// 		if (Ref<Texture>& pIrradianceMap = skyLightComponent.GetIrradianceMap(); !pIrradianceMap)
// 		{
// 			const String irradianceMapName = skyLightComponent.GetEnvironment()->GetEnvironmentMap()->GetName() + "_IrradianceMap";
// 			pIrradianceMap = m_pDevice->CreateTexture(TextureDesc::CreateCube(32u, 32u, ResourceFormat::RGBA32_FLOAT, 1u, TextureFlag::UnorderedAccess | TextureFlag::ShaderResource), irradianceMapName.c_str());
// 		}
// 		if (Ref<Texture>& pRadianceMap = skyLightComponent.GetRadianceMap(); !pRadianceMap)
// 		{
// 			const String name = skyLightComponent.GetEnvironment()->GetEnvironmentMap()->GetName() + "_RadianceMap";
// 			const uint32 radianceMapSize = skyLightComponent.GetRadianceMapSize();
// 			const uint32 numMips = static_cast<uint32>(std::floor(Math::Log2f(static_cast<float>(radianceMapSize)) + 1.0f));
// 			pRadianceMap = m_pDevice->CreateTexture(TextureDesc::CreateCube(radianceMapSize, radianceMapSize, ResourceFormat::RGBA32_FLOAT, numMips, TextureFlag::UnorderedAccess | TextureFlag::ShaderResource), name.c_str());
// 		}
// 
// 		if (skyLightComponent.IsIrradianceDirty())
// 		{
// 			CommandContext* pCommandContext = m_pDevice->AllocateCommandContext(D3D12_COMMAND_LIST_TYPE_COMPUTE);
// 			ExecuteIrradianceConvolutionPass(*pCommandContext, aRenderView, skyLightComponent);
// 			pCommandContext->Execute();
// 		}
// 		if (skyLightComponent.IsRadianceDirty())
// 		{
// 			CommandContext* pCommandContext = m_pDevice->AllocateCommandContext(D3D12_COMMAND_LIST_TYPE_COMPUTE);
// 			ExecuteRadiancePrefilterPass(*pCommandContext, aRenderView, skyLightComponent);
// 			pCommandContext->Execute();
// 		}
// 
// 		m_LastCapturedFrame = aRenderView.FrameIndex;
// 	}
// 
// 	void SkyLightRenderer::ExecuteIrradianceConvolutionPass(CommandContext& aCommandContext, const RenderView& aRenderView, SkyLightComponent& aSkyLightComponent) noexcept
// 	{
// 		Ref<Environment> pEnvironment = aSkyLightComponent.GetEnvironment();
// 		Ref<TextureCube> pBackingCubemap = pEnvironment->GetEnvironmentMap();
// 		const Ref<Texture>& pIrradianceMap = aSkyLightComponent.GetIrradianceMap();
// 		const uint32 size = pIrradianceMap->GetWidth();
// 		const Color& lowerHemisphereColor = aSkyLightComponent.GetLowerHemisphereColor();
// 
// 		struct
// 		{
// 			uint32 RadianceMapTextureIndex		= ShaderInterop::INVALID_DESCRIPTOR_INDEX;
// 			uint32 IrradianceMapTextureIndex	= ShaderInterop::INVALID_DESCRIPTOR_INDEX;
// 			uint32 Dimensions					= 0u;
// 			uint32 Samples						= 0u;
// 			Vector4 LowerHemisphereColor		= Vector4::Zero;
// 		} passData;
// 
// 		passData.RadianceMapTextureIndex = pBackingCubemap->GetResource()->GetSRVIndex();
// 		passData.IrradianceMapTextureIndex = pIrradianceMap->GetUAVIndex();
// 		passData.Dimensions = size;
// 		passData.Samples = 8;
// 		passData.LowerHemisphereColor = Vector4(lowerHemisphereColor.R(), lowerHemisphereColor.G(), lowerHemisphereColor.B(), lowerHemisphereColor.A());
// 
// 		aCommandContext.InsertResourceBarrier(pBackingCubemap->GetResource(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
// 		aCommandContext.InsertResourceBarrier(pIrradianceMap, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
// 
// 		aCommandContext.SetComputeRootSignature(m_pDevice->GetGlobalRootSignature());
// 
// 		if (aSkyLightComponent.GetLowerHemisphereMode() == ESkyLightLowerHemisphereMode::Environment)
// 			aCommandContext.SetPipelineState(m_pIrradiancePSO);
// 		else 
// 			aCommandContext.SetPipelineState(m_pIrradianceLowerHemisphereColorPSO);
// 
// 		aCommandContext.BindRootCBV(BindingSlot::PerInstance, (const void*)&passData, sizeof(passData));
// 		aCommandContext.Dispatch(ComputeUtils::GetNumThreadGroups(size, 32u, size, 32u, 6u, 1u));
// 
// 		aSkyLightComponent.NotifyIrradianceUpdated();
// 	}
// 
// 	void SkyLightRenderer::ExecuteRadiancePrefilterPass(CommandContext& aCommandContext, const RenderView& aRenderView, SkyLightComponent& aSkyLightComponent) noexcept
// 	{
// 		Ref<Environment> pEnvironment = aSkyLightComponent.GetEnvironment();
// 		const Ref<TextureCube> pBackingCubemap = pEnvironment->GetEnvironmentMap();
// 		const Ref<Texture>& pCubemapGPUResource = pBackingCubemap->GetResource();
// 		const Ref<Texture>& pRadianceMap = aSkyLightComponent.GetRadianceMap();
// 		const uint32 size = pRadianceMap->GetWidth();
// 		const uint32 numMips = pRadianceMap->GetMipLevels();
// 		const Color& lowerHemisphereColor = aSkyLightComponent.GetLowerHemisphereColor();
// 
// 		{
// 			aCommandContext.InsertResourceBarrier(pRadianceMap, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
// 			
// 			struct
// 			{
// 				uint32 SrcTextureIndex	= ShaderInterop::INVALID_DESCRIPTOR_INDEX;
// 				uint32 DstTextureIndex	= ShaderInterop::INVALID_DESCRIPTOR_INDEX;
// 				uint32 SrcSize			= 0u;
// 				uint32 DstSize			= 0u;
// 			} passData;
// 
// 			passData.SrcTextureIndex = pCubemapGPUResource->GetSRVIndex();
// 			passData.DstTextureIndex = pRadianceMap->GetUAVIndex(0);
// 			passData.SrcSize = pCubemapGPUResource->GetWidth();
// 			passData.DstSize = size;
// 
// 			aCommandContext.SetComputeRootSignature(m_pDevice->GetGlobalRootSignature());
// 			aCommandContext.SetPipelineState(m_pCubemapResamplePSO);
// 			aCommandContext.BindRootCBV(BindingSlot::PerInstance, (const void*)&passData, sizeof(passData));
// 			aCommandContext.Dispatch(ComputeUtils::GetNumThreadGroups(passData.DstSize, 8, passData.DstSize, 8, 6, 1));
// 		}
// 
// 		struct
// 		{
// 			uint32 InputTextureIndex		= ShaderInterop::INVALID_DESCRIPTOR_INDEX;
// 			uint32 OutputTextureIndex		= ShaderInterop::INVALID_DESCRIPTOR_INDEX;
// 			uint32 InputDimensions			= 0u;
// 			uint32 OutputDimensions			= 0u;
// 			Vector4 LowerHemisphereColor	= Vector4::Zero;
// 			float Roughness					= 0.0f;
// 			float Padding[3]				= {};
// 		} passData;
// 
// 		passData.InputTextureIndex = pCubemapGPUResource->GetSRVIndex();
// 		passData.InputDimensions = pCubemapGPUResource->GetWidth();
// 		passData.LowerHemisphereColor = Vector4(lowerHemisphereColor.R(), lowerHemisphereColor.G(), lowerHemisphereColor.B(), lowerHemisphereColor.A());
// 
// 		aCommandContext.InsertResourceBarrier(pCubemapGPUResource, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
// 
// 		for (uint32 mip = 1u; mip < numMips; ++mip)
// 		{
// 			passData.Roughness = static_cast<float>(mip) / static_cast<float>(numMips - 1u);
// 			passData.OutputDimensions = Math::Max(1u, size >> mip);
// 			passData.OutputTextureIndex = pRadianceMap->GetUAVIndex(mip);
// 
// 			aCommandContext.SetPipelineState(m_pRadiancePSO);
// 			aCommandContext.SetComputeRootSignature(m_pDevice->GetGlobalRootSignature());
// 
// 			aCommandContext.BindRootCBV(BindingSlot::PerInstance, (const void*)&passData, sizeof(passData));
// 			aCommandContext.Dispatch(ComputeUtils::GetNumThreadGroups(passData.OutputDimensions, 32u, passData.OutputDimensions, 32u, 6u, 1u));
// 		}
// 
// 		aSkyLightComponent.NotifyRadianceUpdated();
// 	}
// 
// 	bool SkyLightRenderer::ShouldProcessSkyLight(SkyLightComponent& aSkyLightComponent, uint32 aCurrentFrameIndex) noexcept
// 	{
// 		if (!aSkyLightComponent.HasAssignedEnvironment())
// 			return false;
// 
// 		//Special case: Regardless of capture options it needs to be at least initialized once.
// 		if (!aSkyLightComponent.GetIrradianceMap() || !aSkyLightComponent.GetRadianceMap())
// 			return true;
// 
// 		const uint32 captureInterval = aSkyLightComponent.GetCaptureInterval();
// 		const ESkyLightCaptureMode captureMode = aSkyLightComponent.GetCaptureMode();
// 
// 		if (captureMode == ESkyLightCaptureMode::Realtime && (captureInterval == 0u || aCurrentFrameIndex - m_LastCapturedFrame < captureInterval))
// 			return false;
// 
// 		if (captureMode != ESkyLightCaptureMode::Realtime && (!aSkyLightComponent.IsIrradianceDirty() && !aSkyLightComponent.IsRadianceDirty()))
// 			return false;
// 
// 		return true;
// 	}
// }