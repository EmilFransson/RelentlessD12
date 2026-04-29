#include "SkyLightComponent.h"

#include "Assets/AssetManager.h"
#include "Assets/CoreTypes/Environment.h"

#include "ECS/EntityManager.h"

#include "Utility/StringUtils.h"

namespace Relentless
{
	SkyLightComponent::~SkyLightComponent() noexcept
	{
		DetachPrimaryEnvironment();
		DetachBlendEnvironment();
	}

	void SkyLightComponent::CopyFrom(const SkyLightComponent& aOtherComponent, entity aThisEntity, EntityManager& aEntityManager)
	{
		DetachPrimaryEnvironment();
		DetachBlendEnvironment();

		m_PrimaryEnvironmentHandle = aOtherComponent.m_PrimaryEnvironmentHandle;
		m_BlendEnvironmentHandle = aOtherComponent.m_BlendEnvironmentHandle;
		m_LowerHemisphereColor = aOtherComponent.m_LowerHemisphereColor;
		m_TintColor = aOtherComponent.m_TintColor;
		m_RadianceMapSize = aOtherComponent.m_RadianceMapSize;
		m_RealtimeMipsPerFrame = aOtherComponent.m_RealtimeMipsPerFrame;
		m_Intensity = aOtherComponent.m_Intensity;
		m_CaptureMode = aOtherComponent.m_CaptureMode;
		m_LowerHemisphereMode = aOtherComponent.m_LowerHemisphereMode;
		m_BlendFactor = aOtherComponent.m_BlendFactor;

		ConnectPrimaryEnvironment();
		ConnectBlendEnvironment();

		m_Self = aThisEntity;
		m_EntityManager = &aEntityManager;
		m_EntityManager->AddOrReplace<DirtyRenderState>(m_Self);
	}

	Ref<Environment> SkyLightComponent::GetBlendEnvironment() const noexcept
	{
		RLS_ASSERT(HasAssignedBlendEnvironment(), "[SkyLightComponent::GetBlendEnvironment]: Environment handle is invalid.");
		return AssetManager::Get<Environment>(m_BlendEnvironmentHandle);
	}

	const AssetHandle& SkyLightComponent::GetBlendEnvironmentHandle() const noexcept
	{
		return m_BlendEnvironmentHandle;
	}

	float SkyLightComponent::GetBlendFactor() const noexcept
	{
		return m_BlendFactor;
	}

	ESkyLightCaptureMode SkyLightComponent::GetCaptureMode() const noexcept
	{
		return m_CaptureMode;
	}

	Ref<Environment> SkyLightComponent::GetPrimaryEnvironment() const noexcept
	{
		RLS_ASSERT(HasAssignedPrimaryEnvironment(), "[SkyLightComponent::GetPrimaryEnvironment]: Environment handle is invalid.");
		return AssetManager::Get<Environment>(m_PrimaryEnvironmentHandle);
	}

	const AssetHandle& SkyLightComponent::GetPrimaryEnvironmentHandle() const noexcept
	{
		return m_PrimaryEnvironmentHandle;
	}

	const Color& SkyLightComponent::GetLowerHemisphereColor() const noexcept
	{
		return m_LowerHemisphereColor;
	}

	ESkyLightLowerHemisphereMode SkyLightComponent::GetLowerHemisphereMode() const noexcept
	{
		return m_LowerHemisphereMode;
	}

	float SkyLightComponent::GetIntensity() const noexcept
	{
		return m_Intensity;
	}

	uint32 SkyLightComponent::GetRadianceMapSize() const noexcept
	{
		return m_RadianceMapSize;
	}

	uint32 SkyLightComponent::GetRealtimeMipsPerFrame() const noexcept
	{
		return m_RealtimeMipsPerFrame;
	}

	const Color& SkyLightComponent::GetTintColor() const noexcept
	{
		return m_TintColor;
	}

	bool SkyLightComponent::HasAssignedBlendEnvironment() const noexcept
	{
		return m_BlendEnvironmentHandle != AssetHandle::INVALID;
	}

	bool SkyLightComponent::HasAssignedPrimaryEnvironment() const noexcept
	{
		return m_PrimaryEnvironmentHandle != AssetHandle::INVALID;
	}

