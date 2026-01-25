#pragma once
#include "Core/DLLExport.h"

namespace Relentless
{
	using TimeStamp = uint64;

	class RLS_API Time
	{
	public:
		Time() noexcept = default;
		~Time() noexcept = default;
		static void Tick() noexcept;

		NO_DISCARD static uint64 GetCurrentTimeStamp() noexcept
		{
			auto now = std::chrono::system_clock::now();
			auto duration = now.time_since_epoch();
			return std::chrono::duration_cast<std::chrono::seconds>(duration).count();
		}

		NO_DISCARD static TimeStamp GetCurrentTimePoint() noexcept
		{
			return std::chrono::steady_clock::now().time_since_epoch().count();
		}

		NO_DISCARD static float GetElapsedMsSince(TimeStamp aStartTimePoint) noexcept
		{
			const TimeStamp now = GetCurrentTimePoint();
			auto duration = std::chrono::nanoseconds(now - aStartTimePoint);
			return std::chrono::duration<float, std::milli>(duration).count();
		}

		NO_DISCARD static float GetDeltaTime() noexcept
		{
			return std::chrono::duration<float>(m_CurrentTimePoint - m_PreviousTimePoint).count();
		}

		NO_DISCARD static float GetElapsedTime() noexcept
		{
			return m_ElapsedApplicationTime;
		}

		NO_DISCARD static uint32 GetFrameCount() noexcept;
		NO_DISCARD static uint32 GetFramesPerSecond() noexcept
		{
			return m_FramesPerSecond;
		}
	private:
		static std::chrono::steady_clock::time_point m_PreviousTimePoint;
		static std::chrono::steady_clock::time_point m_CurrentTimePoint;
		static float m_ElapsedTime;
		static float m_ElapsedApplicationTime;
		static uint32 m_FramesPerSecond;
		static uint32 m_FramesPerSecondCounter;
	};
}