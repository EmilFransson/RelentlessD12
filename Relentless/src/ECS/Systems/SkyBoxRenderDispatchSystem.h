#pragma once
#include "ECS/ISystem.h"

namespace Relentless
{
	class SkyBoxRenderDispatchSystem : public ISystem
	{
	public:
		void Execute(SceneState& aSceneState) noexcept override final;
	};
}