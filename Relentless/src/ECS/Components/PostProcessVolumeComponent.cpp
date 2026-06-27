#include "PostProcessVolumeComponent.h"

#include "Assets/AssetManager.h"
#include "Assets/CoreTypes/Texture2D.h"

#include "ECS/EntityManager.h"

#include "Utility/StringUtils.h"

namespace Relentless
{
	AmbientOcclusionSettings::AmbientOcclusionSettings(PostProcessVolumeComponent* aOwner) noexcept
		:SubObject<PostProcessVolumeComponent>{ aOwner }
	{
	}

	float AmbientOcclusionSettings::GetBias() const noexcept
	{
		return m_Bias;
	}

	EAmbientOcclusionBlurRadius AmbientOcclusionSettings::GetBlurRadius() const noexcept
	{
		return m_BlurRadius;
	}

	float AmbientOcclusionSettings::GetBlurSharpness() const noexcept
	{
		return m_BlurSharpness;
	}

	EAmbientOcclusionDepthPrecision AmbientOcclusionSettings::GetDepthPrecision() const noexcept
	{
		return m_DepthPrecision;
	}

	float AmbientOcclusionSettings::GetPowerExponent() const noexcept
	{
		return m_PowerExponent;
	}

	float AmbientOcclusionSettings::GetRadius() const noexcept
	{
		return m_Radius;
	}

	EAmbientOcclusionStepcount AmbientOcclusionSettings::GetStepCount() const noexcept
	{
		return m_StepCount;
	}

	bool AmbientOcclusionSettings::IsBlurEnabled() const noexcept
	{
		return m_BlurEnabled;
	}

	bool AmbientOcclusionSettings::IsEnabled() const noexcept
	{
		return m_IsEnabled;
	}

	void AmbientOcclusionSettings::SetBias(float aBias) noexcept
	{
		if (Math::AreValuesClose(m_Bias, aBias))
			return;

		m_Bias = aBias;
		NOTIFY_NESTED_PROPERTY_CHANGED("m_AmbientOcclusionSettings", m_Bias);
	}

	void AmbientOcclusionSettings::SetBlurEnabled(bool aEnabled) noexcept
	{
		if (m_BlurEnabled == aEnabled)
			return;

		m_BlurEnabled = aEnabled;
		NOTIFY_NESTED_PROPERTY_CHANGED("m_AmbientOcclusionSettings", m_BlurEnabled);
	}

	void AmbientOcclusionSettings::SetBlurRadius(EAmbientOcclusionBlurRadius aBlurRadius) noexcept
	{
		if (m_BlurRadius == aBlurRadius)
			return;

		m_BlurRadius = aBlurRadius;
		NOTIFY_NESTED_PROPERTY_CHANGED("m_AmbientOcclusionSettings", m_BlurRadius);
	}

	void AmbientOcclusionSettings::SetBlurSharpness(float aBlurSharpness) noexcept
	{
		if (Math::AreValuesClose(m_BlurSharpness, aBlurSharpness))
			return;

		m_BlurSharpness = aBlurSharpness;
		NOTIFY_NESTED_PROPERTY_CHANGED("m_AmbientOcclusionSettings", m_BlurSharpness);
	}

	void AmbientOcclusionSettings::SetDepthPrecision(EAmbientOcclusionDepthPrecision aDepthPrecision) noexcept
	{
		if (m_DepthPrecision == aDepthPrecision)
			return;

		m_DepthPrecision = aDepthPrecision;
		NOTIFY_NESTED_PROPERTY_CHANGED("m_AmbientOcclusionSettings", m_DepthPrecision);
	}

	void AmbientOcclusionSettings::SetEnabled(bool aEnabled) noexcept
	{
		if (m_IsEnabled == aEnabled)
			return;

		m_IsEnabled = aEnabled;
		NOTIFY_NESTED_PROPERTY_CHANGED("m_AmbientOcclusionSettings", m_IsEnabled);
	}

	void AmbientOcclusionSettings::SetPowerExponent(float aPowerExponent) noexcept
	{
		if (Math::AreValuesClose(m_PowerExponent, aPowerExponent))
			return;

		m_PowerExponent = aPowerExponent;
		NOTIFY_NESTED_PROPERTY_CHANGED("m_AmbientOcclusionSettings", m_PowerExponent);
	}

	void AmbientOcclusionSettings::SetRadius(float aRadius) noexcept
	{
		if (Math::AreValuesClose(m_Radius, aRadius))
			return;

		m_Radius = aRadius;
		NOTIFY_NESTED_PROPERTY_CHANGED("m_AmbientOcclusionSettings", m_Radius);
	}

