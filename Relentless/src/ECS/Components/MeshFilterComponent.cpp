#include "MeshFilterComponent.h"

#include "Assets/AssetManager.h"
#include "Assets/CoreTypes/Mesh.h"

#include "ECS/EntityManager.h"

namespace Relentless
{
	MeshFilterComponent::~MeshFilterComponent() noexcept
	{
		DetachMesh();
	}

	void MeshFilterComponent::CopyFrom(const MeshFilterComponent& aOtherComponent, entity aThisEntity, EntityManager& aEntityManager)
	{
		m_Self = aThisEntity;
		m_EntityManager = &aEntityManager;

		DetachMesh();
		m_MeshHandle = aOtherComponent.m_MeshHandle;
		ConnectMesh();

		m_EntityManager->AddOrReplace<DirtyRenderState>(m_Self);
	}

	Ref<Mesh> MeshFilterComponent::GetMesh() const noexcept
	{
		RLS_ASSERT(m_MeshHandle.IsValid(), "[MeshFilterComponent::GetMesh]: Mesh handle is invalid.");
		return AssetManager::Get<Mesh>(m_MeshHandle);
	}

	const AssetHandle& MeshFilterComponent::GetMeshHandle() const noexcept
	{
		return m_MeshHandle;
	}

	bool MeshFilterComponent::HasAssignedMesh() const noexcept
	{
		return m_MeshHandle.IsValid();
	}

	void MeshFilterComponent::OnBound() noexcept
	{
		m_EntityManager->AddOrReplace<DirtyRenderState>(m_Self);
		ConnectMesh();
	}

	void MeshFilterComponent::RemoveMesh() noexcept
	{
		if (!m_MeshHandle.IsValid())
			return;

		DetachMesh();

		m_EntityManager->AddOrReplace<DirtyRenderState>(m_Self);
		NOTIFY_PROPERTY_CHANGED(m_MeshHandle);
	}

	void MeshFilterComponent::SetMesh(const AssetHandle& aAssetHandle) noexcept
	{
		RLS_ASSERT(aAssetHandle.Type == Mesh::StaticType(), "[MeshFilterComponent::SetMesh]: Asset handle is not of mesh type.");

		if (m_MeshHandle == aAssetHandle)
			return;

		DetachMesh();
		m_MeshHandle = aAssetHandle;
		ConnectMesh();

		m_EntityManager->AddOrReplace<DirtyRenderState>(m_Self);
		NOTIFY_PROPERTY_CHANGED(m_MeshHandle);
	}

	void MeshFilterComponent::ConnectMesh() noexcept
	{
		if (!m_MeshHandle.IsValid())
			return;

		Ref<Mesh> pMesh = AssetManager::Get<Mesh>(m_MeshHandle);

		m_MeshDestroyCallbackID = pMesh->OnDestroy.Connect(
			[pManager = m_EntityManager, self = m_Self](MAYBE_UNUSED IAsset* aAsset)
			{
				pManager->Get<MeshFilterComponent>(self).RemoveMesh();
			});

		m_MeshChangedCallbackID = pMesh->OnPropertyChanged.Connect(
			[pManager = m_EntityManager, self = m_Self](MAYBE_UNUSED IAsset* aAsset, MAYBE_UNUSED uint64 aProperty)
			{
				pManager->AddOrReplace<DirtyRenderState>(self);
			});
	}

	void MeshFilterComponent::DetachMesh() noexcept
	{
		if (!m_MeshHandle.IsValid())
			return;

		Ref<Mesh> pMesh = AssetManager::Get<Mesh>(m_MeshHandle);
		pMesh->OnDestroy.Detach(m_MeshDestroyCallbackID);
		pMesh->OnPropertyChanged.Detach(m_MeshChangedCallbackID);

		m_MeshDestroyCallbackID = INVALID_CALLBACK_ID;
		m_MeshChangedCallbackID = INVALID_CALLBACK_ID;
		m_MeshHandle = AssetHandle::INVALID;
	}

	void MeshFilterComponent::OnMeshAssetDestroy(MAYBE_UNUSED IAsset* aAsset) noexcept
	{
		RemoveMesh();
	}

	void MeshFilterComponent::OnMeshAssetPropertyChanged(MAYBE_UNUSED IAsset* aAsset, MAYBE_UNUSED uint64 aProperty) noexcept
	{
		m_EntityManager->AddOrReplace<DirtyRenderState>(m_Self);
	}

}