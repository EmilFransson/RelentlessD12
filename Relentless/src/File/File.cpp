#include "File.h"

namespace Relentless
{
	bool File::Exists(const std::filesystem::path& path) noexcept
	{
		return std::filesystem::exists(path);
	}
}

