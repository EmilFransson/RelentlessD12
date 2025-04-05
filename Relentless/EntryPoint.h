#pragma once
#include "src/Core/Log.h"

extern const std::unique_ptr<Relentless::Application> Relentless::CreateApplication() noexcept;

int main(int, char**)
{
	INITIALIZE_DEBUG_MEMORY_LEAK_DETECTION;

#if defined RLS_DEBUG
	HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	RLS_ASSERT(!FAILED(hr), "Call to CoinitializeEx failed.");
#else
	CoInitializeEx(nullptr, COINIT_MULTITHREADED);
#endif

	Relentless::Log::Initialize();
	{
		auto app = Relentless::CreateApplication();
		app->Run();
	}

	_CrtDumpMemoryLeaks();
	return 0;
}