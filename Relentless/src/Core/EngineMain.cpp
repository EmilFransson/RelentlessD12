#include "EngineMain.h"

namespace Relentless
{
	int EngineMain(UniquePtr<Application> aApplication) noexcept
	{
		if (!aApplication)
			return -1;

		INITIALIZE_DEBUG_MEMORY_LEAK_DETECTION;

		RLS_VERIFY(!FAILED(CoInitializeEx(nullptr, COINIT_MULTITHREADED)));

		aApplication->Run();

		CoUninitialize();
		_CrtDumpMemoryLeaks();

		return 0;
	}
}