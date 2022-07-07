#pragma once
extern const std::unique_ptr<Relentless::Application> Relentless::CreateApplication() noexcept;

int WINAPI wWinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int)
{
	auto app = Relentless::CreateApplication();
	app->SayHello();

	return 0;
}