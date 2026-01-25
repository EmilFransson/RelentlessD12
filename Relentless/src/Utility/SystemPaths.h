#pragma once

namespace Relentless
{
	class SystemPaths
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