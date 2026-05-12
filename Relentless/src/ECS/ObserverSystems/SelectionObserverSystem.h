#pragma once
#include "ECS/IObserverSystem.h"

namespace Relentless
{
	class SelectionObserverSystem : public IObserverSystem
	{
	public:
		void Register(Scene& aScene) noexcept override final;
	private:
		void OnEntitySelectedInEditor(const UUID& aSceneUUID, entity aEntity) noexcept;
		void OnEntityDeselectedInEditor(const UUID& aSceneUUID, entity aEntity) noexcept;
	};
}