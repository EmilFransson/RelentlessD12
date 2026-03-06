#pragma once
#include <Relentless.h>

namespace Relentless
{
	enum class ETransformSpace : int { Relative = 0, Absolute };
	enum class ELightIntensityType : int { Candelas = 0, Lumens };

	struct EntityDetailsContext
	{
		std::vector<entity> Entities;

		Vector3 CachedEulerWorldRotation = Vector3::Zero;
		Vector3 CachedEulerLocalRotation = Vector3::Zero;
		
		ETransformSpace LocationTransformSpace = ETransformSpace::Relative;
		ETransformSpace RotationTransformSpace = ETransformSpace::Relative;
		ETransformSpace ScaleTransformSpace	= ETransformSpace::Relative;

		ELightIntensityType LightIntensityType = ELightIntensityType::Candelas;

		bool ScaleLocked = true;
		bool ScaleWasLocked = true;

		EntityManager* EntityManager = nullptr;
	};
}