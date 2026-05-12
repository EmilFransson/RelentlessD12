#pragma once
#include "ECS/IObserverSystem.h"

namespace Relentless
{
	class PostProcessObserverSystem : public IObserverSystem
	{
	public:
		void Register(Scene& aScene) noexcept override final;
	private:
		void OnEntityVisibilityChanged(EntityManager& aEntityManager, entity aEntity) noexcept;
		void OnPostProcessVolumeComponentRemoved(entity aEntity, const UUID& aUUID) noexcept;
	};
}