#pragma once
#include "ECS/ISystem.h"

namespace Relentless
{
	class MeshFilterCleanupSystem : public ISystem
	{
		void Execute(SceneState& aSceneState) noexcept override final;
	};
}