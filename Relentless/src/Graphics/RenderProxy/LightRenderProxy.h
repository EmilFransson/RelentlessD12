#pragma once 
#include "ECS/Components/LightComponent.h"

namespace Relentless
{
	struct LightRenderProxy
	{
		Vector3 Color = Vector3::Zero;
		Vector3 Direction = Vector3::Zero;
		Vector3 Position = Vector3::Zero;
		uint32 ID = 0xFFFFFFFF;
		float Intensity = 0.0f;
		float Temperature = 0.0f;
		float AttenuationRadius = 0.0f;
		float InnerConeAngle = 0.0f;
		float OuterConeAngle = 0.0f;
		ELightType LightType;
		bool IsEnabled = false;
	};
}