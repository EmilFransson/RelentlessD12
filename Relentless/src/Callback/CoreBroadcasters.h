#pragma once
#include "Broadcaster.h"

#include "Core/StaticTypeInfo.h"

#include "ECS/ECSCommon.h"

namespace Relentless
{
	class IAsset;
	struct IComponent;
	
	struct RLS_API CoreObjectBroadcasters
	{
		//ECS:
		static Broadcaster<void(entity aEntity, TypeIndex aComponentType, IComponent* aComponent, uint64 aProperty)> OnEntityComponentPropertyChanged;

		//Asset:
		static Broadcaster<void(IAsset* aAsset, TypeIndex aAssetType, uint64 aProperty)> OnAssetPropertyChanged;
		static Broadcaster<void(IAsset* aAsset, TypeIndex aAssetType)> OnAssetCreated;
		static Broadcaster<void(IAsset* aAsset, TypeIndex aAssetType)> OnAssetDestroy;
	};
}