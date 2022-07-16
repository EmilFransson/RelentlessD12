#pragma once
#pragma warning(push, 0)
#include "spdlog\spdlog.h"
#include "spdlog/fmt/ostr.h"
#pragma warning(pop)

namespace Relentless
{
	class Log
	{
	public:
		static void Initialize() noexcept;
		[[nodiscard]] static constexpr std::shared_ptr<spdlog::logger>& GetCoreLogger() noexcept { return s_CoreLogger; };
		//[[nodiscard]] static constexpr std::shared_ptr<spdlog::logger>& GetClientLogger() noexcept { return s_ClientLogger; };
	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		//static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};

#if defined(RLS_DEBUG)
	#define RLS_CORE_TRACE(...)    ::Relentless::Log::GetCoreLogger()->trace(__VA_ARGS__);
	#define RLS_CORE_INFO(...)     ::Relentless::Log::GetCoreLogger()->info(__VA_ARGS__);
	#define RLS_CORE_WARN(...)     ::Relentless::Log::GetCoreLogger()->warn(__VA_ARGS__);
	#define RLS_CORE_ERROR(...)    ::Relentless::Log::GetCoreLogger()->error(__VA_ARGS__);
	#define RLS_CORE_CRITICAL(...) ::Relentless::Log::GetCoreLogger()->critical(__VA_ARGS__);

	//#define RLS_TRACE(...)         ::Relentless::Log::GetClientLogger()->trace(__VA_ARGS__);
	//#define RLS_INFO(...)          ::Relentless::Log::GetClientLogger()->info(__VA_ARGS__);
	//#define RLS_WARN(...)          ::Relentless::Log::GetClientLogger()->warn(__VA_ARGS__);
	//#define RLS_ERROR(...)         ::Relentless::Log::GetClientLogger()->error(__VA_ARGS__);
	//#define RLS_CRITICAL(...)      ::Relentless::Log::GetClientLogger()->critical(__VA_ARGS__);

	#define RLS_ASSERT(x, ...) {if (!(x)) {RLS_CORE_ERROR("Assertion failed: {0}", __VA_ARGS__); __debugbreak(); } }
#else
	#define RLS_CORE_TRACE(...)
	#define RLS_CORE_INFO(...)
	#define RLS_CORE_WARN(...)
	#define RLS_CORE_ERROR(...)
	#define RLS_CORE_CRITICAL(...)

	//#define RLS_TRACE(...)
	//#define RLS_INFO(...)
	//#define RLS_WARN(...)
	//#define RLS_ERROR(...)
	//#define RLS_CRITICAL(...)

	#define RLS_ASSERT(x, ...)
#endif

}