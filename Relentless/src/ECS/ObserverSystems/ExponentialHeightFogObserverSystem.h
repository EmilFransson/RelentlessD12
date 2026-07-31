#pragma once
#include "ECS/IObserverSystem.h"

namespace Relentless
{
	class ExponentialHeightFogObserverSystem : public IObserverSystem
	{
	public:
		void Register(Scene& aScene) noexcept override final;
	private:
		void OnActiveExponentialHeightFogChange(Scene& aScene, entity aCurrentExponentialHeightFog, entity aNewExponentialHeightFog) noexcept;
		void OnEntityVisibilityChanged(EntityManager& aEntityManager, entity aEntity) noexcept;
		void OnExponentialHeightFogComponentRemoved(entity aEntity, const UUID& aUUID) noexcept;
	};
}