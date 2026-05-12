#pragma once
#include "ECS/ISystem.h"

namespace Relentless
{
	class PostProcessRenderDispatchSystem : public ISystem
	{
	public:
		void Execute(SceneState& aSceneState) noexcept override final;
	};
}