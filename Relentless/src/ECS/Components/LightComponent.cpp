#include "LightComponent.h"

namespace Relentless
{
	void DirectionalLightComponent::CopyFrom(const DirectionalLightComponent& aOtherComponent, entity aThisEntity, EntityManager& aEntityManager)
	{
		m_Color = aOtherComponent.m_Color;
		m_Intensity = aOtherComponent.m_Intensity;
		m_Temperature = aOtherComponent.m_Temperature;
		m_UseTemperature = aOtherComponent.m_UseTemperature;

		this->m_Self = aThisEntity;
		this->m_EntityManager = &aEntityManager;
		this->m_EntityManager->AddOrReplace<DirtyRenderState>(this->m_Self);
	}

	void DirectionalLightComponent::OnBound() noexcept
	{
		this->m_EntityManager->AddOrReplace<DirtyRenderState>(this->m_Self);
	}

	void DirectionalLightComponent::SetIntensity(float aWatts, float aEfficiency) noexcept
	{
		SetIntensityLux(aWatts * 683.0f * aEfficiency);
	}

	void DirectionalLightComponent::SetIntensityLux(float aLuxValue) noexcept
	{
		if (Math::AreValuesClose(m_Intensity, aLuxValue))
			return;

		m_Intensity = aLuxValue;
		this->m_EntityManager->AddOrReplace<DirtyRenderState>(this->m_Self);
		NOTIFY_PROPERTY_CHANGED(m_Intensity);
	}

	void PointLightComponent::CopyFrom(const PointLightComponent& aOtherComponent, entity aThisEntity, EntityManager& aEntityManager)
	{
		m_Color = aOtherComponent.m_Color;
		m_Intensity = aOtherComponent.m_Intensity;
		m_Temperature = aOtherComponent.m_Temperature;
		m_UseTemperature = aOtherComponent.m_UseTemperature;
		m_AttenuationRadius = aOtherComponent.m_AttenuationRadius;

		this->m_Self = aThisEntity;
		this->m_EntityManager = &aEntityManager;
		this->m_EntityManager->AddOrReplace<DirtyRenderState>(this->m_Self);
	}

	float PointLightComponent::GetAttenuationRadius() const noexcept
	{
		return m_AttenuationRadius;
	}

	void PointLightComponent::OnBound() noexcept
	{
		this->m_EntityManager->AddOrReplace<DirtyRenderState>(this->m_Self);
	}

	void PointLightComponent::SetAttenuationRadius(float aRadius) noexcept
	{
		if (Math::AreValuesClose(m_AttenuationRadius, aRadius))
			return;

		m_AttenuationRadius = aRadius;
		this->m_EntityManager->AddOrReplace<DirtyRenderState>(this->m_Self);
		NOTIFY_PROPERTY_CHANGED(m_AttenuationRadius);
	}

	void PointLightComponent::SetIntensity(float aWatts, float aEfficiency) noexcept
	{
		SetIntensityLumen(aWatts * 683.0f * aEfficiency);
	}

	void PointLightComponent::SetIntensityCandela(float aCandelaValue) noexcept
	{
		if (Math::AreValuesClose(m_Intensity, aCandelaValue))
			return;

		m_Intensity = aCandelaValue;
		m_EntityManager->AddOrReplace<DirtyRenderState>(this->m_Self);
		NOTIFY_PROPERTY_CHANGED(m_Intensity);
	}

	void PointLightComponent::SetIntensityLumen(float aLumenValue) noexcept
	{
		SetIntensityCandela(Math::Photometry::LumenToCandela_Point(aLumenValue));
	}

