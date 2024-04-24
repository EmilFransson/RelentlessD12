#pragma once

namespace Relentless
{
	class SystemPaths
	{
	public:
		static void Initialize() noexcept;
		static [[nodiscard]] std::filesystem::path GetWorkingDirectory() noexcept;
		static [[nodiscard]] std::filesystem::path GetUserDocumentsDirectory() noexcept;
		static [[nodiscard]] std::filesystem::path GetEngineAssetsDirectory() noexcept;
		static [[nodiscard]] std::filesystem::path GetEditorAssetsDirectory() noexcept;
	};
}