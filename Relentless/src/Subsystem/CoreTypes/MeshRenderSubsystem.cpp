#include "MeshRenderSubsystem.h"

#include "Graphics/Renderer/Renderer.h"
#include "Graphics/RHI/CommandContext.h"
#include "Graphics/RHI/Device.h"
#include "Graphics/Scene/RenderScene.h"

namespace Relentless
{
	uint32 MeshRenderSubsystem::GetNumMeshes() const noexcept
	{
		return m_MeshDataBuffer.Count;
	}

	const Buffer* MeshRenderSubsystem::GetRenderData() const noexcept
	{
		RLS_ASSERT(m_MeshDataBuffer.pBuffer, "[MeshRenderSubsystem::GetRenderData]: Buffer is invalid.");
		return m_MeshDataBuffer.pBuffer;
	}

	uint32 MeshRenderSubsystem::GetSlotIndex(const UUID& aUUID) const noexcept
	{
		RLS_ASSERT(m_IDToSlotMap.contains(aUUID), "[MeshRenderSubsystem::GetSlotIndex]: Slot Index does not exist for UUID");
		return m_IDToSlotMap.at(aUUID);
	}

	const MeshRenderProxy& MeshRenderSubsystem::GetProxy(const UUID& aUUID) const noexcept
	{
		RLS_ASSERT(m_RenderData.contains(aUUID), "[MeshRenderSubsystem::GetProxy]: Proxy does not exist for UUID.");
		return m_RenderData.at(aUUID);
	}

	bool MeshRenderSubsystem::OnLoad(ISystemManager* aSystemManager) noexcept
	{
		RenderScene* pRenderScene = static_cast<RenderScene*>(aSystemManager);

		Renderer* pRenderer = pRenderScene->GetRenderer();
		m_OnUploadCallbackID = pRenderer->RegisterOnUploadCallback(Callback<void(CommandContext&)>::Bind(this, &MeshRenderSubsystem::OnUpload));
		m_pGraphicsDevice = pRenderer->GetDevice();

		return true;
	}

	void MeshRenderSubsystem::OnUnload(ISystemManager* aSystemManager) noexcept
	{
		RenderScene* pRenderScene = static_cast<RenderScene*>(aSystemManager);
		Renderer* pRenderer = pRenderScene->GetRenderer();
		pRenderer->UnregisterOnUploadCallback(m_OnUploadCallbackID);
	}

	bool MeshRenderSubsystem::ShouldCreateSubsystem(ISystemManager* aSystemManager) noexcept
	{
		return dynamic_cast<RenderScene*>(aSystemManager) != nullptr;
	}

	void MeshRenderSubsystem::Patch(std::vector<MeshRenderProxy> someRenderProxyUpdates) noexcept
	{
		for (auto& renderProxy : someRenderProxyUpdates)
			m_RenderData[renderProxy.ID] = renderProxy;
	}

	void MeshRenderSubsystem::Remove(std::vector<UUID> someIDs) noexcept
	{
		for (auto& id : someIDs)
			m_RenderData.erase(id);
	}

	void MeshRenderSubsystem::BuildMeshData(ShaderInterop::MeshData& outMeshData, const MeshRenderProxy& aRenderProxy) const noexcept
	{
		outMeshData.VertexBufferIndex = aRenderProxy.VertexBuffer->GetSRVIndex();
		outMeshData.IndexBufferIndex = aRenderProxy.IndexBuffer->GetSRVIndex();
	}

	void MeshRenderSubsystem::OnUpload(CommandContext& aCommandContext) noexcept
	{
		m_IDToSlotMap.clear();
		m_MeshCache.clear();
		m_MeshCache.reserve(m_RenderData.size());

		for (const auto& [_, renderProxy] : m_RenderData)
		{
			if (renderProxy.VertexBuffer == nullptr || renderProxy.IndexBuffer == nullptr)
				continue;

			m_IDToSlotMap[renderProxy.ID] = (uint32)m_MeshCache.size();

			ShaderInterop::MeshData& meshData = m_MeshCache.emplace_back();
			BuildMeshData(meshData, renderProxy);
		}

		const uint32 numElements = static_cast<uint32>(m_MeshCache.size());
		const uint32 desiredElements = Math::AlignUp(Math::Max(1u, numElements), 8u);
		const uint32 stride = sizeof(ShaderInterop::MeshData);

		if (!m_MeshDataBuffer.pBuffer || desiredElements > m_MeshDataBuffer.pBuffer->GetNrOfElements())
			m_MeshDataBuffer.pBuffer = m_pGraphicsDevice->CreateBuffer(BufferDesc::CreateStructured(desiredElements, stride), "MeshData");

		const uint64 totalSize = numElements * stride;
		ScratchAllocation alloc = aCommandContext.AllocateScratch(totalSize);
		memcpy(alloc.pMappedMemory, m_MeshCache.data(), totalSize);
		aCommandContext.CopyBuffer(alloc.pBackingResource, m_MeshDataBuffer.pBuffer, alloc.Size, alloc.Offset, 0);
		m_MeshDataBuffer.Count = numElements;
	}
}