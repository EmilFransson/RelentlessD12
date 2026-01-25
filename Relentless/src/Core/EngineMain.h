#pragma once
#include "Core/Application.h"
#include "Core/DLLExport.h"

namespace Relentless
{
	RLS_API int EngineMain(UniquePtr<Application> aApplication) noexcept;
}