#pragma once
#include "ECS/IObserverSystem.h"

namespace Relentless
{
	class SkyLightObserverSystem : public IObserverSystem
	{
	public:
		void Register(Scene& aScene) noexcept override final;
	private:
		void OnActiveSkyLightChange(Scene& aScene, entity aCurrentSkyLight, entity aNewSkyLight) noexcept;
		void OnEntityVisibilityChanged(EntityManager& aEntityManager, entity aEntity) noexcept;
		void OnSkyLightComponentRemoved(entity aEntity, const UUID& aUUID) noexcept;
	};
}