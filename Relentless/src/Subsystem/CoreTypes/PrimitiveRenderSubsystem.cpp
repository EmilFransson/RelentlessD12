#include "PrimitiveRenderSubsystem.h"

#include "Graphics/RHI/CommandContext.h"
#include "Graphics/RHI/Device.h"
#include "Graphics/Renderer/Renderer.h"
#include "Graphics/Scene/RenderScene.h"

#include "Subsystem/CoreTypes/MaterialRenderSubsystem.h"
#include "Subsystem/CoreTypes/MeshRenderSubsystem.h"

namespace Relentless
{
	uint32 PrimitiveRenderSubsystem::GetNumInstances() const noexcept
	{
		return m_InstanceDataBuffer.Count;
	}

	const Buffer* PrimitiveRenderSubsystem::GetRenderData() const
	{
		RLS_ASSERT(m_InstanceDataBuffer.pBuffer, "[PrimitiveRenderSubsystem::GetRenderData]: Buffer is invalid.");
		return m_InstanceDataBuffer.pBuffer;
	}

	const std::vector<ShaderInterop::InstanceData>& PrimitiveRenderSubsystem::GetInstanceCache() const
	{
		return m_InstanceCache;
	}

	const PrimitiveRenderProxy& PrimitiveRenderSubsystem::GetProxy(uint32 aEntityID) const
	{
		RLS_ASSERT(m_RenderData.contains(aEntityID), "[PrimitiveRenderSubsystem::GetProxy]: Proxy does not exist for aEntityID.");
		return m_RenderData.at(aEntityID);
	}

	bool PrimitiveRenderSubsystem::OnLoad(ISystemManager* aSystemManager) noexcept
	{
		RenderScene* pRenderScene = static_cast<RenderScene*>(aSystemManager);
		m_pMaterialRenderSubsystem = pRenderScene->GetSubsystem<MaterialRenderSubsystem>();
		m_pMeshRenderSubsystem = pRenderScene->GetSubsystem<MeshRenderSubsystem>();

		Renderer* pRenderer = pRenderScene->GetRenderer();
		m_OnUploadCallbackID = pRenderer->RegisterOnUploadCallback(Callback<void(CommandContext&)>::Bind(this, &PrimitiveRenderSubsystem::OnUpload));
		m_pGraphicsDevice = pRenderer->GetDevice();

		return true;
	}

	void PrimitiveRenderSubsystem::OnUnload(ISystemManager* aSystemManager) noexcept
	{
		RenderScene* pRenderScene = static_cast<RenderScene*>(aSystemManager);

		Renderer* pRenderer = pRenderScene->GetRenderer();
		pRenderer->UnregisterOnUploadCallback(m_OnUploadCallbackID);
	}

	bool PrimitiveRenderSubsystem::ShouldCreateSubsystem(ISystemManager* aSystemManager) noexcept
	{
		return dynamic_cast<RenderScene*>(aSystemManager) != nullptr;
	}

	void PrimitiveRenderSubsystem::Patch(std::vector<PrimitiveRenderProxy> someRenderProxyUpdates) noexcept
	{
		for (auto& renderProxy : someRenderProxyUpdates)
			m_RenderData[renderProxy.EntityID] = renderProxy;
	}

	void PrimitiveRenderSubsystem::Remove(std::vector<uint32> someIDs) noexcept
	{
		for (auto& id : someIDs)
			m_RenderData.erase(id);
	}

	void PrimitiveRenderSubsystem::BuildInstanceData(ShaderInterop::InstanceData& outInstanceData, const PrimitiveRenderProxy& aRenderProxy) const noexcept
	{
		outInstanceData.LocalToWorld = aRenderProxy.LocalToWorld;
		outInstanceData.MaterialIndex = m_pMaterialRenderSubsystem->GetSlotIndex(aRenderProxy.MaterialUUID);
		outInstanceData.MeshDataIndex = m_pMeshRenderSubsystem->GetSlotIndex(aRenderProxy.MeshUUID);
		outInstanceData.EntityID = aRenderProxy.EntityID;
	}

	void PrimitiveRenderSubsystem::OnUpload(CommandContext& aCommandContext) noexcept
	{
		PROFILE_FUNC;

		m_InstanceCache.clear();
		m_InstanceCache.reserve(m_RenderData.size());

		for (const auto& [entityID, renderProxy] : m_RenderData)
		{
			if (!renderProxy.Visible)
				continue;

			//Possibly fall back to pink/magenta default material here.
			if (renderProxy.MaterialUUID == NULL_UUID)
				continue;

			if (renderProxy.MeshUUID == NULL_UUID)
				continue;

			ShaderInterop::InstanceData& instanceData = m_InstanceCache.emplace_back();
			BuildInstanceData(instanceData, renderProxy);
		}

		auto&& CompareSort = [this](const ShaderInterop::InstanceData& a, const ShaderInterop::InstanceData& b)
			{
				const PrimitiveRenderProxy& primitiveProxyA = m_RenderData.at(a.EntityID);
				const MaterialRenderProxy& materialProxyA = m_pMaterialRenderSubsystem->GetProxy(primitiveProxyA.MaterialUUID);

				const PrimitiveRenderProxy& primitiveProxyB = m_RenderData.at(b.EntityID);
				const MaterialRenderProxy& materialProxyB = m_pMaterialRenderSubsystem->GetProxy(primitiveProxyB.MaterialUUID);

				if (materialProxyA.BlendMode != materialProxyB.BlendMode)
					return (int)materialProxyA.BlendMode < (int)materialProxyB.BlendMode;

				if (materialProxyA.IsTwoSided != materialProxyB.IsTwoSided)
					return (int)materialProxyA.IsTwoSided < (int)materialProxyB.IsTwoSided;

				return a.MeshDataIndex < b.MeshDataIndex;
			};

		std::sort(m_InstanceCache.begin(), m_InstanceCache.end(), CompareSort);

		for (uint32 instanceID = 0u; instanceID < m_InstanceCache.size(); ++instanceID)
			m_InstanceCache[instanceID].ID = instanceID;

		const uint32 numElements = static_cast<uint32>(m_InstanceCache.size());
		const uint32 desiredElements = Math::AlignUp(Math::Max(1u, numElements), 8u);
		const uint32 stride = sizeof(ShaderInterop::InstanceData);

		if (!m_InstanceDataBuffer.pBuffer || desiredElements > m_InstanceDataBuffer.pBuffer->GetNrOfElements())
			m_InstanceDataBuffer.pBuffer = m_pGraphicsDevice->CreateBuffer(BufferDesc::CreateStructured(desiredElements, stride), "InstanceData");

		const uint64 totalSize = numElements * stride;
		ScratchAllocation alloc = aCommandContext.AllocateScratch(totalSize);
		memcpy(alloc.pMappedMemory, m_InstanceCache.data(), totalSize);
		aCommandContext.CopyBuffer(alloc.pBackingResource, m_InstanceDataBuffer.pBuffer, alloc.Size, alloc.Offset, 0);
		m_InstanceDataBuffer.Count = numElements;
	}
}