#pragma once

#include "Assets/CoreTypes/Environment.h"

#include "ECS/Components/SkyLightComponent.h"

#include "Graphics/RHI/Texture.h"

namespace Relentless
{
	struct SkyLightRenderProxy
	{
		Color LowerHemisphereColor = Colors::Black;
		Color TintColor = Colors::White;
		Color PrimaryEnvironmentColor = Colors::Black;
		Color BlendEnvironmentColor = Colors::Black;
		Quaternion WorldRotation = Quaternion::Identity;
		Ref<Texture> PrimaryEnvironmentMap = nullptr;
		Ref<Texture> BlendEnvironmentMap = nullptr;
		uint32 RadianceMapSize = 256u;
		uint32 RealtimeMipsPerFrame = 1u;
		float Intensity = 1.0f;
		float BlendFactor = 0.0f;
		uint32 ID = 0xFFFFFFFF;
		ESkyLightCaptureMode CaptureMode = ESkyLightCaptureMode::Static;
		ESkyLightLowerHemisphereMode LowerHemisphereMode = ESkyLightLowerHemisphereMode::Environment;
		EEnvironmentSourceType PrimaryEnvironmentSourceType = EEnvironmentSourceType::Cubemap;
		EEnvironmentSourceType BlendEnvironmentSourceType = EEnvironmentSourceType::Cubemap;
		bool IsActive = false;
	};
}