	void AmbientOcclusionSettings::SetStepCount(EAmbientOcclusionStepcount aStepCount) noexcept
	{
		if (m_StepCount == aStepCount)
			return;

		m_StepCount = aStepCount;
		NOTIFY_NESTED_PROPERTY_CHANGED("m_AmbientOcclusionSettings", m_StepCount);
	}

	BloomSettings::BloomSettings(PostProcessVolumeComponent* aOwner) noexcept
		: SubObject<PostProcessVolumeComponent>{ aOwner }
	{
	}

	BloomSettings::BloomSettings(const BloomSettings& aOther) noexcept
		: SubObject<PostProcessVolumeComponent>{ aOther.m_pOwner }
		, m_DirtMaskHandle{ aOther.m_DirtMaskHandle }
		, m_DirtMaskTint{ aOther.m_DirtMaskTint }
		, m_Intensity{ aOther.m_Intensity }
		, m_DirtMaskIntensity{ aOther.m_DirtMaskIntensity }
	{
		ConnectDirtMask();
	}

	BloomSettings& BloomSettings::operator=(const BloomSettings& aOther) noexcept
	{
		if (this != &aOther)
		{
			DetachDirtMask();

			m_DirtMaskHandle = aOther.m_DirtMaskHandle;
			m_DirtMaskTint = aOther.m_DirtMaskTint;
			m_Intensity = aOther.m_Intensity;
			m_DirtMaskIntensity = aOther.m_DirtMaskIntensity;

			ConnectDirtMask();
		}
		return *this;
	}

	BloomSettings::BloomSettings(BloomSettings&& aOther) noexcept
		: SubObject<PostProcessVolumeComponent>{ std::move(aOther.m_pOwner) }
		, m_DirtMaskHandle{ aOther.m_DirtMaskHandle }
		, m_DirtMaskTint{ aOther.m_DirtMaskTint }
		, m_Intensity{ aOther.m_Intensity }
		, m_DirtMaskIntensity{ aOther.m_DirtMaskIntensity }
	{
		aOther.DetachDirtMask();
		aOther.m_DirtMaskHandle = AssetHandle::INVALID;
		ConnectDirtMask();
	}

	BloomSettings& BloomSettings::operator=(BloomSettings&& aOther) noexcept
	{
		if (this != &aOther)
		{
			DetachDirtMask();
			aOther.DetachDirtMask();
			
			m_pOwner = std::move(aOther.m_pOwner);
			m_DirtMaskHandle = std::move(aOther.m_DirtMaskHandle);
			m_DirtMaskTint = std::move(aOther.m_DirtMaskTint);
			m_Intensity = std::move(aOther.m_Intensity);
			m_DirtMaskIntensity = std::move(aOther.m_DirtMaskIntensity);

			aOther.m_DirtMaskHandle = AssetHandle::INVALID;
			aOther.m_pOwner = nullptr;

			ConnectDirtMask();
		}
		return *this;
	}

	BloomSettings::~BloomSettings() noexcept
	{
		DetachDirtMask();
	}

	Ref<Texture2D> BloomSettings::GetDirtMask() const noexcept
	{
		RLS_ASSERT(m_DirtMaskHandle.IsValid(), "[BloomSettings::GetDirtMask]: Dirt mask handle is invalid.");
		return AssetManager::Get<Texture2D>(m_DirtMaskHandle);
	}

	const AssetHandle& BloomSettings::GetDirtMaskHandle() const noexcept
	{
		return m_DirtMaskHandle;
	}

	float BloomSettings::GetDirtMaskIntensity() const noexcept
	{
		return m_DirtMaskIntensity;
	}

	const Color& BloomSettings::GetDirtMaskTint() const noexcept
	{
		return m_DirtMaskTint;
	}

	float BloomSettings::GetIntensity() const noexcept
	{
		return m_Intensity;
	}

	void BloomSettings::SetDirtMaskIntensity(float aIntensity) noexcept
	{
		if (Math::AreValuesClose(m_DirtMaskIntensity, aIntensity))
			return;

		m_DirtMaskIntensity = aIntensity;
		NOTIFY_NESTED_PROPERTY_CHANGED("m_BloomSettings", m_DirtMaskIntensity);
	}

	void BloomSettings::SetDirtMask(const AssetHandle& aDirtMaskHandle) noexcept
	{
		RLS_ASSERT(aDirtMaskHandle.Type == Texture2D::StaticType(), "[BloomSettings::SetDirtMask]: Asset handle is not of texture2D type.");

		if (m_DirtMaskHandle == aDirtMaskHandle)
			return;

		DetachDirtMask();
		m_DirtMaskHandle = aDirtMaskHandle;
		ConnectDirtMask();

		NOTIFY_NESTED_PROPERTY_CHANGED("m_BloomSettings", m_DirtMaskHandle);
	}

