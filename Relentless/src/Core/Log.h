#pragma once
#include "Core/DLLExport.h"

#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/fmt/bundled/std.h>
#pragma warning(pop)

namespace Relentless
{
	class RLS_API Log
	{
	public:
		static void Initialize() noexcept;
		[[nodiscard]] static constexpr std::shared_ptr<spdlog::logger>& GetCoreLogger() noexcept { return s_CoreLogger; };
	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
	};

#if defined(RLS_DEBUG) || defined RLS_RELWITHDEBINFO
	#define RLS_CORE_TRACE(...)    ::Relentless::Log::GetCoreLogger()->trace(__VA_ARGS__);
	#define RLS_CORE_INFO(...)     ::Relentless::Log::GetCoreLogger()->info(__VA_ARGS__);
	#define RLS_CORE_WARN(...)     ::Relentless::Log::GetCoreLogger()->warn(__VA_ARGS__);
	#define RLS_CORE_ERROR(...)    ::Relentless::Log::GetCoreLogger()->error(__VA_ARGS__);
	#define RLS_CORE_CRITICAL(...) ::Relentless::Log::GetCoreLogger()->critical(__VA_ARGS__);
#else
	#define RLS_CORE_TRACE(...)
	#define RLS_CORE_INFO(...)
	#define RLS_CORE_WARN(...)
	#define RLS_CORE_ERROR(...)
	#define RLS_CORE_CRITICAL(...)
#endif

#if defined(RLS_DEBUG)
	#define RLS_ASSERT(x, ...) {if (!(x)) {RLS_CORE_ERROR("Assertion failed: {0}", __VA_ARGS__); __debugbreak(); } }
	#define RLS_DEBUG_ONLY(x) x
#else
	#define RLS_ASSERT(x, ...)
	#define RLS_DEBUG_ONLY(x)
#endif


#define RLS_VERIFY(x, ...) {if (!(x)) {RLS_CORE_ERROR("Assertion failed: {0}", __VA_ARGS__); __debugbreak(); } }
}