#include "Application.h"
namespace Relentless
{
	void Application::SayHello()
	{
		::MessageBoxA(nullptr, "HELLO", "HELLO", MB_OK);
	}
}