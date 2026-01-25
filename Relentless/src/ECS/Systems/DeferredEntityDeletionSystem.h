#pragma once

namespace Relentless
{
	struct SceneState;

	struct DeferredEntityDeletionSystem
	{
		static void Execute(SceneState& sceneState) noexcept;
	};
}