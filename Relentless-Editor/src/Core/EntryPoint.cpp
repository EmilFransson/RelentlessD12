#include <Relentless.h>

int main(int, char**)
{
	Relentless::Log::Initialize();
	return Relentless::EngineMain(Relentless::CreateApplication());
}