#include "LightRenderSubsystem.h"

#include "Graphics/RHI/CommandContext.h"
#include "Graphics/RHI/Device.h"
#include "Graphics/Renderer/Renderer.h"
#include "Graphics/Scene/RenderScene.h"

namespace Relentless
{
	uint32 LightRenderSubsystem::GetNumLights() const noexcept
	{
		return m_LightDataBuffer.Count;
	}

	const Buffer* LightRenderSubsystem::GetRenderData() const
	{
		RLS_ASSERT(m_LightDataBuffer.pBuffer, "[LightRenderSubsystem::GetRenderData]: Buffer is invalid.");
		return m_LightDataBuffer.pBuffer;
	}

	bool LightRenderSubsystem::OnLoad(ISystemManager* aSystemManager) noexcept
	{
		RenderScene* pRenderScene = static_cast<RenderScene*>(aSystemManager);

		Renderer* pRenderer = pRenderScene->GetRenderer();
		m_OnUploadCallbackID = pRenderer->RegisterOnUploadCallback(Callback<void(CommandContext&)>::Bind(this, &LightRenderSubsystem::OnUpload));

		m_pGraphicsDevice = pRenderer->GetDevice();

		return true;
	}

	void LightRenderSubsystem::OnUnload(ISystemManager* aSystemManager) noexcept
	{
		RenderScene* pRenderScene = static_cast<RenderScene*>(aSystemManager);

		Renderer* pRenderer = pRenderScene->GetRenderer();
		pRenderer->UnregisterOnUploadCallback(m_OnUploadCallbackID);
	}

	bool LightRenderSubsystem::ShouldCreateSubsystem(ISystemManager* aSystemManager) noexcept
	{
		return dynamic_cast<RenderScene*>(aSystemManager) != nullptr;
	}

	void LightRenderSubsystem::Patch(std::vector<LightRenderProxy> someRenderProxyUpdates) noexcept
	{
		for (auto& renderProxy : someRenderProxyUpdates)
			m_RenderData[renderProxy.ID] = renderProxy;
	}

	void LightRenderSubsystem::Remove(std::vector<uint32> someIDs) noexcept
	{
		for (auto& id : someIDs)
			m_RenderData.erase(id);
	}

	void LightRenderSubsystem::BuildLightData(ShaderInterop::Light& outLightData, const LightRenderProxy& aRenderProxy) const noexcept
	{
		outLightData.Color = aRenderProxy.Color;
		outLightData.Position = aRenderProxy.Position;
		outLightData.Direction = aRenderProxy.Direction;
		outLightData.Intensity = aRenderProxy.LightType == ELightType::Directional ? (aRenderProxy.Intensity / Math::PI) : aRenderProxy.Intensity;
		outLightData.IsDirectional = aRenderProxy.LightType == ELightType::Directional;
		outLightData.IsPoint = aRenderProxy.LightType == ELightType::Point;
		outLightData.IsSpot = aRenderProxy.LightType == ELightType::Spot;
		outLightData.Range = aRenderProxy.AttenuationRadius;
		outLightData.SpotlightAngles = Vector2(aRenderProxy.InnerConeAngle, aRenderProxy.OuterConeAngle);
		outLightData.IsEnabled = outLightData.Intensity > 0.0f;
	}

	void LightRenderSubsystem::OnUpload(CommandContext& aCommandContext) noexcept
	{
		m_LightCache.clear();
		m_LightCache.reserve(m_RenderData.size());

		for (const auto& [_, renderProxy] : m_RenderData)
		{
			if (!renderProxy.IsEnabled)
				continue;

			ShaderInterop::Light& lightData = m_LightCache.emplace_back();
			BuildLightData(lightData, renderProxy);
		}

		const uint32 numElements = static_cast<uint32>(m_LightCache.size());
		const uint32 desiredElements = Math::AlignUp(Math::Max(1u, numElements), 8u);
		const uint32 stride = sizeof(ShaderInterop::Light);

		if (!m_LightDataBuffer.pBuffer || desiredElements > m_LightDataBuffer.pBuffer->GetNrOfElements())
			m_LightDataBuffer.pBuffer = m_pGraphicsDevice->CreateBuffer(BufferDesc::CreateStructured(desiredElements, stride), "LightData");

		const uint64 totalSize = numElements * stride;
		ScratchAllocation alloc = aCommandContext.AllocateScratch(totalSize);
		memcpy(alloc.pMappedMemory, m_LightCache.data(), totalSize);
		aCommandContext.CopyBuffer(alloc.pBackingResource, m_LightDataBuffer.pBuffer, alloc.Size, alloc.Offset, 0);
		m_LightDataBuffer.Count = numElements;
	}
}