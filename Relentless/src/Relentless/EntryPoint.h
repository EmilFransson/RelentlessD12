#pragma once

extern const std::unique_ptr<Relentless::Application> Relentless::CreateApplication() noexcept;

const bool InitializeConsole() noexcept;

int WINAPI wWinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int)
{
	INITIALIZE_DEBUG_MEMORY_LEAK_DETECTION;

	InitializeConsole();
	auto app = Relentless::CreateApplication();
	app->Run();

	return 0;
}

const bool InitializeConsole() noexcept
{
#pragma warning(push, 0) //To allow freopen-function
	if (AllocConsole() == FALSE)
		return false;
	if (freopen("CONIN$", "r", stdin) == nullptr)
		return false;
	if (freopen("CONOUT$", "w", stdout) == nullptr)
		return false;
	if (freopen("CONOUT$", "w", stderr) == nullptr)
		return false;
#pragma warning(pop)
	return true;
}