	void SpotLightComponent::CopyFrom(const SpotLightComponent& aOtherComponent, entity aThisEntity, EntityManager& aEntityManager)
	{
		m_Color = aOtherComponent.m_Color;
		m_Intensity = aOtherComponent.m_Intensity;
		m_Temperature = aOtherComponent.m_Temperature;
		m_UseTemperature = aOtherComponent.m_UseTemperature;
		m_AttenuationRadius = aOtherComponent.m_AttenuationRadius;
		m_InnerConeAngle = aOtherComponent.m_InnerConeAngle;
		m_OuterConeAngle = aOtherComponent.m_OuterConeAngle;

		this->m_Self = aThisEntity;
		this->m_EntityManager = &aEntityManager;
		this->m_EntityManager->AddOrReplace<DirtyRenderState>(this->m_Self);
	}

	float SpotLightComponent::GetAttenuationRadius() const noexcept
	{
		return m_AttenuationRadius;
	}

	float SpotLightComponent::GetInnerConeAngleDegrees() const noexcept
	{
		return Math::RadToDeg(m_InnerConeAngle);
	}

	float SpotLightComponent::GetInnerConeAngleRadians() const noexcept
	{
		return m_InnerConeAngle;
	}

	float SpotLightComponent::GetOuterConeAngleDegrees() const noexcept
	{
		return Math::RadToDeg(m_OuterConeAngle);
	}

	float SpotLightComponent::GetOuterConeAngleRadians() const noexcept
	{
		return m_OuterConeAngle;
	}

	void SpotLightComponent::OnBound() noexcept
	{
		this->m_EntityManager->AddOrReplace<DirtyRenderState>(this->m_Self);
	}

	void SpotLightComponent::SetAttenuationRadius(float aRadius) noexcept
	{
		if (Math::AreValuesClose(m_AttenuationRadius, aRadius))
			return;

		m_AttenuationRadius = aRadius;
		m_EntityManager->AddOrReplace<DirtyRenderState>(this->m_Self);
		NOTIFY_PROPERTY_CHANGED(m_AttenuationRadius);
	}

	void SpotLightComponent::SetIntensity(float aWatts, float aEfficiency) noexcept
	{
		SetIntensityLumen(aWatts * 683.0f * aEfficiency);
	}

	void SpotLightComponent::SetIntensityCandela(float aCandelaValue) noexcept
	{
		if (Math::AreValuesClose(m_Intensity, aCandelaValue))
			return;

		m_Intensity = aCandelaValue;
		m_EntityManager->AddOrReplace<DirtyRenderState>(this->m_Self);
		NOTIFY_PROPERTY_CHANGED(m_Intensity);
	}

	void SpotLightComponent::SetIntensityLumen(float aLumenValue) noexcept
	{
		SetIntensityCandela(Math::Photometry::LumenToCandela_Spot(aLumenValue, m_OuterConeAngle));
	}

	void SpotLightComponent::SetInnerConeAngleDegrees(float aAngleDegrees) noexcept
	{
		const float innerConeAngle = Math::DegToRad(aAngleDegrees);
		if (Math::AreValuesClose(m_InnerConeAngle, innerConeAngle))
			return;

		m_InnerConeAngle = innerConeAngle;
		if (m_InnerConeAngle > m_OuterConeAngle)
		{
			std::swap(m_InnerConeAngle, m_OuterConeAngle);
			NOTIFY_PROPERTY_CHANGED(m_OuterConeAngle);
		}

		m_EntityManager->AddOrReplace<DirtyRenderState>(this->m_Self);
		NOTIFY_PROPERTY_CHANGED(m_InnerConeAngle);
	}

	void SpotLightComponent::SetOuterConeAngleDegrees(float aAngleDegrees) noexcept
	{
		const float outerConeAngle = Math::DegToRad(aAngleDegrees);
		if (Math::AreValuesClose(m_OuterConeAngle, outerConeAngle))
			return;

		m_OuterConeAngle = outerConeAngle;
		if (m_OuterConeAngle < m_InnerConeAngle)
		{
			std::swap(m_OuterConeAngle, m_InnerConeAngle);
			NOTIFY_PROPERTY_CHANGED(m_InnerConeAngle);
		}
	
		m_EntityManager->AddOrReplace<DirtyRenderState>(this->m_Self);
		NOTIFY_PROPERTY_CHANGED(m_OuterConeAngle);
	}
}