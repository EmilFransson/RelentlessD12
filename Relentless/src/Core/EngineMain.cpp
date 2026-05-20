#include "EngineMain.h"

namespace Relentless
{
	int EngineMain(UniquePtr<Application> aApplication) noexcept
	{
		if (!aApplication)
			return -1;

		RLS_VERIFY(!FAILED(CoInitializeEx(nullptr, COINIT_MULTITHREADED)));

		aApplication->Run();
		aApplication.reset();

		CoUninitialize();

		return 0;
	}
}