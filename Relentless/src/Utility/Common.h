#pragma once 
#include "Core/DLLExport.h"

namespace Relentless
{
	NO_DISCARD RLS_API UUID ConvertStringToGUID(const String& guidString) noexcept;
	NO_DISCARD RLS_API String ConvertUUIDToString(const UUID& uuid) noexcept;
	NO_DISCARD RLS_API UUID CreateUUID() noexcept;

	struct ScopedFlag 
	{
		bool& flag;
		explicit ScopedFlag(bool& f) noexcept : flag(f) { flag = true; }
		~ScopedFlag() noexcept { flag = false; }
	};
}

namespace std
{
	template<>
	struct RLS_API hash<UUID>
	{
		size_t operator()(const UUID& id) const;
	};
}