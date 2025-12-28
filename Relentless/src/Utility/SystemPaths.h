#pragma once

namespace Relentless
{
	class SystemPaths
	{
	public:
		static void Initialize() noexcept;
		static NO_DISCARD Path GetWorkingDirectory() noexcept;
		static NO_DISCARD Path GetUserDocumentsDirectory() noexcept;
		static NO_DISCARD Path GetUserHomeDirectory() noexcept;
		static NO_DISCARD Path GetEngineAssetsDirectory() noexcept;
		static NO_DISCARD Path GetEditorAssetsDirectory() noexcept;
	};
}