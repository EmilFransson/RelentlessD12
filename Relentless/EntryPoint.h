#pragma once

extern const std::unique_ptr<Relentless::Application> Relentless::CreateApplication() noexcept;

int main(int, char**)
{
	INITIALIZE_DEBUG_MEMORY_LEAK_DETECTION;

	auto app = Relentless::CreateApplication();
	app->Run();

	return 0;
}