#pragma once
#include "Core/DLLExport.h"

namespace Relentless
{
	struct ProfilerMetrics
	{
		const char* ContextName;
		float durationInMilliSeconds;
	};

	template<class LambdaFunction>
	class RLS_API Profiler
	{
	public:
		Profiler(const char* contextName, const LambdaFunction&& lambdaFunction) noexcept
			: m_ContextName{ contextName }, m_LambdaFunction{ lambdaFunction }
		{
			m_StartTime = std::chrono::high_resolution_clock::now();
		}
		~Profiler() noexcept
		{
			std::chrono::time_point<std::chrono::steady_clock> endTime = std::chrono::high_resolution_clock::now();

			long long startTimeInMicroSeconds = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTime).time_since_epoch().count();
			long long endTimeInMicroSeconds = std::chrono::time_point_cast<std::chrono::microseconds>(endTime).time_since_epoch().count();

			float durationInMilliSeconds = (endTimeInMicroSeconds - startTimeInMicroSeconds) * 0.001f;
			m_LambdaFunction({ m_ContextName, durationInMilliSeconds });
		}
	private:
		const char* m_ContextName;
		const LambdaFunction m_LambdaFunction;
		std::chrono::time_point<std::chrono::steady_clock> m_StartTime;
	};

	struct RLS_API ProfilerManager
	{
		static std::vector<ProfilerMetrics> ProfilerMetrics;
		static void ClearData() noexcept { ProfilerMetrics.clear(); }
	};

#define TOKENPASTE(x, y) x ## y
#define TOKENPASTE2(x, y) TOKENPASTE(x, y)
#define PROFILE_FUNC Profiler TOKENPASTE2(profiler, __LINE__) (__FUNCTION__, [&](ProfilerMetrics profileMetrics) { ProfilerManager::ProfilerMetrics.emplace_back(profileMetrics);})
#define PROFILE_SCOPE(contextName) Profiler TOKENPASTE2(profiler, __LINE__) (contextName, [&](ProfilerMetrics profileMetrics) { ProfilerManager::ProfilerMetrics.emplace_back(profileMetrics);})
}