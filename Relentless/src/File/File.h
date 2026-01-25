#pragma once
#include "Core/DLLExport.h"

namespace Relentless
{
	enum class EFileMoveMode : uint8 { NoOverWrite = 0, OverWrite };

	class RLS_API File
	{
	public:
		static bool ClearAttributes(const Path& aPath) noexcept;

		NO_DISCARD static bool Exists(const Path& aPath) noexcept;
		NO_DISCARD static bool ExistsDir(const Path& aPath) noexcept;

		static bool Delete(const Path& aPath) noexcept;
	
		static bool Move(const Path& aSrcPath, const Path& aDstPath, EFileMoveMode aMoveMode = EFileMoveMode::NoOverWrite) noexcept;
		
		static bool Replace(const Path& aSrcPath, const Path& aDstPath) noexcept;
	};
}