	void BloomSettings::SetDirtMaskTint(const Color& aColor) noexcept
	{
		if (m_DirtMaskTint == aColor)
			return;

		m_DirtMaskTint = aColor;
		NOTIFY_NESTED_PROPERTY_CHANGED("m_BloomSettings", m_DirtMaskTint);
	}

	void BloomSettings::SetIntensity(float aIntensity) noexcept
	{
		if (Math::AreValuesClose(m_Intensity, aIntensity))
			return;

		m_Intensity = aIntensity;
		NOTIFY_NESTED_PROPERTY_CHANGED("m_BloomSettings", m_Intensity);
	}

	void BloomSettings::ConnectDirtMask() noexcept
	{
		if (!m_DirtMaskHandle.IsValid())
			return;

		Ref<Texture2D> pDirtMask = AssetManager::Get<Texture2D>(m_DirtMaskHandle);
		pDirtMask->OnDestroy.Connect(this, &BloomSettings::OnDirtMaskAssetDestroy);
		pDirtMask->OnPropertyChanged.Connect(this, &BloomSettings::OnDirtMaskAssetPropertyChanged);
	}

	void BloomSettings::OnDirtMaskAssetDestroy(MAYBE_UNUSED IAsset* aAsset) noexcept
	{
		RemoveDirtMask();
	}

	void BloomSettings::OnDirtMaskAssetPropertyChanged(MAYBE_UNUSED IAsset* aAsset, MAYBE_UNUSED uint64 aProperty) noexcept
	{
		NOTIFY_NESTED_PROPERTY_CHANGED("m_BloomSettings", m_DirtMaskHandle);
	}

	void BloomSettings::RemoveDirtMask() noexcept
	{
		if (!m_DirtMaskHandle.IsValid())
			return;

		m_DirtMaskHandle = AssetHandle::INVALID;
		NOTIFY_NESTED_PROPERTY_CHANGED("m_BloomSettings", m_DirtMaskHandle);
	}

	bool BloomSettings::HasAssignedDirtMask() const noexcept
	{
		return m_DirtMaskHandle.IsValid();
	}

	void BloomSettings::DetachDirtMask() noexcept
	{
		if (!m_DirtMaskHandle.IsValid())
			return;

		Ref<Texture2D> pDirtMask = AssetManager::Get<Texture2D>(m_DirtMaskHandle);
		pDirtMask->OnDestroy.Detach(this);
		pDirtMask->OnPropertyChanged.Detach(this);
	}

	ExposureSettings::ExposureSettings(PostProcessVolumeComponent* aOwner) noexcept
		: SubObject<PostProcessVolumeComponent>{ aOwner }
	{
	}

	float ExposureSettings::GetCompensation() const noexcept
	{
		return m_Compensation;
	}

	float ExposureSettings::GetMinEV100() const noexcept
	{
		return m_MinEV100;
	}

	float ExposureSettings::GetMaxEV100() const noexcept
	{
		return m_MaxEV100;
	}

	float ExposureSettings::GetSpeedUp() const noexcept
	{
		return m_SpeedUp;
	}

	float ExposureSettings::GetSpeedDown() const noexcept
	{
		return m_SpeedDown;
	}

	float ExposureSettings::GetLowPercent() const noexcept
	{
		return m_LowPercent;
	}

	float ExposureSettings::GetHighPercent() const noexcept
	{
		return m_HighPercent;
	}

	float ExposureSettings::GetHistogramMinEV100() const noexcept
	{
		return m_HistogramMinEV100;
	}

	float ExposureSettings::GetHistogramMaxEV100() const noexcept
	{
		return m_HistogramMaxEV100;
	}

	void ExposureSettings::SetCompensation(float aCompensation) noexcept
	{
		if (Math::AreValuesClose(m_Compensation, aCompensation))
			return;

		m_Compensation = aCompensation;
		NOTIFY_NESTED_PROPERTY_CHANGED("m_ExposureSettings", m_Compensation);
	}

	void ExposureSettings::SetMinEV100(float aMinEV100) noexcept
	{
		if (Math::AreValuesClose(m_MinEV100, aMinEV100))
			return;

		m_MinEV100 = aMinEV100;
		NOTIFY_NESTED_PROPERTY_CHANGED("m_ExposureSettings", m_MinEV100);
	}

	void ExposureSettings::SetMaxEV100(float aMaxEV100) noexcept
	{
		if (Math::AreValuesClose(m_MaxEV100, aMaxEV100))
			return;

		m_MaxEV100 = aMaxEV100;
		NOTIFY_NESTED_PROPERTY_CHANGED("m_ExposureSettings", m_MaxEV100);
	}

