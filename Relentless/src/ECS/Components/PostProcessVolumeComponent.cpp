#include "PostProcessVolumeComponent.h"

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