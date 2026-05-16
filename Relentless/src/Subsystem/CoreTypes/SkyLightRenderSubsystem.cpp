#include "SkyLightRenderSubsystem.h"

#include "Assets/CoreTypes/Environment.h"

#include "Callback/Callback.h"

#include "Graphics/RHI/CommandContext.h"
#include "Graphics/RHI/Device.h"
#include "Graphics/Renderer/Renderer.h"
#include "Graphics/Renderer/RenderTypes.h"
#include "Graphics/Renderer/Service/IBLGenerationService.h"
#include "Graphics/Scene/RenderScene.h"

#include "Module/ModuleManager.h"
#include "Module/RenderModule.h"

#include "Subsystem/ISystemManager.h"

namespace Relentless
{
	const Buffer* SkyLightRenderSubsystem::GetRenderData() const
	{
		RLS_ASSERT(m_pSkyLightDataBuffer, "[SkyLightRenderSubsystem::GetRenderData]: Buffer is invalid.");
		return m_pSkyLightDataBuffer;
	}

	bool SkyLightRenderSubsystem::OnLoad(ISystemManager* aSystemManager) noexcept
	{
		RenderScene* pRenderScene = static_cast<RenderScene*>(aSystemManager);

		Renderer* pRenderer = pRenderScene->GetRenderer();
		m_OnFrameBeginCallbackID = pRenderer->RegisterOnFrameRenderBeginCallback(Callback<void()>::Bind(this, &SkyLightRenderSubsystem::OnRenderFrameBegin));
		m_OnUploadCallbackID = pRenderer->RegisterOnUploadCallback(Callback<void(CommandContext&)>::Bind(this, &SkyLightRenderSubsystem::OnUpload));

		m_pGraphicsDevice = pRenderer->GetDevice();

		RenderModule& renderModule = ModuleManager::LoadModuleChecked<RenderModule>();
		m_pIBLGenerationService = renderModule.GetIBLGenerationService().get();

		return true;
	}

	void SkyLightRenderSubsystem::OnUnload(ISystemManager* aSystemManager) noexcept
	{
		RenderScene* pRenderScene = static_cast<RenderScene*>(aSystemManager);

		Renderer* pRenderer = pRenderScene->GetRenderer();
		pRenderer->UnregisterOnFrameRenderBeginCallback(m_OnFrameBeginCallbackID);
		pRenderer->UnregisterOnUploadCallback(m_OnUploadCallbackID);

		m_OnFrameBeginCallbackID = INVALID_CALLBACK_ID;
		m_OnUploadCallbackID = INVALID_CALLBACK_ID;
	}

	bool SkyLightRenderSubsystem::ShouldCreateSubsystem(ISystemManager* aSystemManager) noexcept
	{
		return dynamic_cast<RenderScene*>(aSystemManager) != nullptr;
	}

	void SkyLightRenderSubsystem::Patch(std::vector<SkyLightRenderProxy> someRenderProxyUpdates) noexcept
	{
		m_PendingRenderProxyUpdates.reserve(m_PendingRenderProxyUpdates.size() + someRenderProxyUpdates.size());

		for (auto& proxy : someRenderProxyUpdates)
			m_PendingRenderProxyUpdates.emplace_back(std::move(proxy), ESkyLightRenderProxyUpdateType::Patch);
	}

	void SkyLightRenderSubsystem::Remove(std::vector<uint32> someIDs) noexcept
	{
		m_PendingRenderProxyUpdates.reserve(m_PendingRenderProxyUpdates.size() + someIDs.size());

		for (uint32 id : someIDs)
			m_PendingRenderProxyUpdates.push_back(PendingSkyLightRenderProxyUpdate::MakeDeletion(id));
	}

	void SkyLightRenderSubsystem::SetBRDFLutTexture(Ref<Texture> aBRDFLUT) noexcept
	{
		m_pBRDFLut = aBRDFLUT;
	}

