#include "CoreBroadcasters.h"

#include "ECS/Component.h"

namespace Relentless
{
	Broadcaster<void(entity aEntity, TypeIndex aComponentType, IComponent* aComponent, uint64 aProperty)> CoreObjectBroadcasters::OnComponentPropertyChanged;
}