#include "FilepathUtils.h"

namespace Relentless
{
	std::string FilepathUtils::ExtractFilename(const std::filesystem::path& filepath) noexcept
	{
		if (filepath.has_filename())
			return filepath.filename().string();
		else
			return "";
	}

	std::string FilepathUtils::ExtractFilenameWithoutExtension(const std::filesystem::path& filepath) noexcept
	{
		if (filepath.has_stem())
			return filepath.stem().string();
		else
			return "";
	}

	std::string FilepathUtils::ExtractExtension(const std::filesystem::path& filepath) noexcept
	{
		if (filepath.has_extension())
			return filepath.extension().string();
		else
			return "";
	}

	std::filesystem::path FilepathUtils::Combine(const std::filesystem::path& basePath, const std::filesystem::path& pathToAppend) noexcept
	{
		return (basePath / pathToAppend).make_preferred();
	}

	void FilepathUtils::Normalize(std::filesystem::path& path) noexcept
	{
		path.make_preferred();
	}

	bool FilepathUtils::HasExtension(const std::filesystem::path& path) noexcept
	{
		return path.has_extension();
	}

	void FilepathUtils::SetExtension(std::filesystem::path& path, const std::string& extension) noexcept
	{
		if (HasExtension(path))
			path.replace_extension(extension);
		else
			path.append(extension);
	}

}
