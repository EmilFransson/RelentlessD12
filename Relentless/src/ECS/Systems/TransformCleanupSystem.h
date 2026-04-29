#pragma once
#include "ECS/ISystem.h"

namespace Relentless
{
	class TransformCleanupSystem : public ISystem
	{
		void Execute(SceneState& aSceneState) noexcept override final;
	};
}