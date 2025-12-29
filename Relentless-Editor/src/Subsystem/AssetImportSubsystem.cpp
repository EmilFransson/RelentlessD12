#include "AssetImportSubsystem.h"
#include "../Core/Editor.h"

namespace Relentless
{
	bool AssetImportSubsystem::OnLoad(ISystemManager*) noexcept
	{
		return true;
	}

	bool AssetImportSubsystem::ShouldCreateSubsystem(ISystemManager* aSystemManager) noexcept
	{
		return dynamic_cast<Editor*>(aSystemManager) != nullptr;
	}
}