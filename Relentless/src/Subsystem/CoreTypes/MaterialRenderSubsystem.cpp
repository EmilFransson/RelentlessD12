#include "MaterialRenderSubsystem.h"

#include "Graphics/Renderer/Renderer.h"
#include "Graphics/RHI/CommandContext.h"
#include "Graphics/RHI/Device.h"
#include "Graphics/Scene/RenderScene.h"

namespace Relentless
{
	uint32 MaterialRenderSubsystem::GetNumMaterials() const noexcept
	{
		return m_MaterialDataBuffer.Count;
	}

	const Buffer* MaterialRenderSubsystem::GetRenderData() const noexcept
	{
		RLS_ASSERT(m_MaterialDataBuffer.pBuffer, "[MaterialRenderSubsystem::GetRenderData]: Buffer is invalid.");
		return m_MaterialDataBuffer.pBuffer;
	}

	uint32 MaterialRenderSubsystem::GetSlotIndex(const UUID& aUUID) const noexcept
	{
		RLS_ASSERT(m_IDToSlotMap.contains(aUUID), "[MaterialRenderSubsystem::GetSlotIndex]: Slot Index does not exist for UUID");
		return m_IDToSlotMap.at(aUUID);
	}

	const MaterialRenderProxy& MaterialRenderSubsystem::GetProxy(const UUID& aUUID) const noexcept
	{
		RLS_ASSERT(m_RenderData.contains(aUUID), "[MaterialRenderSubsystem::GetProxy]: Proxy does not exist for UUID.");
		return m_RenderData.at(aUUID);
	}

	bool MaterialRenderSubsystem::OnLoad(ISystemManager* aSystemManager) noexcept
	{
		RenderScene* pRenderScene = static_cast<RenderScene*>(aSystemManager);

		Renderer* pRenderer = pRenderScene->GetRenderer();
		m_OnUploadCallbackID = pRenderer->RegisterOnUploadCallback(Callback<void(CommandContext&)>::Bind(this, &MaterialRenderSubsystem::OnUpload));
		m_pGraphicsDevice = pRenderer->GetDevice();

		return true;
	}

	void MaterialRenderSubsystem::OnUnload(ISystemManager* aSystemManager) noexcept
	{
		RenderScene* pRenderScene = static_cast<RenderScene*>(aSystemManager);
		Renderer* pRenderer = pRenderScene->GetRenderer();
		pRenderer->UnregisterOnUploadCallback(m_OnUploadCallbackID);
	}

	bool MaterialRenderSubsystem::ShouldCreateSubsystem(ISystemManager* aSystemManager) noexcept
	{
		return dynamic_cast<RenderScene*>(aSystemManager) != nullptr;
	}

	void MaterialRenderSubsystem::Patch(std::vector<MaterialRenderProxy> someRenderProxyUpdates) noexcept
	{
		for (auto& renderProxy : someRenderProxyUpdates)
			m_RenderData[renderProxy.ID] = renderProxy;
	}

	void MaterialRenderSubsystem::Remove(std::vector<UUID> someIDs) noexcept
	{
		for (auto& id : someIDs)
			m_RenderData.erase(id);
	}

