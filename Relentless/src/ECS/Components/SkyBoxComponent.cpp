#include "SkyBoxComponent.h"

#include "Assets/AssetManager.h"
#include "Assets/CoreTypes/Environment.h"

#include "ECS/EntityManager.h"

#include "Utility/StringUtils.h"

namespace Relentless
{
	SkyBoxComponent::~SkyBoxComponent() noexcept
	{
		DetachPrimaryEnvironment();
		DetachBlendEnvironment();
	}

	void SkyBoxComponent::CopyFrom(const SkyBoxComponent& aOtherComponent, entity aThisEntity, EntityManager& aEntityManager)
	{
		DetachPrimaryEnvironment();
		DetachBlendEnvironment();

		m_PrimaryEnvironmentHandle = aOtherComponent.m_PrimaryEnvironmentHandle;
		m_BlendEnvironmentHandle = aOtherComponent.m_BlendEnvironmentHandle;
		m_TintColor = aOtherComponent.m_TintColor;
		m_Intensity = aOtherComponent.m_Intensity;
		m_LodBias = aOtherComponent.m_LodBias;
		m_BlendFactor = aOtherComponent.m_BlendFactor;

		ConnectPrimaryEnvironment();
		ConnectBlendEnvironment();

		m_Self = aThisEntity;
		m_EntityManager = &aEntityManager;
		m_EntityManager->AddOrReplace<DirtyRenderState>(m_Self);
	}

	Ref<Environment> SkyBoxComponent::GetBlendEnvironment() const noexcept
	{
		RLS_ASSERT(HasAssignedBlendEnvironment(), "[SkyBoxComponent::GetBlendEnvironment]: Blend Environment handle is invalid.");
		return AssetManager::Get<Environment>(m_BlendEnvironmentHandle);
	}
	
	const AssetHandle& SkyBoxComponent::GetBlendEnvironmentHandle() const noexcept
	{
		return m_BlendEnvironmentHandle;
	}

	float SkyBoxComponent::GetBlendFactor() const noexcept
	{
		return m_BlendFactor;
	}

	float SkyBoxComponent::GetIntensity() const noexcept
	{
		return m_Intensity;
	}

	float SkyBoxComponent::GetLODBias() const noexcept
	{
		return m_LodBias;
	}

	Ref<Environment> SkyBoxComponent::GetPrimaryEnvironment() const noexcept
	{
		RLS_ASSERT(HasAssignedPrimaryEnvironment(), "[SkyBoxComponent::GetPrimaryEnvironment]: Primary Environment handle is invalid.");
		return AssetManager::Get<Environment>(m_PrimaryEnvironmentHandle);
	}

	const AssetHandle& SkyBoxComponent::GetPrimaryEnvironmentHandle() const noexcept
	{
		return m_PrimaryEnvironmentHandle;
	}
	
	const Color& SkyBoxComponent::GetTintColor() const noexcept
	{
		return m_TintColor;
	}

	bool SkyBoxComponent::HasAssignedBlendEnvironment() const noexcept
	{
		return m_BlendEnvironmentHandle != AssetHandle::INVALID;
	}

	bool SkyBoxComponent::HasAssignedPrimaryEnvironment() const noexcept
	{
		return m_PrimaryEnvironmentHandle != AssetHandle::INVALID;
	}

	void SkyBoxComponent::OnBound() noexcept
	{
		m_EntityManager->AddOrReplace<SkyBoxComponent::DirtyRenderState>(m_Self);
	}

	void SkyBoxComponent::RemoveBlendEnvironment() noexcept
	{
		if (!m_BlendEnvironmentHandle.IsValid())
			return;

		DetachBlendEnvironment();
		m_BlendEnvironmentHandle = NULL_HANDLE;

		m_EntityManager->AddOrReplace<DirtyRenderState>(m_Self);
		NOTIFY_PROPERTY_CHANGED(m_BlendEnvironmentHandle);
	}

	void SkyBoxComponent::RemovePrimaryEnvironment() noexcept
	{
		if (!m_PrimaryEnvironmentHandle.IsValid())
			return;

		DetachPrimaryEnvironment();
		m_PrimaryEnvironmentHandle = NULL_HANDLE;

		m_EntityManager->AddOrReplace<DirtyRenderState>(m_Self);
		NOTIFY_PROPERTY_CHANGED(m_PrimaryEnvironmentHandle);
	}

	void SkyBoxComponent::SetBlendEnvironment(const AssetHandle& aHandle) noexcept
	{
		RLS_ASSERT(aHandle.Type == Environment::StaticType(), "[SkyBoxComponent::SetBlendEnvironment]: Invalid asset type.");

		if (m_BlendEnvironmentHandle == aHandle)
			return;

		DetachBlendEnvironment();
		m_BlendEnvironmentHandle = aHandle;
		ConnectBlendEnvironment();

		m_EntityManager->AddOrReplace<SkyBoxComponent::DirtyRenderState>(m_Self);
		NOTIFY_PROPERTY_CHANGED(m_BlendEnvironmentHandle);
	}