	void ExposureSettings::SetSpeedUp(float aSpeedUp) noexcept
	{
		if (Math::AreValuesClose(m_SpeedUp, aSpeedUp))
			return;

		m_SpeedUp = aSpeedUp;
		NOTIFY_NESTED_PROPERTY_CHANGED("m_ExposureSettings", m_SpeedUp);
	}

	void ExposureSettings::SetSpeedDown(float aSpeedDown) noexcept
	{
		if (Math::AreValuesClose(m_SpeedDown, aSpeedDown))
			return;

		m_SpeedDown = aSpeedDown;
		NOTIFY_NESTED_PROPERTY_CHANGED("m_ExposureSettings", m_SpeedDown);
	}

	void ExposureSettings::SetLowPercent(float aLowPercent) noexcept
	{
		if (Math::AreValuesClose(m_LowPercent, aLowPercent))
			return;

		m_LowPercent = aLowPercent;
		NOTIFY_NESTED_PROPERTY_CHANGED("m_ExposureSettings", m_LowPercent);
	}

	void ExposureSettings::SetHighPercent(float aHighPercent) noexcept
	{
		if (Math::AreValuesClose(m_HighPercent, aHighPercent))
			return;

		m_HighPercent = aHighPercent;
		NOTIFY_NESTED_PROPERTY_CHANGED("m_ExposureSettings", m_HighPercent);
	}

	void ExposureSettings::SetHistogramMinEV100(float aHistogramMinEV100) noexcept
	{
		if (Math::AreValuesClose(m_HistogramMinEV100, aHistogramMinEV100))
			return;

		m_HistogramMinEV100 = aHistogramMinEV100;
		NOTIFY_NESTED_PROPERTY_CHANGED("m_ExposureSettings", m_HistogramMinEV100);
	}

	void ExposureSettings::SetHistogramMaxEV100(float aHistogramMaxEV100) noexcept
	{
		if (Math::AreValuesClose(m_HistogramMaxEV100, aHistogramMaxEV100))
			return;

		m_HistogramMaxEV100 = aHistogramMaxEV100;
		NOTIFY_NESTED_PROPERTY_CHANGED("m_ExposureSettings", m_HistogramMaxEV100);
	}

	AmbientOcclusionSettings& PostProcessVolumeComponent::GetAmbientOcclusion() noexcept
	{
		return m_AmbientOcclusionSettings;
	}

	const AmbientOcclusionSettings& PostProcessVolumeComponent::GetAmbientOcclusion() const noexcept
	{
		return m_AmbientOcclusionSettings;
	}

	BloomSettings& PostProcessVolumeComponent::GetBloom() noexcept
	{
		return m_BloomSettings;
	}

	const BloomSettings& PostProcessVolumeComponent::GetBloom() const noexcept
	{
		return m_BloomSettings;
	}

	ExposureSettings& PostProcessVolumeComponent::GetExposure() noexcept
	{
		return m_ExposureSettings;
	}

	const ExposureSettings& PostProcessVolumeComponent::GetExposure() const noexcept
	{
		return m_ExposureSettings;
	}

	void PostProcessVolumeComponent::CopyFrom(const PostProcessVolumeComponent& aOtherComponent, entity aThisEntity, EntityManager& aEntityManager)
	{
		m_AmbientOcclusionSettings = aOtherComponent.m_AmbientOcclusionSettings;
		m_ExposureSettings = aOtherComponent.m_ExposureSettings;
		m_BloomSettings = aOtherComponent.m_BloomSettings;

		InjectSelf();
		
		m_Self = aThisEntity;
		m_EntityManager = &aEntityManager;
		
		m_EntityManager->AddOrReplace<DirtyRenderState>(m_Self);
	}

	bool PostProcessVolumeComponent::HasInfiniteExtent() const noexcept
	{
		return m_InfiniteExtent;
	}

	void PostProcessVolumeComponent::OnBound() noexcept
	{
		InjectSelf();
		m_EntityManager->AddOrReplace<DirtyRenderState>(m_Self);
	}

	void PostProcessVolumeComponent::InjectSelf() noexcept
	{
		m_AmbientOcclusionSettings.m_pOwner = this;
		m_ExposureSettings.m_pOwner = this;
	}

	void PostProcessVolumeComponent::NotifyPropertyChanged(uint64 aPropertyHash) noexcept
	{
		m_EntityManager->AddOrReplace<DirtyRenderState>(m_Self);
		BroadcastPropertyChanged(aPropertyHash);
	}

}