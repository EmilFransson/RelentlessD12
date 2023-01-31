#include "AssetManager.h"
namespace Relentless
{
	AssetManager AssetManager::s_instance;

	bool AssetManager::HasLoaded(const std::string& assetPath) const noexcept
	{
		return m_PathToResourceIDMap.contains(assetPath);
	}
}