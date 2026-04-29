#pragma once
#include "ECS/IObserverSystem.h"

namespace Relentless
{
	class LightObserverSystem : public IObserverSystem
	{
	public:
		void Register(Scene& aScene) noexcept override final;
	private:
		void OnEntityVisibilityChanged(EntityManager& aEntityManager, entity aEntity) noexcept;
		void OnLightComponentRemoved(entity aEntity, const UUID& aUUID) noexcept;
	};
}