#include "MeshRendererComponent.h"

#include "Assets/AssetManager.h"
#include "Assets/CoreTypes/Material.h"

#include "ECS/EntityManager.h"

namespace Relentless
{
	MeshRendererComponent::~MeshRendererComponent() noexcept
	{
		DetachMaterial();
	}

	MeshRendererComponent::MeshRendererComponent(MeshRendererComponent&& aOther) noexcept
		: ManagedComponent<MeshRendererComponent>(std::move(aOther))
		, m_MaterialHandle(aOther.m_MaterialHandle)
	{
		aOther.DetachMaterial();
		aOther.m_MaterialHandle = NULL_HANDLE;
		ConnectMaterial();
	}

	MeshRendererComponent& MeshRendererComponent::operator=(MeshRendererComponent&& aOther) noexcept
	{
		if (this != &aOther)
		{
			DetachMaterial();
			aOther.DetachMaterial();
			m_MaterialHandle = std::move(aOther.m_MaterialHandle);
			aOther.m_MaterialHandle = NULL_HANDLE;
			ConnectMaterial();
		}
		return *this;
	}

	void MeshRendererComponent::CopyFrom(const MeshRendererComponent& aOtherComponent, entity aThisEntity, EntityManager& aEntityManager)
	{
		DetachMaterial();
		m_MaterialHandle = aOtherComponent.m_MaterialHandle;
		ConnectMaterial();

		m_Self = aThisEntity;
		m_EntityManager = &aEntityManager;
		m_EntityManager->AddOrReplace<DirtyRenderState>(m_Self);
	}

	Ref<Material> MeshRendererComponent::GetMaterial() const noexcept
	{
		RLS_ASSERT(m_MaterialHandle.IsValid(), "[MeshRendererComponent::GetMaterial]: Material handle is invalid.");
		return AssetManager::Get<Material>(m_MaterialHandle);
	}

	const AssetHandle& MeshRendererComponent::GetMaterialHandle() const noexcept
	{
		return m_MaterialHandle;
	}

	bool MeshRendererComponent::HasAssignedMaterial() const noexcept
	{
		return m_MaterialHandle.IsValid();
	}

	void MeshRendererComponent::OnBound() noexcept
	{
		m_EntityManager->AddOrReplace<DirtyRenderState>(m_Self);
	}

	void MeshRendererComponent::RemoveMaterial() noexcept
	{
		if (!m_MaterialHandle.IsValid())
			return;

		DetachMaterial();
		m_MaterialHandle = NULL_HANDLE;

		m_EntityManager->AddOrReplace<DirtyRenderState>(m_Self);
		NOTIFY_PROPERTY_CHANGED(m_MaterialHandle);
	}

	void MeshRendererComponent::SetMaterial(const AssetHandle& aAssetHandle) noexcept
	{
		RLS_ASSERT(aAssetHandle.Type == Material::StaticType(), "[MeshRendererComponent::SetMaterial]: Asset handle is not of material type.");

		if (m_MaterialHandle == aAssetHandle)
			return;

		DetachMaterial();
		m_MaterialHandle = aAssetHandle;
		ConnectMaterial();

		m_EntityManager->AddOrReplace<DirtyRenderState>(m_Self);
		NOTIFY_PROPERTY_CHANGED(m_MaterialHandle);
	}

	void MeshRendererComponent::ConnectMaterial() noexcept
	{
		if (!m_MaterialHandle.IsValid())
			return;

		Ref<Material> pMaterial = AssetManager::Get<Material>(m_MaterialHandle);
		pMaterial->OnDestroy.Connect(this, &MeshRendererComponent::OnMaterialAssetDestroy);
		pMaterial->OnPropertyChanged.Connect(this, &MeshRendererComponent::OnMaterialAssetPropertyChanged);
	}

	void MeshRendererComponent::DetachMaterial() noexcept
	{
		if (!m_MaterialHandle.IsValid())
			return;

		Ref<Material> pMaterial = AssetManager::Get<Material>(m_MaterialHandle);
		pMaterial->OnDestroy.Detach(this);
		pMaterial->OnPropertyChanged.Detach(this);
	}

	void MeshRendererComponent::OnMaterialAssetDestroy(MAYBE_UNUSED IAsset* aAsset) noexcept
	{
		RemoveMaterial();
	}

	void MeshRendererComponent::OnMaterialAssetPropertyChanged(MAYBE_UNUSED IAsset* aAsset, MAYBE_UNUSED uint64 aProperty) noexcept
	{
		m_EntityManager->AddOrReplace<DirtyRenderState>(m_Self);
	}

}