#include "FogRenderSubsystem.h"

#include "Graphics/RHI/CommandContext.h"
#include "Graphics/RHI/Device.h"
#include "Graphics/Renderer/Renderer.h"
#include "Graphics/Scene/RenderScene.h"

namespace Relentless
{
	const Buffer* FogRenderSubsystem::GetRenderData() const
	{
		RLS_ASSERT(m_pFogDataBuffer, "[FogRenderSubsystem::GetRenderData]: Buffer is invalid.");
		return m_pFogDataBuffer;
	}

	bool FogRenderSubsystem::OnLoad(ISystemManager* aSystemManager) noexcept
	{
		RenderScene* pRenderScene = static_cast<RenderScene*>(aSystemManager);

		Renderer* pRenderer = pRenderScene->GetRenderer();
		m_OnUploadCallbackID = pRenderer->RegisterOnUploadCallback(Callback<void(CommandContext&)>::Bind(this, &FogRenderSubsystem::OnUpload));

		m_pGraphicsDevice = pRenderer->GetDevice();

		return true;
	}

	void FogRenderSubsystem::OnUnload(ISystemManager* aSystemManager) noexcept
	{
		RenderScene* pRenderScene = static_cast<RenderScene*>(aSystemManager);

		Renderer* pRenderer = pRenderScene->GetRenderer();
		pRenderer->UnregisterOnUploadCallback(m_OnUploadCallbackID);

		m_OnUploadCallbackID = INVALID_CALLBACK_ID;
	}

	bool FogRenderSubsystem::ShouldCreateSubsystem(ISystemManager* aSystemManager) noexcept
	{
		return dynamic_cast<RenderScene*>(aSystemManager) != nullptr;
	}

	void FogRenderSubsystem::Patch(std::vector<ExponentialHeightFogRenderProxy> someRenderProxyUpdates) noexcept
	{
		std::ranges::for_each(someRenderProxyUpdates, [this](const ExponentialHeightFogRenderProxy& aRenderProxy)
			{
				m_RenderData[aRenderProxy.ID] = aRenderProxy;
				if (aRenderProxy.IsActive)
					m_ActiveID = aRenderProxy.ID;
				else if (aRenderProxy.ID == m_ActiveID)
					m_ActiveID = INVALID_ID;
			});
	}

	void FogRenderSubsystem::Remove(std::vector<uint32> someIDs) noexcept
	{
		std::ranges::for_each(someIDs, [this](uint32 aID)
			{  
				m_RenderData.erase(aID);
				if (m_ActiveID == aID)
					m_ActiveID = INVALID_ID;
			});
	}

	void FogRenderSubsystem::BuildFogData(ShaderInterop::FogData& outFogData) const noexcept
	{
		if (m_ActiveID == INVALID_ID)
		{
			outFogData.InScatteringColor = Vector3::Zero;
			outFogData.InScatteringTextureColorTint = Vector3::One;
			outFogData.DensityLayer0 = 0.0f;
			outFogData.DensityLayer1 = 0.0f;
			outFogData.InScatteringTextureIndex = INVALID_ID;
			return;
		}

		const ExponentialHeightFogRenderProxy& renderProxy = m_RenderData.at(m_ActiveID);
		outFogData.InScatteringColor = renderProxy.InScatteringColor;
		outFogData.MaxOpacity = renderProxy.MaxOpacity;
		outFogData.InScatteringTextureColorTint = renderProxy.InScatteringTextureColorTint;
		outFogData.InScatteringTextureIndex = renderProxy.InScatterTexture ? renderProxy.InScatterTexture->GetSRVIndex() : INVALID_ID;
		outFogData.DensityLayer0 = renderProxy.DensityLayer0;
		outFogData.DensityLayer1 = renderProxy.DensityLayer1;
		outFogData.HeightFallOffLayer0 = renderProxy.HeightFallOffLayer0;
		outFogData.HeightFallOffLayer1 = renderProxy.HeightFallOffLayer1;
		
		outFogData.StartDistanceLayer0 = renderProxy.StartDistanceLayer0;
		outFogData.StartDistanceLayer1 = renderProxy.StartDistanceLayer1;

		outFogData.EndDistanceLayer0 = renderProxy.EndDistanceLayer0;
		outFogData.EndDistanceLayer1 = renderProxy.EndDistanceLayer1;
		outFogData.HeightOffsetLayer0 = renderProxy.HeightOffsetLayer0;
		outFogData.HeightOffsetLayer1 = renderProxy.HeightOffsetLayer1;
	}

	void FogRenderSubsystem::OnUpload(CommandContext& aCommandContext) noexcept
	{
		ScratchAllocation alloc = aCommandContext.AllocateScratch(sizeof(ShaderInterop::FogData));
		ShaderInterop::FogData& parameters = alloc.As<ShaderInterop::FogData>();
		BuildFogData(parameters);

		if (!m_pFogDataBuffer)
			m_pFogDataBuffer = m_pGraphicsDevice->CreateBuffer(BufferDesc::CreateStructured(1u, sizeof(ShaderInterop::FogData)), "FogData");

		aCommandContext.CopyBuffer(alloc.pBackingResource, m_pFogDataBuffer, alloc.Size, alloc.Offset, 0);
	}
}
