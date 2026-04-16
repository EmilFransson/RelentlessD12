#pragma once
#include "ECS/ECSCommon.h"

namespace Relentless
{
	class Scene;

	class IObserverSystem
	{
	public:
		virtual ~IObserverSystem() noexcept = default;
		virtual void Register(Scene& aScene) noexcept = 0;
	};
}