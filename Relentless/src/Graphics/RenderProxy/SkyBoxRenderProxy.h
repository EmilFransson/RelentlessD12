#pragma once

#include "Assets/AssetMeta.h"
#include "Assets/CoreTypes/Environment.h"

#include "ECS/Components/SkyBoxComponent.h"

#include "Graphics/RHI/Texture.h"

namespace Relentless
{
	struct SkyBoxRenderProxy
	{
		Ref<Texture> EnvironmentMapA = nullptr;
		Ref<Texture> EnvironmentMapB = nullptr;
		Quaternion WorldRotation = Quaternion::Identity;
		Color TintColor = Colors::White;
		uint32 ID = 0xFFFFFFFF;
		float Intensity = 1.0f;
		float LodBias = 0.0f;
		float BlendFactor = 0.0f;
		bool IsActive = false;
	};
}