#pragma once
#include "Graphics/RHI/Texture.h"

namespace Relentless
{
	struct ExponentialHeightFogRenderProxy
	{
		Vector3 InScatteringColor				= Vector3::Zero;
		Vector3 InScatteringTextureColorTint	= Vector3::One;
		uint32 ID								= 0xFFFFFFFF;
		float DensityLayer0						= 0.0f;
		float DensityLayer1						= 0.0f;
		float HeightFallOffLayer0				= 0.0f;
		float HeightFallOffLayer1				= 0.0f;
		float StartDistanceLayer0				= 0.0f;
		float StartDistanceLayer1				= 0.0f;
		float EndDistanceLayer0					= 0.0f;
		float EndDistanceLayer1					= 0.0f;
		float HeightOffsetLayer0				= 0.0f;
		float HeightOffsetLayer1				= 0.0f;
		float MaxOpacity						= 0.0f;
		float FullyDirectionalDistance			= 0.0f;
		float NonDirectionalDistance			= 0.0f;
		float InscatteringColorIntensity		= 0.0f;
		bool UseUniformInscatter				= true;
		bool IsActive							= false;
		Ref<Texture> InScatterTexture			= nullptr;
	};
}