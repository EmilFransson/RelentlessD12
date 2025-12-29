#pragma once 
#include <Relentless.h>
#include "../Assets/Factory/IFactory.h"

namespace Relentless
{
	class AssetImportSubsystem : public ISubsystem
	{
	public:
		NO_DISCARD bool OnLoad(ISystemManager* aSystemManager) noexcept override;
		static bool ShouldCreateSubsystem(ISystemManager* aSystemManager) noexcept;
		
		Broadcaster<void(Ref<IFactory> aFactory, AssetHandle aAssetHandle)> OnPostAssetImported;
	};
}