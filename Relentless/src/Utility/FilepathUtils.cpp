#include "FilepathUtils.h"
#include "Rules.h"

namespace Relentless
{
	bool FilepathUtils::CreateDirectoryTree(const Path& aPath) noexcept
	{
		String pathAsString = aPath.string();
		std::replace(pathAsString.begin(), pathAsString.end(), '\\', '/');
		size_t slash = pathAsString.find('/', 0);
		while (slash != String::npos)
		{
			if (slash > 1)
			{
				const String dirToCreate = pathAsString.substr(0, slash);
				const BOOL success = ::CreateDirectoryA(dirToCreate.c_str(), nullptr);
				const DWORD error = ::GetLastError();
				if (success != TRUE && error == ERROR_PATH_NOT_FOUND)
				{
					return false;
				}
			}
			slash = pathAsString.find('/', slash + 1);
		}
		return true;
	}

	String FilepathUtils::ExtractFilename(const Path& aFilepath) noexcept
	{
		if (aFilepath.has_filename())
			return aFilepath.filename().string();
		else
			return "";
	}

	String FilepathUtils::ExtractFilenameWithoutExtension(const Path& aFilepath) noexcept
	{
		if (aFilepath.has_stem())
			return aFilepath.stem().string();
		else
			return "";
	}

	String FilepathUtils::ExtractExtension(const Path& aFilepath) noexcept
	{
		if (aFilepath.has_extension())
			return aFilepath.extension().string();
		else
			return "";
	}

	Path FilepathUtils::Combine(const Path& basePath, const Path& pathToAppend) noexcept
	{
		return (basePath / pathToAppend).make_preferred();
	}

	String FilepathUtils::CombineDisplay(const Path& aBasePath, const Path& aPathToAppend) noexcept
	{
		String s = (aBasePath / aPathToAppend).string();
		std::replace(s.begin(), s.end(), '\\', '/'); 
		return s;
	}

	void FilepathUtils::Normalize(Path& aFilepath) noexcept
	{
		aFilepath.make_preferred();
	}

	bool FilepathUtils::HasExtension(const Path& aFilepath) noexcept
	{
		return aFilepath.has_extension();
	}

	void FilepathUtils::SetExtension(Path& aFilepath, const String& aExtension) noexcept
	{
		if (HasExtension(aFilepath))
			aFilepath.replace_extension(aExtension);
		else
			aFilepath.append(aExtension);
	}

	String FilepathUtils::SanitizeFileName(const String& aFileName) noexcept
	{
		String sanitized = aFileName;
		const String illegalCharacters = R"(\/:*?"<>|)";
		for (char& ch : sanitized) 
		{
			if (illegalCharacters.find(ch) != String::npos || std::iscntrl(static_cast<unsigned char>(ch)))
				ch = '_';
		}
		
		return sanitized;
	}

	bool FilepathUtils::IsDirectory(const Path& aFilepath) noexcept
	{
		return std::filesystem::is_directory(aFilepath);
	}

}
