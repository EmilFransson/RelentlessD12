#pragma once
#include "ECS/ISystem.h"

namespace Relentless
{
	class DeferredEntityDeletionSystem : public ISystem
	{
		void Execute(SceneState& aSceneState) noexcept override final;
	};
}