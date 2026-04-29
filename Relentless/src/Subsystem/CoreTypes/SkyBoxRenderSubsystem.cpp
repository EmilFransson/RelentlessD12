#include "SkyBoxRenderSubsystem.h"

#include "Graphics/Renderer/Renderer.h"
#include "Graphics/RHI/CommandContext.h"
#include "Graphics/RHI/Device.h"
#include "Graphics/Scene/RenderScene.h"

namespace Relentless
{
	const Buffer* SkyBoxRenderSubsystem::GetRenderData() const
	{
		RLS_ASSERT(m_pSkyBoxDataBuffer, "[SkyBoxRenderSubsystem::GetRenderData]: Buffer is invalid.");
		return m_pSkyBoxDataBuffer;
	}

	bool SkyBoxRenderSubsystem::OnLoad(ISystemManager* aSystemManager) noexcept
	{
		RenderScene* pRenderScene = static_cast<RenderScene*>(aSystemManager);

		Renderer* pRenderer = pRenderScene->GetRenderer();
		m_OnFrameBeginCallbackID = pRenderer->RegisterOnFrameRenderBeginCallback(Callback<void()>::Bind(this, &SkyBoxRenderSubsystem::OnRenderFrameBegin));
		m_OnUploadCallbackID = pRenderer->RegisterOnUploadCallback(Callback<void(CommandContext&)>::Bind(this, &SkyBoxRenderSubsystem::OnUpload));

		m_pGraphicsDevice = pRenderer->GetDevice();

		return true;
	}

	void SkyBoxRenderSubsystem::OnUnload(ISystemManager* aSystemManager) noexcept
	{
		RenderScene* pRenderScene = static_cast<RenderScene*>(aSystemManager);

		Renderer* pRenderer = pRenderScene->GetRenderer();
		pRenderer->UnregisterOnFrameRenderBeginCallback(m_OnFrameBeginCallbackID);
		pRenderer->UnregisterOnUploadCallback(m_OnUploadCallbackID);

		m_OnFrameBeginCallbackID = INVALID_CALLBACK_ID;
		m_OnUploadCallbackID = INVALID_CALLBACK_ID;
	}

	bool SkyBoxRenderSubsystem::ShouldCreateSubsystem(ISystemManager* aSystemManager) noexcept
	{
		return dynamic_cast<RenderScene*>(aSystemManager) != nullptr;
	}

	void SkyBoxRenderSubsystem::Patch(std::vector<SkyBoxRenderProxy> someRenderProxyUpdates) noexcept
	{
		m_PendingRenderProxyUpdates.reserve(m_PendingRenderProxyUpdates.size() + someRenderProxyUpdates.size());

		for (auto& proxy : someRenderProxyUpdates)
			m_PendingRenderProxyUpdates.emplace_back(std::move(proxy), ESkyBoxRenderProxyUpdateType::Patch);
	}

	void SkyBoxRenderSubsystem::Remove(std::vector<uint32> someIDs) noexcept
	{
		m_PendingRenderProxyUpdates.reserve(m_PendingRenderProxyUpdates.size() + someIDs.size());

		for (uint32 id : someIDs)
			m_PendingRenderProxyUpdates.push_back(PendingSkyBoxRenderProxyUpdate::MakeDeletion(id));
	}

	bool SkyBoxRenderSubsystem::ShouldBlendEnvironments() const noexcept
	{
		if (m_ActiveID == INVALID_ID)
			return false;

		const SkyBoxRenderProxy& activeProxy = m_RenderData.at(m_ActiveID);
		return activeProxy.EnvironmentMapA && activeProxy.EnvironmentMapB && activeProxy.BlendFactor > 0.0f && activeProxy.BlendFactor < 1.0f;
	}

	void SkyBoxRenderSubsystem::ApplyPatch(SkyBoxRenderProxy aRenderProxy) noexcept
	{
		UpdateActiveID(aRenderProxy);
		m_RenderData[aRenderProxy.ID] = std::move(aRenderProxy);
	}

