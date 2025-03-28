#pragma once

namespace Relentless
{
	class FilePath
	{
	public:
		static [[nodiscard]] std::filesystem::path GetEngineWorkingDirectory()
		{
			return std::filesystem::path(MAIN_ENGINE_DIRECTORY);
		}
	private:

	};

}