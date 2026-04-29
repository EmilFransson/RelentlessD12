#include "Component.h"

#include "Assets/AssetManager.h"
#include "Assets/CoreTypes/Environment.h"

#include "Scene/Scene.h"

namespace Relentless
{
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
		m_Compensation = aCompensation;
	}

	void ExposureSettings::SetMinEV100(float aMinEV100) noexcept
	{
		m_MinEV100 = aMinEV100;
	}

	void ExposureSettings::SetMaxEV100(float aMaxEV100) noexcept
	{
		m_MaxEV100 = aMaxEV100;
	}

	void ExposureSettings::SetSpeedUp(float aSpeedUp) noexcept
	{
		m_SpeedUp = aSpeedUp;
	}

	void ExposureSettings::SetSpeedDown(float aSpeedDown) noexcept
	{
		m_SpeedDown = aSpeedDown;
	}

	void ExposureSettings::SetLowPercent(float aLowPercent) noexcept
	{
		m_LowPercent = aLowPercent;
	}

	void ExposureSettings::SetHighPercent(float aHighPercent) noexcept
	{
		m_HighPercent = aHighPercent;
	}

	void ExposureSettings::SetHistogramMinEV100(float aHistogramMinEV100) noexcept
	{
		m_HistogramMinEV100 = aHistogramMinEV100;
	}

	void ExposureSettings::SetHistogramMaxEV100(float aHistogramMaxEV100) noexcept
	{
		m_HistogramMaxEV100 = aHistogramMaxEV100;
	}

	ExposureSettings& PostProcessVolumeComponent::GetExposure() noexcept
	{
		return m_ExposureSettings;
	}

	const ExposureSettings& PostProcessVolumeComponent::GetExposure() const noexcept
	{
		return m_ExposureSettings;
	}

	bool PostProcessVolumeComponent::HasInfiniteExtent() const noexcept
	{
		return m_InfiniteExtent;
	}

}
