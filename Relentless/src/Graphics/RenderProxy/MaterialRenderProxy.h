#pragma once
#include "Assets/CoreTypes/Material.h"

#include "Graphics/RHI/Texture.h"

namespace Relentless
{
	struct MaterialRenderProxy
	{
		UUID ID								= {};

		Vector4 AlbedoColor					= Vector4::Zero;
		Vector4 EmissiveColor				= Vector4::Zero;
		Vector2 TilingFactor				= Vector2::One;
		Vector2 Offset						= Vector2::Zero;

		float Metallic						= 0.0f;
		float EmissionIntensity				= 0.0f;
		float Roughness						= 0.0f;
		float DisplacementIntensity			= 0.0f;
		float AmbientOcclusionIntensity		= 0.0f;
		EBlendMode BlendMode				= EBlendMode::Opaque;
		bool IsTwoSided						= false;

		Ref<Texture> AlbedoMap				= nullptr;
		Ref<Texture> NormalMap				= nullptr;
		Ref<Texture> DisplacementMap		= nullptr;
		Ref<Texture> RoughnessMap			= nullptr;
		Ref<Texture> MetalnessMap			= nullptr;
		Ref<Texture> EmissionMap			= nullptr;
		Ref<Texture> AmbientOcclusionMap	= nullptr;
	};
}