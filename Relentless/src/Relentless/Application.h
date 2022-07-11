#pragma once

namespace Relentless
{
	class Application
	{
	public:
		Application() = default;
		virtual ~Application() = default;
		void SayHello();
	};

	//To be defined in client:
	[[nodiscard]] const std::unique_ptr<Application> CreateApplication() noexcept;
}