	void SkyLightRenderSubsystem::ApplyPatch(SkyLightRenderProxy aRenderProxy) noexcept
	{
		SkyLightRenderData& renderData = m_RenderData[aRenderProxy.ID];

		renderData.PrimaryEntry.IrradianceDirty = aRenderProxy.PrimaryEnvironmentMap  && (aRenderProxy.PrimaryEnvironmentMap != renderData.RenderProxy.PrimaryEnvironmentMap);
		renderData.PrimaryEntry.RadianceDirty = aRenderProxy.PrimaryEnvironmentMap  && (aRenderProxy.PrimaryEnvironmentMap != renderData.RenderProxy.PrimaryEnvironmentMap);

		renderData.SecondaryEntry.IrradianceDirty = aRenderProxy.BlendEnvironmentMap && (aRenderProxy.BlendEnvironmentMap != renderData.RenderProxy.BlendEnvironmentMap);
		renderData.SecondaryEntry.RadianceDirty = aRenderProxy.BlendEnvironmentMap && (aRenderProxy.BlendEnvironmentMap != renderData.RenderProxy.BlendEnvironmentMap);

		UpdateActiveID(aRenderProxy);
		EnsureIBLMapsAllocated(aRenderProxy, renderData);

		const bool captureModeSwitched = renderData.RenderProxy.CaptureMode != aRenderProxy.CaptureMode;
		if (renderData.PrimaryEntry.IrradianceDirty || renderData.PrimaryEntry.RadianceDirty || captureModeSwitched)
			renderData.PrimaryEntry.TargetMip = 0u;

		if (renderData.SecondaryEntry.IrradianceDirty || renderData.SecondaryEntry.RadianceDirty || captureModeSwitched)
			renderData.SecondaryEntry.TargetMip = 0u;

		renderData.RenderProxy = std::move(aRenderProxy);
		renderData.RenderProxy.LowerHemisphereColor.w = (renderData.RenderProxy.LowerHemisphereMode == ESkyLightLowerHemisphereMode::SolidColor) ? 1.0f : 0.0f;
	}

	void SkyLightRenderSubsystem::EnsureIBLMapsAllocated(const SkyLightRenderProxy& aRenderProxy, SkyLightRenderData& outRenderData) noexcept
	{
		if (aRenderProxy.PrimaryEnvironmentSourceType == EEnvironmentSourceType::Cubemap && aRenderProxy.PrimaryEnvironmentMap)
		{
			if (!outRenderData.PrimaryEntry.IrradianceMap)
			{
				const String name = "SkyLight_Primary_IrradianceMap_" + std::to_string(aRenderProxy.ID);
				outRenderData.PrimaryEntry.IrradianceMap = m_pGraphicsDevice->CreateTexture(TextureDesc::CreateCube(32u, 32u, ResourceFormat::RGBA16_FLOAT, 1u, TextureFlag::UnorderedAccess | TextureFlag::ShaderResource), name.c_str());
				outRenderData.PrimaryEntry.IrradianceDirty = true;
			}

			const bool radianceSizeChanged = outRenderData.PrimaryEntry.RadianceMap && outRenderData.PrimaryEntry.RadianceMap->GetWidth() != aRenderProxy.RadianceMapSize;
			if (!outRenderData.PrimaryEntry.RadianceMap || radianceSizeChanged)
			{
				const String name = "SkyLight_Primary_RadianceMap_" + std::to_string(aRenderProxy.ID);
				const uint32 size = aRenderProxy.RadianceMapSize;
				const uint32 numMips = static_cast<uint32>(std::floor(Math::Log2f(static_cast<float>(size)) + 1.0f));
				outRenderData.PrimaryEntry.RadianceMap = m_pGraphicsDevice->CreateTexture(TextureDesc::CreateCube(size, size, ResourceFormat::RGBA16_FLOAT, numMips, TextureFlag::UnorderedAccess | TextureFlag::ShaderResource), name.c_str());
				outRenderData.PrimaryEntry.RadianceDirty = true;
			}
		}

		if (aRenderProxy.BlendEnvironmentSourceType == EEnvironmentSourceType::Cubemap && aRenderProxy.BlendEnvironmentMap)
		{
			if (!outRenderData.SecondaryEntry.IrradianceMap)
			{
				const String name = "SkyLight_Secondary_IrradianceMap_" + std::to_string(aRenderProxy.ID);
				outRenderData.SecondaryEntry.IrradianceMap = m_pGraphicsDevice->CreateTexture(TextureDesc::CreateCube(32u, 32u, ResourceFormat::RGBA16_FLOAT, 1u, TextureFlag::UnorderedAccess | TextureFlag::ShaderResource), name.c_str());
				outRenderData.SecondaryEntry.IrradianceDirty = true;
			}

			const bool radianceSizeChanged = outRenderData.SecondaryEntry.RadianceMap && outRenderData.SecondaryEntry.RadianceMap->GetWidth() != aRenderProxy.RadianceMapSize;
			if (!outRenderData.SecondaryEntry.RadianceMap || radianceSizeChanged)
			{
				const String name = "SkyLight_Secondary_RadianceMap_" + std::to_string(aRenderProxy.ID);
				const uint32 size = aRenderProxy.RadianceMapSize;
				const uint32 numMips = static_cast<uint32>(std::floor(Math::Log2f(static_cast<float>(size)) + 1.0f));
				outRenderData.SecondaryEntry.RadianceMap = m_pGraphicsDevice->CreateTexture(TextureDesc::CreateCube(size, size, ResourceFormat::RGBA16_FLOAT, numMips, TextureFlag::UnorderedAccess | TextureFlag::ShaderResource), name.c_str());
				outRenderData.SecondaryEntry.RadianceDirty = true;
			}
		}
	}

