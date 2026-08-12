#pragma once
#include "ECS/Component.h"

#include "Scene/Scene.h"

#include "Utility/StringUtils.h"

#include "Graphics/Renderer/RenderTypes.h"

namespace Relentless
{
	enum class ELightType : uint8 { Directional = 0, Point, Spot, Sky };

	template<typename LightType>
	struct LightBaseComponent : public ManagedComponent<LightType>
	{
	public:
		struct DirtyRenderState {};

		NO_DISCARD const Color& GetColor() const noexcept;
		NO_DISCARD float GetIntensity() const noexcept;
		NO_DISCARD ELightChannel GetChannel() const noexcept;
		NO_DISCARD float GetTemperature() const noexcept;
		NO_DISCARD float GetShadowAmount() const noexcept;
		NO_DISCARD float GetShadowBias() const noexcept;
		NO_DISCARD float GetShadowResolutionScale() const noexcept;
		NO_DISCARD float GetShadowSlopeBias() const noexcept;

		NO_DISCARD bool HasAnyChannel(ELightChannel aChannels) const noexcept;

		NO_DISCARD bool IsUsingTemperature() const noexcept;
		NO_DISCARD bool IsCastingShadows() const noexcept;

		void SetCastShadows(bool aCastShadows) noexcept;
		void SetColor(const Color& aColor) noexcept;
		void SetChannelEnabled(ELightChannel aChannel, bool aEnabled) noexcept;
		void SetShadowAmount(float aShadowAmount) noexcept;
		void SetShadowBias(float aShadowBias) noexcept;
		void SetShadowResolutionScale(float aShadowResolutionScale) noexcept;
		void SetShadowSlopeBias(float aShadowSlopeBias) noexcept;
		void SetTemperature(float aTemperature) noexcept;
		void SetUseTemperature(bool aUseTemperature) noexcept;
	private:
		void ApplyChannelMask(ELightChannel aNewMask) noexcept;
	protected:
		Color m_Color = Colors::Normalize(255.0f, 244.0f, 214.0f, 255.0f);
		ELightChannel m_LightChannel = ELightChannel::Default;
		float m_Intensity = 8.0f;
		float m_Temperature = 6'500.0f;
		float m_ShadowAmount = 1.0f;
		float m_ShadowResolutionScale = 1.0f;
		float m_ShadowBias = 0.0001f;
		float m_ShadowSlopeBias = 0.0005f;
		bool m_UseTemperature = false;
		bool m_CastsShadows = true;
	};

	template<typename LightType>
	ELightChannel LightBaseComponent<LightType>::GetChannel() const noexcept
	{
		return m_LightChannel;
	}

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
	float LightBaseComponent<LightType>::GetShadowAmount() const noexcept
	{
		return m_ShadowAmount;
	}

	template<typename LightType>
	float LightBaseComponent<LightType>::GetShadowBias() const noexcept
	{
		return m_ShadowBias;
	}

	template<typename LightType>
	float LightBaseComponent<LightType>::GetShadowResolutionScale() const noexcept
	{
		return m_ShadowResolutionScale;
	}

	template<typename LightType>
	float LightBaseComponent<LightType>::GetShadowSlopeBias() const noexcept
	{
		return m_ShadowSlopeBias;
	}

	template<typename LightType>
	float LightBaseComponent<LightType>::GetTemperature() const noexcept
	{
		return m_Temperature;
	}

	template<typename LightType>
	bool LightBaseComponent<LightType>::HasAnyChannel(ELightChannel aChannels) const noexcept
	{
		return EnumHasAnyFlags(m_LightChannel, aChannels);
	}

	template<typename LightType>
	bool LightBaseComponent<LightType>::IsCastingShadows() const noexcept
	{
		return m_CastsShadows;
	}

	template<typename LightType>
	bool LightBaseComponent<LightType>::IsUsingTemperature() const noexcept
	{
		return m_UseTemperature;
	}

