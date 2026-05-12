#pragma once
#include "Core/DLLExport.h"

namespace Relentless
{
	class RLS_API SystemPaths
	{
	public:
		static void Initialize() noexcept;
		NO_DISCARD static Path GetWorkingDirectory() noexcept;
		NO_DISCARD static Path GetUserDocumentsDirectory() noexcept;
		NO_DISCARD static Path GetUserHomeDirectory() noexcept;
		NO_DISCARD static Path GetEngineAssetsDirectory() noexcept;
		NO_DISCARD static Path GetEditorAssetsDirectory() noexcept;
	};
}