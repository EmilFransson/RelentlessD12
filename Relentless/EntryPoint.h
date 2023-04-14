#pragma once

extern const std::unique_ptr<Relentless::Application> Relentless::CreateApplication() noexcept;

int main(int, char**)
{
	INITIALIZE_DEBUG_MEMORY_LEAK_DETECTION;

	HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	RLS_ASSERT(!FAILED(hr), "Call to CoinitializeEx failed.");


	auto app = Relentless::CreateApplication();
	app->Run();

	return 0;
}