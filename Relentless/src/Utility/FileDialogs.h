#pragma once
#include "Core/DLLExport.h"

namespace Relentless
{
	class RLS_API FileDialogs
	{
	public:
		NO_DISCARD static std::vector<std::string> OpenFile(const char* filter) noexcept;
		NO_DISCARD static std::string SaveFile(const char* filter) noexcept;
	};
}