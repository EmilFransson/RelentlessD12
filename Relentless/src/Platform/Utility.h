#pragma once
#include "Core/DLLExport.h"

namespace Relentless::Platform
{
	NO_DISCARD RLS_API bool IsReservedDeviceName(StringView aName) noexcept;
	NO_DISCARD RLS_API bool IsValidFileName(StringView aName) noexcept;
}