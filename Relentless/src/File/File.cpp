#include "File.h"

namespace Relentless
{
	bool File::ClearAttributes(const Path& aPath) noexcept
	{
		DWORD attrs = ::GetFileAttributesW(aPath.c_str());
		if (attrs == INVALID_FILE_ATTRIBUTES)
			return false;

		attrs &= ~(FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);

		return ::SetFileAttributesW(aPath.c_str(), attrs) != FALSE;
	}

	bool File::Exists(const Path& aPath) noexcept
	{
		return std::filesystem::exists(aPath);
	}

	bool File::ExistsDir(const Path& aPath) noexcept
	{
		Path p = aPath;
		if (p.has_filename())
			p = p.remove_filename();

		return std::filesystem::is_directory(p);
	}

	bool File::Delete(const Path& aPath) noexcept
	{
		return std::filesystem::remove(aPath);
	}

	bool File::Move(const Path& aSrcPath, const Path& aDstPath, EFileMoveMode aMoveMode) noexcept
	{
		DWORD moveFlags = MOVEFILE_WRITE_THROUGH;

		if (aMoveMode == EFileMoveMode::OverWrite)
			moveFlags |= MOVEFILE_REPLACE_EXISTING;

		return ::MoveFileExW(aSrcPath.c_str(), aDstPath.c_str(), moveFlags);
	}

	bool File::Replace(const Path& aSrcPath, const Path& aDstPath) noexcept
	{
		return ::ReplaceFileW(aDstPath.c_str(), aSrcPath.c_str(), nullptr, REPLACEFILE_IGNORE_MERGE_ERRORS, nullptr, nullptr);
	}
}

