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

	void MeshRendererComponent::CopyFrom(const MeshRendererComponent& aOtherComponent, entity aThisEntity, EntityManager& aEntityManager)
	{
		m_Self = aThisEntity;
		m_EntityManager = &aEntityManager;

		DetachMaterial();
		m_MaterialHandle = aOtherComponent.m_MaterialHandle;
		ConnectMaterial();

		m_LightChannels = aOtherComponent.m_LightChannels;
		m_CastShadows = aOtherComponent.m_CastShadows;

		m_EntityManager->AddOrReplace<DirtyRenderState>(m_Self);
	}

	ELightChannel MeshRendererComponent::GetLightChannels() const noexcept
	{
		return m_LightChannels;
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

	bool MeshRendererComponent::HasLightChannelsEnabled(ELightChannel aChannels) const noexcept
	{
		return EnumHasAnyFlags(m_LightChannels, aChannels);
	}

	bool MeshRendererComponent::HasAssignedMaterial() const noexcept
	{
		return m_MaterialHandle.IsValid();
	}

	bool MeshRendererComponent::IsCastingShadows() const noexcept
	{
		return m_CastShadows;
	}

	void MeshRendererComponent::OnBound() noexcept
	{
		m_EntityManager->AddOrReplace<DirtyRenderState>(m_Self);
		ConnectMaterial();
	}

	void MeshRendererComponent::RemoveMaterial() noexcept
	{
		if (!m_MaterialHandle.IsValid())
			return;

		DetachMaterial();

		m_EntityManager->AddOrReplace<DirtyRenderState>(m_Self);
		NOTIFY_PROPERTY_CHANGED(m_MaterialHandle);
	}

	void MeshRendererComponent::SetCastShadows(bool aCastShadows) noexcept
	{
		if (m_CastShadows == aCastShadows)
			return;

		m_CastShadows = aCastShadows;
		m_EntityManager->AddOrReplace<DirtyRenderState>(m_Self);
		NOTIFY_PROPERTY_CHANGED(m_CastShadows);
	}

	void MeshRendererComponent::SetLightChannelEnabled(ELightChannel aChannel, bool aEnabled) noexcept
	{
		ApplyChannelMask(aEnabled ? m_LightChannels | aChannel : m_LightChannels & ~aChannel);
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

	void MeshRendererComponent::ApplyChannelMask(ELightChannel aNewMask) noexcept
	{
		if (m_LightChannels == aNewMask)
			return;
		
		m_LightChannels = aNewMask;
		this->m_EntityManager->template AddOrReplace<DirtyRenderState>(this->m_Self);
		NOTIFY_PROPERTY_CHANGED(m_LightChannels);
	}

	void MeshRendererComponent::ConnectMaterial() noexcept
	{
		if (!m_MaterialHandle.IsValid())
			return;

		Ref<Material> pMaterial = AssetManager::Get<Material>(m_MaterialHandle);

		m_MaterialDestroyCallbackID = pMaterial->OnDestroy.Connect(
			[pManager = m_EntityManager, self = m_Self](MAYBE_UNUSED IAsset* aAsset)
			{
				pManager->Get<MeshRendererComponent>(self).RemoveMaterial();
			});

		m_MaterialChangedCallbackID = pMaterial->OnPropertyChanged.Connect(
			[pManager = m_EntityManager, self = m_Self](MAYBE_UNUSED IAsset* aAsset, MAYBE_UNUSED uint64 aProperty)
			{
				pManager->AddOrReplace<DirtyRenderState>(self);
			});
	}

	void MeshRendererComponent::DetachMaterial() noexcept
	{
		if (!m_MaterialHandle.IsValid())
			return;

		Ref<Material> pMaterial = AssetManager::Get<Material>(m_MaterialHandle);
		pMaterial->OnDestroy.Detach(m_MaterialDestroyCallbackID);
		pMaterial->OnPropertyChanged.Detach(m_MaterialChangedCallbackID);

		m_MaterialDestroyCallbackID = INVALID_CALLBACK_ID;
		m_MaterialChangedCallbackID = INVALID_CALLBACK_ID;
		m_MaterialHandle = AssetHandle::INVALID;
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