	template<typename LightType>
	void LightBaseComponent<LightType>::SetCastShadows(bool aCastShadows) noexcept
	{
		if (m_CastsShadows == aCastShadows)
			return;

		m_CastsShadows = aCastShadows;
		this->m_EntityManager->template AddOrReplace<DirtyRenderState>(this->m_Self);
		NOTIFY_PROPERTY_CHANGED(m_CastsShadows);
	}

	template<typename LightType>
	void LightBaseComponent<LightType>::SetChannelEnabled(ELightChannel aChannel, bool aEnabled) noexcept
	{
		ApplyChannelMask(aEnabled ? m_LightChannel | aChannel : m_LightChannel & ~aChannel); 
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
	void LightBaseComponent<LightType>::SetShadowAmount(float aShadowAmount) noexcept
	{
		const float clampedShadowAmount = Math::Clamp(aShadowAmount, 0.0f, 1.0f);
		if (Math::AreValuesClose(m_ShadowAmount, clampedShadowAmount))
			return;

		m_ShadowAmount = clampedShadowAmount;
		this->m_EntityManager->template AddOrReplace<DirtyRenderState>(this->m_Self);
		NOTIFY_PROPERTY_CHANGED(m_ShadowAmount);
	}

	template<typename LightType>
	void LightBaseComponent<LightType>::SetShadowBias(float aShadowBias) noexcept
	{
		if (Math::AreValuesClose(m_ShadowBias, aShadowBias))
			return;

		m_ShadowBias = aShadowBias;
		this->m_EntityManager->template AddOrReplace<DirtyRenderState>(this->m_Self);
		NOTIFY_PROPERTY_CHANGED(m_ShadowBias);
	}

	template<typename LightType>
	void LightBaseComponent<LightType>::SetShadowResolutionScale(float aShadowResolutionScale) noexcept
	{
		const float clampedShadowResolutionScale = Math::Clamp(aShadowResolutionScale, 0.0f, 8.0f);
		if (Math::AreValuesClose(m_ShadowResolutionScale, clampedShadowResolutionScale))
			return;

		m_ShadowResolutionScale = clampedShadowResolutionScale;
		this->m_EntityManager->template AddOrReplace<DirtyRenderState>(this->m_Self);
		NOTIFY_PROPERTY_CHANGED(m_ShadowResolutionScale);
	}

	template<typename LightType>
	void LightBaseComponent<LightType>::SetShadowSlopeBias(float aShadowSlopeBias) noexcept
	{
		if (Math::AreValuesClose(m_ShadowSlopeBias, aShadowSlopeBias))
			return;

		m_ShadowSlopeBias = aShadowSlopeBias;
		this->m_EntityManager->template AddOrReplace<DirtyRenderState>(this->m_Self);
		NOTIFY_PROPERTY_CHANGED(m_ShadowBias);
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

	template<typename LightType>
	void LightBaseComponent<LightType>::ApplyChannelMask(ELightChannel aNewMask) noexcept
	{
		if (m_LightChannel == aNewMask)
			return;

		m_LightChannel = aNewMask;
		this->m_EntityManager->template AddOrReplace<DirtyRenderState>(this->m_Self);
		NOTIFY_PROPERTY_CHANGED(m_LightChannel);
	}

	struct RLS_API DirectionalLightComponent : public LightBaseComponent<DirectionalLightComponent>
	{
	public:
		virtual void CopyFrom(const DirectionalLightComponent& aOtherComponent, entity aThisEntity, EntityManager& aEntityManager) override final;

		NO_DISCARD float GetCascadeDistribution() const noexcept;
		NO_DISCARD uint32 GetNumCascades() const noexcept;

		void OnBound() noexcept override final;
		
		void SetCascadeDistribution(float aCascadeDistribution) noexcept;
		void SetIntensity(float aWatts, float aEfficiency) noexcept;
		void SetIntensityLux(float aLuxValue) noexcept;
		void SetNumCascades(uint32 aNumCascades) noexcept;
	private:
		float m_CascadeDistribution = 0.85f;
		uint32 m_NumCascades = 4u;
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