	void SkyLightComponent::OnBound() noexcept
	{
		m_EntityManager->AddOrReplace<DirtyRenderState>(m_Self);
	}

	void SkyLightComponent::RemoveBlendEnvironment() noexcept
	{
		if (!m_BlendEnvironmentHandle.IsValid())
			return;

		DetachBlendEnvironment();
		m_BlendEnvironmentHandle = NULL_HANDLE;

		m_EntityManager->AddOrReplace<DirtyRenderState>(m_Self);
		NOTIFY_PROPERTY_CHANGED(m_BlendEnvironmentHandle);
	}

	void SkyLightComponent::RemovePrimaryEnvironment() noexcept
	{
		if (!m_PrimaryEnvironmentHandle.IsValid())
			return;

		DetachPrimaryEnvironment();
		m_PrimaryEnvironmentHandle = NULL_HANDLE;
		
		m_EntityManager->AddOrReplace<DirtyRenderState>(m_Self);
		NOTIFY_PROPERTY_CHANGED(m_PrimaryEnvironmentHandle);
	}

	void SkyLightComponent::SetBlendEnvironment(const AssetHandle& aHandle) noexcept
	{
		RLS_ASSERT(aHandle.Type == Environment::StaticType(), "[SkyLightComponent::SetBlendEnvironment]: Invalid asset type.");

		if (m_BlendEnvironmentHandle == aHandle)
			return;

		DetachBlendEnvironment();
		m_BlendEnvironmentHandle = aHandle;
		ConnectBlendEnvironment();

		m_EntityManager->AddOrReplace<DirtyRenderState>(m_Self);
		NOTIFY_PROPERTY_CHANGED(m_BlendEnvironmentHandle);
	}

	void SkyLightComponent::SetBlendFactor(float aBlendFactor) noexcept
	{
		if (Math::AreValuesClose(aBlendFactor, m_BlendFactor))
			return;

		m_BlendFactor = aBlendFactor;
		m_EntityManager->AddOrReplace<DirtyRenderState>(m_Self);
		NOTIFY_PROPERTY_CHANGED(m_BlendFactor);
	}

	void SkyLightComponent::SetCaptureMode(ESkyLightCaptureMode aCaptureMode) noexcept
	{
		if (m_CaptureMode == aCaptureMode)
			return;

		m_CaptureMode = aCaptureMode;
		m_EntityManager->AddOrReplace<DirtyRenderState>(m_Self);
		NOTIFY_PROPERTY_CHANGED(m_CaptureMode);
	}

	void SkyLightComponent::SetPrimaryEnvironment(const AssetHandle& aHandle) noexcept
	{
		RLS_ASSERT(aHandle.Type == Environment::StaticType(), "[SkyLightComponent::SetPrimaryEnvironment]: Invalid asset type.");

		if (m_PrimaryEnvironmentHandle == aHandle)
			return;

		DetachPrimaryEnvironment();
		m_PrimaryEnvironmentHandle = aHandle;
		ConnectPrimaryEnvironment();
		
		m_EntityManager->AddOrReplace<DirtyRenderState>(m_Self);
		NOTIFY_PROPERTY_CHANGED(m_PrimaryEnvironmentHandle);
	}

	void SkyLightComponent::SetIntensity(float aIntensity) noexcept
	{
		if (Math::AreValuesClose(m_Intensity, aIntensity))
			return;

		m_Intensity = aIntensity;
		m_EntityManager->AddOrReplace<DirtyRenderState>(m_Self);
		NOTIFY_PROPERTY_CHANGED(m_Intensity);
	}

	void SkyLightComponent::SetLowerHemisphereColor(const Color& aColor) noexcept
	{
		if (m_LowerHemisphereColor == aColor)
			return;

		m_LowerHemisphereColor = aColor;
		m_EntityManager->AddOrReplace<DirtyRenderState>(m_Self);
		NOTIFY_PROPERTY_CHANGED(m_LowerHemisphereColor);
	}

	void SkyLightComponent::SetLowerHemisphereMode(ESkyLightLowerHemisphereMode aMode) noexcept
	{
		if (m_LowerHemisphereMode == aMode)
			return;

		m_LowerHemisphereMode = aMode;
		m_EntityManager->AddOrReplace<DirtyRenderState>(m_Self);
		NOTIFY_PROPERTY_CHANGED(m_LowerHemisphereMode);
	}

