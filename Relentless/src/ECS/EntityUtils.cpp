#include "EntityUtils.h"

namespace Relentless::EntityUtils
{
	void ConvertLightType(entity aEntity, EntityManager& aEntityManager, Relentless::ELightType aFromLightType, Relentless::ELightType aToLightType) noexcept
	{
		if (aFromLightType == aToLightType)
			return;

		if (aFromLightType == Relentless::ELightType::Directional)
		{
			const DirectionalLightComponent& directionalLightComponent = aEntityManager.Get<DirectionalLightComponent>(aEntity);

			if (aToLightType == Relentless::ELightType::Spot)
			{
				SpotLightComponent& spotLightComponent = aEntityManager.Add<SpotLightComponent>(aEntity);
				spotLightComponent.SetColor(directionalLightComponent.GetColor());
				spotLightComponent.SetChannelEnabled(ELightChannel::All, false);
				spotLightComponent.SetChannelEnabled(directionalLightComponent.GetChannel(), true);
				spotLightComponent.SetIntensityCandela(Math::Min(160.0f, directionalLightComponent.GetIntensity()));
				spotLightComponent.SetTemperature(directionalLightComponent.GetTemperature());
				spotLightComponent.SetUseTemperature(directionalLightComponent.IsUsingTemperature());
				spotLightComponent.SetCastShadows(directionalLightComponent.IsCastingShadows());
				spotLightComponent.SetShadowAmount(directionalLightComponent.GetShadowAmount());
				spotLightComponent.SetShadowResolutionScale(directionalLightComponent.GetShadowResolutionScale());
				spotLightComponent.SetShadowBias(directionalLightComponent.GetShadowBias());
				spotLightComponent.SetShadowSlopeBias(directionalLightComponent.GetShadowSlopeBias());
			}
			else if (aToLightType == Relentless::ELightType::Point)
			{
				PointLightComponent& pointLightComponent = aEntityManager.Add<PointLightComponent>(aEntity);
				pointLightComponent.SetColor(directionalLightComponent.GetColor());
				pointLightComponent.SetChannelEnabled(ELightChannel::All, false);
				pointLightComponent.SetChannelEnabled(directionalLightComponent.GetChannel(), true);
				pointLightComponent.SetIntensityCandela(Math::Min(160.0f, directionalLightComponent.GetIntensity()));
				pointLightComponent.SetTemperature(directionalLightComponent.GetTemperature());
				pointLightComponent.SetUseTemperature(directionalLightComponent.IsUsingTemperature());
				pointLightComponent.SetCastShadows(directionalLightComponent.IsCastingShadows());
				pointLightComponent.SetShadowAmount(directionalLightComponent.GetShadowAmount());
				pointLightComponent.SetShadowResolutionScale(directionalLightComponent.GetShadowResolutionScale());
				pointLightComponent.SetShadowBias(directionalLightComponent.GetShadowBias());
				pointLightComponent.SetShadowSlopeBias(directionalLightComponent.GetShadowSlopeBias());
			}

			aEntityManager.Remove<DirectionalLightComponent>(aEntity);
		}
		else if (aFromLightType == Relentless::ELightType::Point)
		{
			const PointLightComponent& pointLightComponent = aEntityManager.Get<PointLightComponent>(aEntity);

			if (aToLightType == Relentless::ELightType::Spot)
			{
				SpotLightComponent& spotLightComponent = aEntityManager.Add<SpotLightComponent>(aEntity);
				spotLightComponent.SetAttenuationRadius(pointLightComponent.GetAttenuationRadius());
				spotLightComponent.SetColor(pointLightComponent.GetColor());
				spotLightComponent.SetChannelEnabled(ELightChannel::All, false);
				spotLightComponent.SetChannelEnabled(pointLightComponent.GetChannel(), true);
				spotLightComponent.SetIntensityCandela(Math::Min(160.0f, pointLightComponent.GetIntensity()));
				spotLightComponent.SetTemperature(pointLightComponent.GetTemperature());
				spotLightComponent.SetUseTemperature(pointLightComponent.IsUsingTemperature());
				spotLightComponent.SetCastShadows(pointLightComponent.IsCastingShadows());
				spotLightComponent.SetShadowAmount(pointLightComponent.GetShadowAmount());
				spotLightComponent.SetShadowResolutionScale(pointLightComponent.GetShadowResolutionScale());
				spotLightComponent.SetShadowBias(pointLightComponent.GetShadowBias());
				spotLightComponent.SetShadowSlopeBias(pointLightComponent.GetShadowSlopeBias());
			}
			else if (aToLightType == Relentless::ELightType::Directional)
			{
				DirectionalLightComponent& directionalLightComponent = aEntityManager.Add<DirectionalLightComponent>(aEntity);
				directionalLightComponent.SetColor(pointLightComponent.GetColor());
				directionalLightComponent.SetChannelEnabled(ELightChannel::All, false);
				directionalLightComponent.SetChannelEnabled(pointLightComponent.GetChannel(), true);
				directionalLightComponent.SetIntensityLux(pointLightComponent.GetIntensity());
				directionalLightComponent.SetTemperature(pointLightComponent.GetTemperature());
				directionalLightComponent.SetUseTemperature(pointLightComponent.IsUsingTemperature());
				directionalLightComponent.SetCastShadows(pointLightComponent.IsCastingShadows());
				directionalLightComponent.SetShadowAmount(pointLightComponent.GetShadowAmount());
				directionalLightComponent.SetShadowResolutionScale(pointLightComponent.GetShadowResolutionScale());
				directionalLightComponent.SetShadowBias(pointLightComponent.GetShadowBias());
				directionalLightComponent.SetShadowSlopeBias(pointLightComponent.GetShadowSlopeBias());
			}

			aEntityManager.Remove<PointLightComponent>(aEntity);
		}
		else //Spot
		{
			const SpotLightComponent& spotLightComponent = aEntityManager.Get<SpotLightComponent>(aEntity);

			if (aToLightType == Relentless::ELightType::Point)
			{
				PointLightComponent& pointLightComponent = aEntityManager.Add<PointLightComponent>(aEntity);
				pointLightComponent.SetAttenuationRadius(spotLightComponent.GetAttenuationRadius());
				pointLightComponent.SetColor(spotLightComponent.GetColor());
				pointLightComponent.SetChannelEnabled(ELightChannel::All, false);
				pointLightComponent.SetChannelEnabled(spotLightComponent.GetChannel(), true);
				pointLightComponent.SetIntensityCandela(Math::Min(160.0f, spotLightComponent.GetIntensity()));
				pointLightComponent.SetTemperature(spotLightComponent.GetTemperature());
				pointLightComponent.SetUseTemperature(spotLightComponent.IsUsingTemperature());
				pointLightComponent.SetCastShadows(spotLightComponent.IsCastingShadows());
				pointLightComponent.SetShadowAmount(spotLightComponent.GetShadowAmount());
				pointLightComponent.SetShadowResolutionScale(spotLightComponent.GetShadowResolutionScale());
				pointLightComponent.SetShadowBias(spotLightComponent.GetShadowBias());
				pointLightComponent.SetShadowSlopeBias(spotLightComponent.GetShadowSlopeBias());
			}
			else if (aToLightType == Relentless::ELightType::Directional)
			{
				DirectionalLightComponent& directionalLightComponent = aEntityManager.Add<DirectionalLightComponent>(aEntity);
				directionalLightComponent.SetColor(spotLightComponent.GetColor());
				directionalLightComponent.SetChannelEnabled(ELightChannel::All, false);
				directionalLightComponent.SetChannelEnabled(spotLightComponent.GetChannel(), true);
				directionalLightComponent.SetIntensityLux(spotLightComponent.GetIntensity());
				directionalLightComponent.SetTemperature(spotLightComponent.GetTemperature());
				directionalLightComponent.SetUseTemperature(spotLightComponent.IsUsingTemperature());
				directionalLightComponent.SetCastShadows(spotLightComponent.IsCastingShadows());
				directionalLightComponent.SetShadowAmount(spotLightComponent.GetShadowAmount());
				directionalLightComponent.SetShadowResolutionScale(spotLightComponent.GetShadowResolutionScale());
				directionalLightComponent.SetShadowBias(spotLightComponent.GetShadowBias());
				directionalLightComponent.SetShadowSlopeBias(spotLightComponent.GetShadowSlopeBias());
			}

			aEntityManager.Remove<SpotLightComponent>(aEntity);
		}
	}
}