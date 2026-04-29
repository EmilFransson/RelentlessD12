#pragma once
#include "ECS/ISystem.h"

namespace Relentless
{
	class SpotLightRenderDispatchSystem : public ISystem
	{
		void Execute(SceneState& aSceneState) noexcept override final;
	};
}