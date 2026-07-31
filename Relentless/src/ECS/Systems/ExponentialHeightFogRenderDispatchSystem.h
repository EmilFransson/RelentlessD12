#pragma once
#include "ECS/ISystem.h"

namespace Relentless
{
	class ExponentialHeightFogRenderDispatchSystem : public ISystem
	{
	public:
		void Execute(SceneState& aSceneState) noexcept override final;
	};
}