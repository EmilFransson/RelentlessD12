#include "AssetManager.h"

namespace Relentless
{
	AssetManager::Data AssetManager::s_Data{};

	void AssetManager::Initialize() noexcept
	{
		s_Data.MaterialManager.Intitialize();
		//s_Data.MeshManager.Initialize();
		//s_Data.TextureManager.Initialize();
	}

	MaterialManager& AssetManager::GetMaterialManager() noexcept 
	{ 
		return s_Data.MaterialManager; 
	}

	MeshManager& AssetManager::GetMeshManager() noexcept 
	{ 
		return s_Data.MeshManager; 
	}

	TextureManager& AssetManager::GetTextureManager() noexcept 
	{ 
		return s_Data.TextureManager; 
	}
}