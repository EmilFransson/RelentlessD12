#pragma once 
#include "ECS/Components/LightComponent.h"

namespace Relentless
{
	struct LightRenderProxy
	{
		Matrix WorldMatrix			= Matrix::Identity;
		Vector3 Color				= Vector3::Zero;
		Vector3 Direction			= Vector3::Zero;
		Vector3 Position			= Vector3::Zero;
		uint32 ID					= 0xFFFFFFFF;
		uint32 NumCascades			= 4u;
		ELightChannel ChannelMask	= ELightChannel::Default;
		float Intensity				= 0.0f;
		float Temperature			= 0.0f;
		float AttenuationRadius		= 0.0f;
		float InnerConeAngle		= 0.0f;
		float OuterConeAngle		= 0.0f;
		float OuterConeAngleRadians = 0.0f;
		float ShadowAmount			= 1.0f;
		float ShadowResolutionScale = 1.0f;
		float ShadowBias			= 0.0f;
		float ShadowSlopeBias		= 0.0f;
		float CascadeDistribution	= 0.85f;
		ELightType LightType        = ELightType::Directional;
		bool IsEnabled				= false;
		bool CastShadows			= true;
	};
}