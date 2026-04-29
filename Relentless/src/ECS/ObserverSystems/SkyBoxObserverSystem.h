#pragma once
#include "ECS/IObserverSystem.h"

namespace Relentless
{
	class SkyBoxObserverSystem : public IObserverSystem
	{
	public:
		void Register(Scene& aScene) noexcept override final;
	private:
		void OnActiveSkyBoxChange(Scene& aScene, entity aCurrentSkybox, entity aNewSkyBox) noexcept;
		void OnEntityVisibilityChanged(EntityManager& aEntityManager, entity aEntity) noexcept;
		void OnSkyBoxComponentRemoved(entity aEntity, const UUID& aUUID) noexcept;
	};
}