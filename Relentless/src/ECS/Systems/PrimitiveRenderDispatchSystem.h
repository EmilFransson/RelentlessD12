#pragma once
#include "ECS/ISystem.h"

namespace Relentless
{
	class PrimitiveRenderDispatchSystem : public ISystem
	{
	public:
		void Execute(SceneState& aSceneState) noexcept override final;
	};
}