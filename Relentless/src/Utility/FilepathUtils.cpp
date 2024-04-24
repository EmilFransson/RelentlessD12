#include "FilepathUtils.h"

namespace Relentless
{
	std::string FilepathUtils::ExtractExtension(const std::filesystem::path& filepath) noexcept
	{
		if (std::filesystem::exists(filepath) && filepath.has_extension())
		{
			return filepath.extension().string();
		}
		else
			return "";
	}

	std::filesystem::path FilepathUtils::Combine(const std::filesystem::path& basePath, const std::filesystem::path& pathToAppend) noexcept
	{
		return basePath / pathToAppend;
	}

}