	void SkyLightRenderSubsystem::FlushPendingProxyUpdates() noexcept
	{
		for (PendingSkyLightRenderProxyUpdate& renderProxyUpdate : m_PendingRenderProxyUpdates)
		{
			if (renderProxyUpdate.UpdateType == ESkyLightRenderProxyUpdateType::Delete)
			{
				RemoveByID(renderProxyUpdate.RenderProxy.ID);
				continue;
			}

			ApplyPatch(std::move(renderProxyUpdate.RenderProxy));
		}

		m_PendingRenderProxyUpdates.clear();
	}

	void SkyLightRenderSubsystem::BuildSkyLightData(ShaderInterop::SkyLightData& outSkyLightData) const noexcept
	{
		outSkyLightData.BRDFLutTextureIndex = m_pBRDFLut->GetSRVIndex();
		
		if (m_ActiveSkyLightID == INVALID_SKYLIGHT_ID)
		{
			const uint32 blackCubeSRV = GraphicsCommon::GetDefaultTexture(DefaultTextureType::BlackCube)->GetSRVIndex();
			outSkyLightData.IrradianceMapIndex = blackCubeSRV;
			outSkyLightData.RadianceMapIndex = blackCubeSRV;
			outSkyLightData.Intensity = 1.0f;
			outSkyLightData.Tint = Vector3::One;
			outSkyLightData.WorldRotation = Matrix::Identity;
			outSkyLightData.LowerHemisphereColor = Vector4::Zero;
			return;
		}

		const SkyLightRenderData& renderData = m_RenderData.at(m_ActiveSkyLightID);
		const SkyLightRenderProxy& proxy = renderData.RenderProxy;
		const bool isCubemap = proxy.PrimaryEnvironmentSourceType == EEnvironmentSourceType::Cubemap;
		const uint32 fallbackSRV = GraphicsCommon::GetDefaultTexture(DefaultTextureType::WhiteCube)->GetSRVIndex();

		outSkyLightData.IrradianceMapIndex = isCubemap && renderData.PrimaryEntry.IrradianceMap ? renderData.PrimaryEntry.IrradianceMap->GetSRVIndex() : fallbackSRV;
		outSkyLightData.RadianceMapIndex = isCubemap && renderData.PrimaryEntry.RadianceMap ? renderData.PrimaryEntry.RadianceMap->GetSRVIndex() : fallbackSRV;
		outSkyLightData.BlendIrradianceMapIndex = isCubemap && renderData.SecondaryEntry.IrradianceMap ? renderData.SecondaryEntry.IrradianceMap->GetSRVIndex() : fallbackSRV;
		outSkyLightData.BlendRadianceMapIndex = isCubemap && renderData.SecondaryEntry.RadianceMap ? renderData.SecondaryEntry.RadianceMap->GetSRVIndex() : fallbackSRV;
		outSkyLightData.Intensity = proxy.Intensity;
		outSkyLightData.BlendFactor = proxy.BlendFactor;
		outSkyLightData.Tint = Vector3(proxy.TintColor.R(), proxy.TintColor.G(), proxy.TintColor.B());
		outSkyLightData.WorldRotation = Matrix::CreateFromQuaternion(proxy.WorldRotation);
		outSkyLightData.LowerHemisphereColor = proxy.LowerHemisphereColor.ToVector4();
	}

