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

	MeshFilterComponent::MeshFilterComponent(MeshFilterComponent&& aOther) noexcept
		: ManagedComponent<MeshFilterComponent>(std::move(aOther))
		, m_MeshHandle(aOther.m_MeshHandle)
	{
		aOther.DetachMesh();
		aOther.m_MeshHandle = NULL_HANDLE;
		ConnectMesh();
	}

	MeshFilterComponent& MeshFilterComponent::operator=(MeshFilterComponent&& aOther) noexcept
	{
		if (this != &aOther)
		{
			DetachMesh();
			aOther.DetachMesh();
			m_MeshHandle = std::move(aOther.m_MeshHandle);
			aOther.m_MeshHandle = NULL_HANDLE;
			ConnectMesh();
		}
		return *this;
	}

	void MeshFilterComponent::CopyFrom(const MeshFilterComponent& aOtherComponent, entity aThisEntity, EntityManager& aEntityManager)
	{
		DetachMesh();
		m_MeshHandle = aOtherComponent.m_MeshHandle;
		ConnectMesh();

		m_Self = aThisEntity;
		m_EntityManager = &aEntityManager;
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
	}

	void MeshFilterComponent::RemoveMesh() noexcept
	{
		if (!m_MeshHandle.IsValid())
			return;

		m_MeshHandle = NULL_HANDLE;
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
		pMesh->OnDestroy.Connect(this, &MeshFilterComponent::OnMeshAssetDestroy);
		pMesh->OnPropertyChanged.Connect(this, &MeshFilterComponent::OnMeshAssetPropertyChanged);
	}

	void MeshFilterComponent::DetachMesh() noexcept
	{
		if (!m_MeshHandle.IsValid())
			return;

		Ref<Mesh> pMesh = AssetManager::Get<Mesh>(m_MeshHandle);
		pMesh->OnDestroy.Detach(this);
		pMesh->OnPropertyChanged.Detach(this);
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