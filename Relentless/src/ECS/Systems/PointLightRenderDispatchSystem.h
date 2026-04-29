#pragma once
#include "ECS/ISystem.h"

namespace Relentless
{
	class PointLightRenderDispatchSystem : public ISystem
	{
		void Execute(SceneState& aSceneState) noexcept override final;
	};
}