	void SkyLightRenderSubsystem::DispatchIBLRequests(SkyLightRenderData& aRenderData) noexcept
	{
		const bool isRealtime = aRenderData.RenderProxy.CaptureMode == ESkyLightCaptureMode::Realtime;

		if (aRenderData.PrimaryEntry.IrradianceMap && aRenderData.RenderProxy.PrimaryEnvironmentMap && (aRenderData.PrimaryEntry.IrradianceDirty || isRealtime))
		{
			m_pIBLGenerationService->RequestIrradiance({
				.SourceEnvironmentCubemap = aRenderData.RenderProxy.PrimaryEnvironmentMap,
				.TargetIrradianceMap = aRenderData.PrimaryEntry.IrradianceMap,
				.NumSamples = 8u
				});
			aRenderData.PrimaryEntry.IrradianceDirty = false;
		}
		if (aRenderData.SecondaryEntry.IrradianceMap && aRenderData.RenderProxy.BlendEnvironmentMap && (aRenderData.SecondaryEntry.IrradianceDirty || isRealtime))
		{
			m_pIBLGenerationService->RequestIrradiance({
				.SourceEnvironmentCubemap = aRenderData.RenderProxy.BlendEnvironmentMap,
				.TargetIrradianceMap = aRenderData.SecondaryEntry.IrradianceMap,
				.NumSamples = 8u
				});
			aRenderData.SecondaryEntry.IrradianceDirty = false;
		}

		if (aRenderData.PrimaryEntry.RadianceMap && aRenderData.RenderProxy.PrimaryEnvironmentMap && (aRenderData.PrimaryEntry.RadianceDirty || isRealtime))
		{
			const uint32 totalMips = aRenderData.PrimaryEntry.RadianceMap->GetMipLevels();
			const uint32 firstMip = isRealtime ? aRenderData.PrimaryEntry.TargetMip : 0u;
			const uint32 mipsPerFrame = isRealtime ? Math::Min(aRenderData.RenderProxy.RealtimeMipsPerFrame, totalMips) : totalMips;
			const uint32 clampedCount = isRealtime ? Math::Min(mipsPerFrame, totalMips - firstMip) : totalMips;

			m_pIBLGenerationService->RequestRadiance({
				.SourceEnvironmentCubemap = aRenderData.RenderProxy.PrimaryEnvironmentMap,
				.TargetRadianceMap = aRenderData.PrimaryEntry.RadianceMap,
				.Mips = {.FirstMip = firstMip, .MipCount = clampedCount }
				});

			aRenderData.PrimaryEntry.RadianceDirty = false;
			aRenderData.PrimaryEntry.TargetMip = (firstMip + clampedCount) % totalMips;
		}
		if (aRenderData.SecondaryEntry.RadianceMap && aRenderData.RenderProxy.BlendEnvironmentMap && (aRenderData.SecondaryEntry.RadianceDirty || isRealtime))
		{
			const uint32 totalMips = aRenderData.SecondaryEntry.RadianceMap->GetMipLevels();
			const uint32 firstMip = isRealtime ? aRenderData.SecondaryEntry.TargetMip : 0u;
			const uint32 mipsPerFrame = isRealtime ? Math::Min(aRenderData.RenderProxy.RealtimeMipsPerFrame, totalMips) : totalMips;
			const uint32 clampedCount = isRealtime ? Math::Min(mipsPerFrame, totalMips - firstMip) : totalMips;

			m_pIBLGenerationService->RequestRadiance({
				.SourceEnvironmentCubemap = aRenderData.RenderProxy.BlendEnvironmentMap,
				.TargetRadianceMap = aRenderData.SecondaryEntry.RadianceMap,
				.Mips = {.FirstMip = firstMip, .MipCount = clampedCount }
				});

			aRenderData.SecondaryEntry.RadianceDirty = false;
			aRenderData.SecondaryEntry.TargetMip = (firstMip + clampedCount) % totalMips;
		}
	}

	void SkyLightRenderSubsystem::OnRenderFrameBegin()
	{
		FlushPendingProxyUpdates();

		if (m_ActiveSkyLightID != INVALID_SKYLIGHT_ID)
			DispatchIBLRequests(m_RenderData.at(m_ActiveSkyLightID));
	}

	void SkyLightRenderSubsystem::OnUpload(CommandContext& aCommandContext) noexcept
	{
		ScratchAllocation alloc = aCommandContext.AllocateScratch(sizeof(ShaderInterop::SkyLightData));
		ShaderInterop::SkyLightData& parameters = alloc.As<ShaderInterop::SkyLightData>();
		BuildSkyLightData(parameters);

		if (!m_pSkyLightDataBuffer)
			m_pSkyLightDataBuffer = m_pGraphicsDevice->CreateBuffer(BufferDesc::CreateStructured(1u, sizeof(ShaderInterop::SkyLightData)), "SkyLightData");

		aCommandContext.CopyBuffer(alloc.pBackingResource, m_pSkyLightDataBuffer, alloc.Size, alloc.Offset, 0);
	}

	void SkyLightRenderSubsystem::RemoveByID(uint32 aID) noexcept
	{
		m_RenderData.erase(aID);

		if (m_ActiveSkyLightID == aID)
			m_ActiveSkyLightID = INVALID_SKYLIGHT_ID;
	}

	void SkyLightRenderSubsystem::UpdateActiveID(const SkyLightRenderProxy& aRenderProxy) noexcept
	{
		if (aRenderProxy.IsActive)
			m_ActiveSkyLightID = aRenderProxy.ID;
		else if (m_ActiveSkyLightID == aRenderProxy.ID)
			m_ActiveSkyLightID = INVALID_SKYLIGHT_ID;
	}

}