	void MaterialRenderSubsystem::BuildMaterialData(ShaderInterop::Material& outMaterialData, const MaterialRenderProxy& aRenderProxy) const noexcept
	{
		outMaterialData.AlbedoIndex = aRenderProxy.AlbedoMap ? aRenderProxy.AlbedoMap->GetSRVIndex() : GraphicsCommon::GetDefaultTexture(DefaultTextureType::White2D)->GetSRVIndex();
		outMaterialData.NormalIndex = aRenderProxy.NormalMap ? aRenderProxy.NormalMap->GetSRVIndex() : GraphicsCommon::GetDefaultTexture(DefaultTextureType::Normal2D)->GetSRVIndex();
		outMaterialData.RoughnessIndex = aRenderProxy.RoughnessMap ? aRenderProxy.RoughnessMap->GetSRVIndex() : GraphicsCommon::GetDefaultTexture(DefaultTextureType::White2D)->GetSRVIndex();
		outMaterialData.MetalnessIndex = aRenderProxy.MetalnessMap ? aRenderProxy.MetalnessMap->GetSRVIndex() : GraphicsCommon::GetDefaultTexture(DefaultTextureType::White2D)->GetSRVIndex();
		outMaterialData.EmissiveIndex = aRenderProxy.EmissionMap ? aRenderProxy.EmissionMap->GetSRVIndex() : GraphicsCommon::GetDefaultTexture(DefaultTextureType::White2D)->GetSRVIndex();
		outMaterialData.HeightMapIndex = aRenderProxy.DisplacementMap ? aRenderProxy.DisplacementMap->GetSRVIndex() : GraphicsCommon::GetDefaultTexture(DefaultTextureType::Black2D)->GetSRVIndex();
		outMaterialData.AOIndex = aRenderProxy.AmbientOcclusionMap ? aRenderProxy.AmbientOcclusionMap->GetSRVIndex() : GraphicsCommon::GetDefaultTexture(DefaultTextureType::White2D)->GetSRVIndex();
		outMaterialData.OpacityIndex = aRenderProxy.OpacityMap ? aRenderProxy.OpacityMap->GetSRVIndex() : GraphicsCommon::GetDefaultTexture(DefaultTextureType::White2D)->GetSRVIndex();
		outMaterialData.RoughnessMetalnessIndex = ShaderInterop::INVALID_DESCRIPTOR_INDEX; //Consider this for future.

		outMaterialData.BaseColorFactor = aRenderProxy.AlbedoColor;
		outMaterialData.EmissiveFactor = aRenderProxy.EmissiveColor;
		outMaterialData.MetalnessFactor = aRenderProxy.Metallic;
		outMaterialData.RoughnessFactor = aRenderProxy.Roughness;
		outMaterialData.AOFactor = aRenderProxy.AmbientOcclusionIntensity;
		outMaterialData.HeightFactor = aRenderProxy.DisplacementIntensity;
		outMaterialData.EmissionIntensity = aRenderProxy.EmissionIntensity;
		outMaterialData.AlphaCutOff = aRenderProxy.AlphaCutOff;
		outMaterialData.IOR = aRenderProxy.IOR;
		outMaterialData.RefractionStrength = aRenderProxy.RefractionStrength;

		outMaterialData.TilingFactor = aRenderProxy.TilingFactor;
		outMaterialData.Offset = aRenderProxy.Offset;
	}

	void MaterialRenderSubsystem::OnUpload(CommandContext& aCommandContext) noexcept
	{
		m_IDToSlotMap.clear();
		m_MaterialCache.clear();
		m_MaterialCache.reserve(m_RenderData.size());

		for (const auto& [_, renderProxy] : m_RenderData)
		{
			m_IDToSlotMap[renderProxy.ID] = (uint32)m_MaterialCache.size();

			ShaderInterop::Material& materialData = m_MaterialCache.emplace_back();
			BuildMaterialData(materialData, renderProxy);
		}

		const uint32 numElements = static_cast<uint32>(m_MaterialCache.size());
		const uint32 desiredElements = Math::AlignUp(Math::Max(1u, numElements), 8u);
		const uint32 stride = sizeof(ShaderInterop::Material);

		if (!m_MaterialDataBuffer.pBuffer || desiredElements > m_MaterialDataBuffer.pBuffer->GetNrOfElements())
			m_MaterialDataBuffer.pBuffer = m_pGraphicsDevice->CreateBuffer(BufferDesc::CreateStructured(desiredElements, stride), "MaterialData");

		const uint64 totalSize = numElements * stride;
		ScratchAllocation alloc = aCommandContext.AllocateScratch(totalSize);
		memcpy(alloc.pMappedMemory, m_MaterialCache.data(), totalSize);
		aCommandContext.CopyBuffer(alloc.pBackingResource, m_MaterialDataBuffer.pBuffer, alloc.Size, alloc.Offset, 0);
		m_MaterialDataBuffer.Count = numElements;
	}
}