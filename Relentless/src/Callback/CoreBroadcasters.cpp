#include "CoreBroadcasters.h"

#include "Assets/IAsset.h"

#include "ECS/Component.h"

namespace Relentless
{
	//ECS:
	Broadcaster<void(entity aEntity, TypeIndex aComponentType, IComponent* aComponent, uint64 aProperty)> CoreObjectBroadcasters::OnEntityComponentPropertyChanged;
	
	//Asset:
	Broadcaster<void(IAsset* aAsset, TypeIndex aAssetType, uint64 aProperty)> CoreObjectBroadcasters::OnAssetPropertyChanged;
	Broadcaster<void(IAsset* aAsset, TypeIndex aAssetType)> CoreObjectBroadcasters::OnAssetCreated;
	Broadcaster<void(IAsset* aAsset, TypeIndex aAssetType)> CoreObjectBroadcasters::OnAssetDestroy;
}