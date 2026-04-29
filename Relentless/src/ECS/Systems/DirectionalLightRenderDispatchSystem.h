#pragma once
#include "ECS/ISystem.h"

namespace Relentless
{
	class DirectionalLightRenderDispatchSystem : public ISystem
	{
		void Execute(SceneState& aSceneState) noexcept override final;
	};
}