#pragma once
#include "ECS/ISystem.h"

namespace Relentless
{
	class MeshRendererCleanupSystem : public ISystem
	{
		void Execute(SceneState& aSceneState) noexcept override final;
	};
}