#pragma once

namespace Relentless
{
	class SceneState;

	struct DeferredEntityDeletionSystem
	{
		static void Execute(SceneState& sceneState) noexcept;
	};
}