	void SkyLightComponent::SetRadianceMapSize(uint32 aSize) noexcept
	{
		uint32 newSize = Math::Min(Math::Max(aSize, MIN_RADIANCE_MAP_SIZE), MAX_RADIANCE_MAP_SIZE);
		newSize = Math::NearestPowerOfTwo(newSize);
		if (newSize == m_RadianceMapSize)
			return;

		m_RadianceMapSize = newSize;
		m_EntityManager->AddOrReplace<DirtyRenderState>(m_Self);
		NOTIFY_PROPERTY_CHANGED(m_RadianceMapSize);
	}

	void SkyLightComponent::SetRealtimeMipsPerFrame(uint32 aNumMips) noexcept
	{
		if (m_RealtimeMipsPerFrame == aNumMips)
			return;

		m_RealtimeMipsPerFrame = aNumMips;
		m_EntityManager->AddOrReplace<DirtyRenderState>(m_Self);
		NOTIFY_PROPERTY_CHANGED(m_RealtimeMipsPerFrame);
	}

	void SkyLightComponent::SetTintColor(const Color& aTintColor) noexcept
	{
		if (m_TintColor == aTintColor)
			return;

		m_TintColor = aTintColor;
		m_EntityManager->AddOrReplace<DirtyRenderState>(m_Self);
		NOTIFY_PROPERTY_CHANGED(m_TintColor);
	}

	void SkyLightComponent::ConnectBlendEnvironment() noexcept
	{
		if (!m_BlendEnvironmentHandle.IsValid())
			return;

		Ref<Environment> pBlendEnvironment = AssetManager::Get<Environment>(m_BlendEnvironmentHandle);
		pBlendEnvironment->OnDestroy.Connect(this, &SkyLightComponent::OnBlendEnvironmentAssetDestroy);
		pBlendEnvironment->OnPropertyChanged.Connect(this, &SkyLightComponent::OnEnvironmentAssetPropertyChanged);
	}

	void SkyLightComponent::ConnectPrimaryEnvironment() noexcept
	{
		if (!m_PrimaryEnvironmentHandle.IsValid())
			return;

		Ref<Environment> pPrimaryEnvironment = AssetManager::Get<Environment>(m_PrimaryEnvironmentHandle);
		pPrimaryEnvironment->OnDestroy.Connect(this, &SkyLightComponent::OnPrimaryEnvironmentAssetDestroy);
		pPrimaryEnvironment->OnPropertyChanged.Connect(this, &SkyLightComponent::OnEnvironmentAssetPropertyChanged);
	}

	void SkyLightComponent::DetachBlendEnvironment() noexcept
	{
		if (!m_BlendEnvironmentHandle.IsValid())
			return;

		Ref<Environment> pBlendEnvironment = AssetManager::Get<Environment>(m_BlendEnvironmentHandle);
		pBlendEnvironment->OnDestroy.Detach(this);
		pBlendEnvironment->OnPropertyChanged.Detach(this);
	}

	void SkyLightComponent::DetachPrimaryEnvironment() noexcept
	{
		if (!m_PrimaryEnvironmentHandle.IsValid())
			return;

		Ref<Environment> pPrimaryEnvironment = AssetManager::Get<Environment>(m_PrimaryEnvironmentHandle);
		pPrimaryEnvironment->OnDestroy.Detach(this);
		pPrimaryEnvironment->OnPropertyChanged.Detach(this);
	}

	void SkyLightComponent::OnBlendEnvironmentAssetDestroy(MAYBE_UNUSED IAsset* aAsset) noexcept
	{
		RemoveBlendEnvironment();
	}

	void SkyLightComponent::OnPrimaryEnvironmentAssetDestroy(MAYBE_UNUSED IAsset* aAsset) noexcept
	{
		RemovePrimaryEnvironment();
	}

	void SkyLightComponent::OnEnvironmentAssetPropertyChanged(MAYBE_UNUSED IAsset* aAsset, MAYBE_UNUSED uint64 aProperty) noexcept
	{
		m_EntityManager->AddOrReplace<DirtyRenderState>(m_Self);
	}
}