#include <Relentless.h>

int main(int, char**)
{
	::SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	Relentless::Log::Initialize();
	return Relentless::EngineMain(Relentless::CreateApplication());
}