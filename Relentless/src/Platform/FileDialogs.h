#pragma once
#include "Core/DLLExport.h"

namespace Relentless::Platform
{
	NO_DISCARD RLS_API std::vector<Path> OpenFileDialog();

	RLS_API bool ShowInExplorer(const Path& aPath);
}