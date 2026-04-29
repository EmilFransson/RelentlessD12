#pragma once
#include "ECS/IObserverSystem.h"

namespace Relentless
{
	class PrimitiveObserverSystem : public IObserverSystem
	{
	public:
		void Register(Scene& aScene) noexcept override final;
	private:
		void OnEntityVisibilityChanged(EntityManager& aEntityManager, entity aEntity) noexcept;
		void OnComponentRemoved(EntityManager& aEntityManager, entity aEntity, const UUID& aUUID) noexcept;
	};
}