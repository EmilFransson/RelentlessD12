#pragma once
#include "Assets/AssetMeta.h"
#include "Assets/CoreTypes/Environment.h"

#include "Graphics/RHI/Texture.h"

namespace Relentless
{
	struct SkyBoxRenderProxy
	{
		Ref<Texture> EnvironmentMapA = nullptr;
		Ref<Texture> EnvironmentMapB = nullptr;
		Quaternion WorldRotation = Quaternion::Identity;
		Color TintColor = Colors::White;
		Color EnvironmentASolidColor = Colors::Black;
		Color EnvironmentBSolidColor = Colors::Black;
		uint32 ID = 0xFFFFFFFF;
		float Intensity = 1.0f;
		float LodBias = 0.0f;
		float BlendFactor = 0.0f;
		EEnvironmentSourceType EnvironmentASourceType = EEnvironmentSourceType::SolidColor;
		EEnvironmentSourceType EnvironmentBSourceType = EEnvironmentSourceType::SolidColor;
		bool IsActive = false;
	};
}