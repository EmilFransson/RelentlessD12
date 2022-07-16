#include "Log.h"
#pragma warning(push, 0)
#include "spdlog/sinks/stdout_color_sinks.h"
#pragma warning(pop)
namespace Relentless
{
	std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
	//std::shared_ptr<spdlog::logger> Log::s_ClientLogger;
	void Log::Initialize() noexcept
	{
		spdlog::set_pattern("%^[%T] %n: %v%$");
		s_CoreLogger = spdlog::stdout_color_mt("RELENTLESS");
		s_CoreLogger->set_level(spdlog::level::trace);
		RLS_CORE_INFO("Core logger initialized.");

		//s_ClientLogger = spdlog::stdout_color_mt("SANDBOX");
		//s_ClientLogger->set_level(spdlog::level::trace);
		//RLS_INFO("App logger initialized.");
	}
}