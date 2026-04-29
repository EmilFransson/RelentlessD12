#pragma once
#include "ECS/Component.h"

#include "Scene/Scene.h"

#include "Utility/StringUtils.h"

namespace Relentless
{
	enum class ELightType : uint8 { Directional = 0, Point, Spot };

	template<typename LightType>
	struct LightBaseComponent : public ManagedComponent<LightType>
	{
	public:
		struct DirtyRenderState {};

		NO_DISCARD const Color& GetColor() const noexcept;
		NO_DISCARD float GetIntensity() const noexcept;
		NO_DISCARD float GetTemperature() const noexcept;
		NO_DISCARD bool IsUsingTemperature() const noexcept;

		void SetColor(const Color& aColor) noexcept;
		void SetTemperature(float aTemperature) noexcept;
		void SetUseTemperature(bool aUseTemperature) noexcept;
	protected:
		Color m_Color = Colors::Normalize(255.0f, 244.0f, 214.0f, 255.0f);
		float m_Intensity = 8.0f;
		float m_Temperature = 6'500.0f;
		bool m_UseTemperature = false;
	};

	template<typename LightType>
	const Color& LightBaseComponent<LightType>::GetColor() const noexcept
	{
		return m_Color;
	}

	template<typename LightType>
	float LightBaseComponent<LightType>::GetIntensity() const noexcept
	{
		return m_Intensity;
	}

	template<typename LightType>
	float LightBaseComponent<LightType>::GetTemperature() const noexcept
	{
		return m_Temperature;
	}

	template<typename LightType>
	bool LightBaseComponent<LightType>::IsUsingTemperature() const noexcept
	{
		return m_UseTemperature;
	}

	template<typename LightType>
	void LightBaseComponent<LightType>::SetColor(const Color& aColor) noexcept
	{
		if (m_Color == aColor)
			return;

		m_Color = aColor;
		this->m_EntityManager->template AddOrReplace<DirtyRenderState>(this->m_Self);
		NOTIFY_PROPERTY_CHANGED(m_Color);
	}

	template<typename LightType>
	void LightBaseComponent<LightType>::SetTemperature(float aTemperature) noexcept
	{
		if (Math::AreValuesClose(m_Temperature, aTemperature))
			return;

		m_Temperature = aTemperature;
		this->m_EntityManager->template AddOrReplace<DirtyRenderState>(this->m_Self);
		NOTIFY_PROPERTY_CHANGED(m_Temperature);
	}

	template<typename LightType>
	void LightBaseComponent<LightType>::SetUseTemperature(bool aUseTemperature) noexcept
	{
		if (m_UseTemperature == aUseTemperature)
			return;

		m_UseTemperature = aUseTemperature;
		this->m_EntityManager->template AddOrReplace<DirtyRenderState>(this->m_Self);
		NOTIFY_PROPERTY_CHANGED(m_UseTemperature);
	}

	struct RLS_API DirectionalLightComponent : public LightBaseComponent<DirectionalLightComponent>
	{
		virtual void CopyFrom(const DirectionalLightComponent& aOtherComponent, entity aThisEntity, EntityManager& aEntityManager) override final;

		void OnBound() noexcept override final;
		
		void SetIntensity(float aWatts, float aEfficiency) noexcept;
		void SetIntensityLux(float aLuxValue) noexcept;
	};

	struct RLS_API PointLightComponent : public LightBaseComponent<PointLightComponent>
	{
		virtual void CopyFrom(const PointLightComponent& aOtherComponent, entity aThisEntity, EntityManager& aEntityManager) override final;

		NO_DISCARD float GetAttenuationRadius() const noexcept;
		
		void OnBound() noexcept override final;

		void SetAttenuationRadius(float aRadius) noexcept;
		void SetIntensity(float aWatts, float aEfficiency) noexcept;
		void SetIntensityCandela(float aCandelaValue) noexcept;
		void SetIntensityLumen(float aLumenValue) noexcept;
	private:
		float m_AttenuationRadius = 10.0f;
	};

	struct RLS_API SpotLightComponent : public LightBaseComponent<SpotLightComponent>
	{
		virtual void CopyFrom(const SpotLightComponent& aOtherComponent, entity aThisEntity, EntityManager& aEntityManager) override final;

		NO_DISCARD float GetAttenuationRadius() const noexcept;
		NO_DISCARD float GetInnerConeAngleDegrees() const noexcept;
		NO_DISCARD float GetInnerConeAngleRadians() const noexcept;
		NO_DISCARD float GetOuterConeAngleDegrees() const noexcept;
		NO_DISCARD float GetOuterConeAngleRadians() const noexcept;

		void OnBound() noexcept override final;

		void SetAttenuationRadius(float aRadius) noexcept;
		void SetIntensity(float aWatts, float aEfficiency) noexcept;
		void SetIntensityCandela(float aCandelaValue) noexcept;
		void SetIntensityLumen(float aLumenValue) noexcept;
		void SetInnerConeAngleDegrees(float aAngleDegrees) noexcept;
		void SetOuterConeAngleDegrees(float aAngleDegrees) noexcept;
	private:
		float m_AttenuationRadius = 10.0f;
		float m_InnerConeAngle = 0.0f;
		float m_OuterConeAngle = Math::DegToRad(44.0f);
	};
}