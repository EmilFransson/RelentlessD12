#pragma once
#include "Broadcaster.h"

#include "Core/StaticTypeInfo.h"

#include "ECS/ECSCommon.h"

namespace Relentless
{
	struct IComponent;
	
	struct RLS_API CoreObjectBroadcasters
	{
		static Broadcaster<void(entity aEntity, TypeIndex aComponentType, IComponent* aComponent, uint64 aProperty)> OnComponentPropertyChanged;
	};
}