	void SkyBoxComponent::SetBlendFactor(float aBlendFactor) noexcept
	{
		if (Math::AreValuesClose(m_BlendFactor, aBlendFactor))
			return;

		m_BlendFactor = aBlendFactor;
		m_EntityManager->AddOrReplace<SkyBoxComponent::DirtyRenderState>(m_Self);
		NOTIFY_PROPERTY_CHANGED(m_BlendFactor);
	}

	void SkyBoxComponent::SetIntensity(float aIntensity) noexcept
	{
		if (Math::AreValuesClose(m_Intensity, aIntensity))
			return;

		m_Intensity = aIntensity;
		m_EntityManager->AddOrReplace<SkyBoxComponent::DirtyRenderState>(m_Self);
		NOTIFY_PROPERTY_CHANGED(m_Intensity);
	}

	void SkyBoxComponent::SetLODBias(float aLODBias) noexcept
	{
		if (Math::AreValuesClose(m_LodBias, aLODBias))
			return;

		m_LodBias = aLODBias;
		m_EntityManager->AddOrReplace<SkyBoxComponent::DirtyRenderState>(m_Self);
		NOTIFY_PROPERTY_CHANGED(m_LodBias);
	}

	void SkyBoxComponent::SetPrimaryEnvironment(const AssetHandle& aHandle) noexcept
	{
		RLS_ASSERT(aHandle.Type == Environment::StaticType(), "[SkyBoxComponent::SetPrimaryEnvironment]: Invalid asset type.");

		if (m_PrimaryEnvironmentHandle == aHandle)
			return;

		DetachPrimaryEnvironment();
		m_PrimaryEnvironmentHandle = aHandle;
		ConnectPrimaryEnvironment();

		m_EntityManager->AddOrReplace<SkyBoxComponent::DirtyRenderState>(m_Self);
		NOTIFY_PROPERTY_CHANGED(m_PrimaryEnvironmentHandle);
	}

	void SkyBoxComponent::SetTintColor(const Color& aColor) noexcept
	{
		if (m_TintColor == aColor)
			return;

		m_TintColor = aColor;
		m_EntityManager->AddOrReplace<SkyBoxComponent::DirtyRenderState>(m_Self);
		NOTIFY_PROPERTY_CHANGED(m_TintColor);
	}

	void SkyBoxComponent::ConnectBlendEnvironment() noexcept
	{
		if (!m_BlendEnvironmentHandle.IsValid())
			return;

		Ref<Environment> pBlendEnvironment = AssetManager::Get<Environment>(m_BlendEnvironmentHandle);
		pBlendEnvironment->OnDestroy.Connect(this, &SkyBoxComponent::OnBlendEnvironmentAssetDestroy);
		pBlendEnvironment->OnPropertyChanged.Connect(this, &SkyBoxComponent::OnEnvironmentAssetPropertyChanged);
	}

	void SkyBoxComponent::ConnectPrimaryEnvironment() noexcept
	{
		if (!m_PrimaryEnvironmentHandle.IsValid())
			return;

		Ref<Environment> pPrimaryEnvironment = AssetManager::Get<Environment>(m_PrimaryEnvironmentHandle);
		pPrimaryEnvironment->OnDestroy.Connect(this, &SkyBoxComponent::OnPrimaryEnvironmentAssetDestroy);
		pPrimaryEnvironment->OnPropertyChanged.Connect(this, &SkyBoxComponent::OnEnvironmentAssetPropertyChanged);
	}

	void SkyBoxComponent::DetachBlendEnvironment() noexcept
	{
		if (!m_BlendEnvironmentHandle.IsValid())
			return;

		Ref<Environment> pBlendEnvironment = AssetManager::Get<Environment>(m_BlendEnvironmentHandle);
		pBlendEnvironment->OnDestroy.Detach(this);
		pBlendEnvironment->OnPropertyChanged.Detach(this);
	}

	void SkyBoxComponent::DetachPrimaryEnvironment() noexcept
	{
		if (!m_PrimaryEnvironmentHandle.IsValid())
			return;

		Ref<Environment> pPrimaryEnvironment = AssetManager::Get<Environment>(m_PrimaryEnvironmentHandle);
		pPrimaryEnvironment->OnDestroy.Detach(this);
		pPrimaryEnvironment->OnPropertyChanged.Detach(this);
	}

	void SkyBoxComponent::OnBlendEnvironmentAssetDestroy(MAYBE_UNUSED IAsset* aAsset) noexcept
	{
		RemoveBlendEnvironment();
	}

	void SkyBoxComponent::OnPrimaryEnvironmentAssetDestroy(MAYBE_UNUSED IAsset* aAsset) noexcept
	{
		RemovePrimaryEnvironment();
	}

	void SkyBoxComponent::OnEnvironmentAssetPropertyChanged(MAYBE_UNUSED IAsset* aAsset, MAYBE_UNUSED uint64 aProperty) noexcept
	{
		m_EntityManager->AddOrReplace<DirtyRenderState>(m_Self);
	}
}
