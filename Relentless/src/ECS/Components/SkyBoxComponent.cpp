#include "SkyBoxComponent.h"

#include "Assets/AssetManager.h"
#include "Assets/CoreTypes/Environment.h"

#include "ECS/EntityManager.h"

#include "Utility/StringUtils.h"

namespace Relentless
{
	void SkyBoxComponent::CopyFrom(const SkyBoxComponent& aOtherComponent, entity aThisEntity, EntityManager& aEntityManager)
	{
		m_PrimaryEnvironmentHandle = aOtherComponent.m_PrimaryEnvironmentHandle;
		m_BlendEnvironmentHandle = aOtherComponent.m_BlendEnvironmentHandle;
		m_TintColor = aOtherComponent.m_TintColor;
		m_Intensity = aOtherComponent.m_Intensity;
		m_LodBias = aOtherComponent.m_LodBias;
		m_BlendFactor = aOtherComponent.m_BlendFactor;

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

	void SkyBoxComponent::SetBlendEnvironment(const AssetHandle& aHandle) noexcept
	{
		RLS_ASSERT(aHandle.Type == Environment::StaticType(), "[SkyBoxComponent::SetBlendEnvironment]: Invalid asset type.");

		if (m_BlendEnvironmentHandle == aHandle)
			return;

		m_BlendEnvironmentHandle = aHandle;
		m_EntityManager->AddOrReplace<SkyBoxComponent::DirtyRenderState>(m_Self);
		BroadcastPropertyChanged("m_BlendEnvironmentHandle"_h);
	}

	void SkyBoxComponent::SetBlendFactor(float aBlendFactor) noexcept
	{
		if (Math::AreValuesClose(m_BlendFactor, aBlendFactor))
			return;

		m_BlendFactor = aBlendFactor;
		m_EntityManager->AddOrReplace<SkyBoxComponent::DirtyRenderState>(m_Self);
		BroadcastPropertyChanged("m_BlendFactor"_h);
	}

	void SkyBoxComponent::SetIntensity(float aIntensity) noexcept
	{
		if (Math::AreValuesClose(m_Intensity, aIntensity))
			return;

		m_Intensity = aIntensity;
		m_EntityManager->AddOrReplace<SkyBoxComponent::DirtyRenderState>(m_Self);
		BroadcastPropertyChanged("m_Intensity"_h);
	}

	void SkyBoxComponent::SetLODBias(float aLODBias) noexcept
	{
		if (Math::AreValuesClose(m_LodBias, aLODBias))
			return;

		m_LodBias = aLODBias;
		m_EntityManager->AddOrReplace<SkyBoxComponent::DirtyRenderState>(m_Self);
		BroadcastPropertyChanged("m_LodBias"_h);
	}

	void SkyBoxComponent::SetPrimaryEnvironment(const AssetHandle& aHandle) noexcept
	{
		RLS_ASSERT(aHandle.Type == Environment::StaticType(), "[SkyBoxComponent::SetPrimaryEnvironment]: Invalid asset type.");

		if (m_PrimaryEnvironmentHandle == aHandle)
			return;

		m_PrimaryEnvironmentHandle = aHandle;
		m_EntityManager->AddOrReplace<SkyBoxComponent::DirtyRenderState>(m_Self);
		BroadcastPropertyChanged("m_PrimaryEnvironmentHandle"_h);
	}

	void SkyBoxComponent::SetTintColor(const Color& aColor) noexcept
	{
		if (m_TintColor == aColor)
			return;

		m_TintColor = aColor;
		m_EntityManager->AddOrReplace<SkyBoxComponent::DirtyRenderState>(m_Self);
		BroadcastPropertyChanged("m_TintColor"_h);
	}
}
