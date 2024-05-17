#include "File.h"

namespace Relentless
{
	bool File::Exists(const std::filesystem::path& path) noexcept
	{
		return std::filesystem::exists(path);
	}

	bool File::ExistsDir(const std::filesystem::path& path) noexcept
	{
		std::filesystem::path p = path;
		if (p.has_filename())
			p = p.remove_filename();

		return std::filesystem::is_directory(p);
	}

}

