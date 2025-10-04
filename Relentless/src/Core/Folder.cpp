#include "Folder.h"

#include "Scene/Scene.h"

namespace Relentless
{
	FolderRoot FolderRoot::CreateFromScene(const Scene& scene) noexcept
	{
		FolderRoot toReturn
		{
			.UUID = scene.GetUUID(),
			.Type = Type::Scene
		};

		return toReturn;
	}

	Folder::Folder(const FolderRoot& aFolderRoot, const String& aPath) noexcept
		:m_Root(aFolderRoot)
		,m_Path(aPath)
	{
	}

}