	void SkyBoxRenderSubsystem::BuildSkyBoxData(ShaderInterop::SkyboxData& outData) const noexcept
	{
		Texture* pBlackCube = GraphicsCommon::GetDefaultTexture(DefaultTextureType::BlackCube);

		if (m_ActiveID == INVALID_ID)
		{
			outData.EnvironmentMapAIndex = pBlackCube->GetSRVIndex();
			outData.EnvironmentMapBIndex = pBlackCube->GetSRVIndex();
			outData.BackgroundColor = Vector3::Zero;
			outData.Intensity = 1.0f;
			outData.LODBias = 0.0f;
			outData.BackgroundColor = Vector3::One;
			outData.WorldRotation = Matrix::Identity;
			outData.BlendFactor = 0.0f;
			return;
		}

		const SkyBoxRenderProxy& renderProxy = m_RenderData.at(m_ActiveID);

		outData.EnvironmentMapAIndex = pBlackCube->GetSRVIndex();
		outData.EnvironmentMapBIndex = pBlackCube->GetSRVIndex();

		if (renderProxy.BlendFactor == 0.0f)
			outData.EnvironmentMapAIndex = renderProxy.EnvironmentMapA ? renderProxy.EnvironmentMapA->GetSRVIndex() : renderProxy.EnvironmentMapB ? renderProxy.EnvironmentMapB->GetSRVIndex() : pBlackCube->GetSRVIndex();
		else if (renderProxy.BlendFactor == 1.0f)
			outData.EnvironmentMapAIndex = renderProxy.EnvironmentMapB ? renderProxy.EnvironmentMapB->GetSRVIndex() : renderProxy.EnvironmentMapA ? renderProxy.EnvironmentMapA->GetSRVIndex() : pBlackCube->GetSRVIndex();
		else if (renderProxy.BlendFactor > 0.0f && renderProxy.BlendFactor < 1.0f)
		{
			outData.EnvironmentMapAIndex = renderProxy.EnvironmentMapA ? renderProxy.EnvironmentMapA->GetSRVIndex() : renderProxy.EnvironmentMapB ? renderProxy.EnvironmentMapB->GetSRVIndex() : pBlackCube->GetSRVIndex();
			outData.EnvironmentMapBIndex = renderProxy.EnvironmentMapB ? renderProxy.EnvironmentMapB->GetSRVIndex() : pBlackCube->GetSRVIndex();
		}

		outData.BackgroundColor = renderProxy.TintColor.ToVector3();
		outData.Intensity = renderProxy.Intensity;
		outData.LODBias = renderProxy.LodBias;
		outData.WorldRotation = Matrix::CreateFromQuaternion(renderProxy.WorldRotation);
		outData.BlendFactor = renderProxy.BlendFactor;
	}

	void SkyBoxRenderSubsystem::FlushPendingProxyUpdates() noexcept
	{
		for (PendingSkyBoxRenderProxyUpdate& renderProxyUpdate : m_PendingRenderProxyUpdates)
		{
			if (renderProxyUpdate.UpdateType == ESkyBoxRenderProxyUpdateType::Delete)
			{
				RemoveByID(renderProxyUpdate.RenderProxy.ID);
				continue;
			}

			ApplyPatch(std::move(renderProxyUpdate.RenderProxy));
		}

		m_PendingRenderProxyUpdates.clear();
	}

	void SkyBoxRenderSubsystem::OnRenderFrameBegin()
	{
		FlushPendingProxyUpdates();
	}

	void SkyBoxRenderSubsystem::OnUpload(CommandContext& aCommandContext) noexcept
	{
		ScratchAllocation alloc = aCommandContext.AllocateScratch(sizeof(ShaderInterop::SkyboxData));
		ShaderInterop::SkyboxData& parameters = alloc.As<ShaderInterop::SkyboxData>();
		BuildSkyBoxData(parameters);

		if (!m_pSkyBoxDataBuffer)
			m_pSkyBoxDataBuffer = m_pGraphicsDevice->CreateBuffer(BufferDesc::CreateStructured(1u, sizeof(ShaderInterop::SkyboxData)), "SkyBoxData");

		aCommandContext.CopyBuffer(alloc.pBackingResource, m_pSkyBoxDataBuffer, alloc.Size, alloc.Offset, 0);
	}

	void SkyBoxRenderSubsystem::RemoveByID(uint32 aID) noexcept
	{
		m_RenderData.erase(aID);

		if (m_ActiveID == aID)
			m_ActiveID = INVALID_ID;
	}

	void SkyBoxRenderSubsystem::UpdateActiveID(const SkyBoxRenderProxy& aRenderProxy) noexcept
	{
		if (aRenderProxy.IsActive)
			m_ActiveID = aRenderProxy.ID;
		else if (m_ActiveID == aRenderProxy.ID)
			m_ActiveID = INVALID_ID;
	}
}