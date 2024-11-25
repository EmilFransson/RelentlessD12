#include "Time.h"
namespace Relentless
{
	std::chrono::steady_clock::time_point Time::m_CurrentTimePoint = std::chrono::steady_clock::now();
	std::chrono::steady_clock::time_point Time::m_PreviousTimePoint = std::chrono::steady_clock::now();
	float Time::m_ElapsedTime = 0.0f;
	float Time::m_ElapsedApplicationTime = 0.0f;
	uint32_t Time::m_FramesPerSecond = 0u;
	uint32_t Time::m_FramesPerSecondCounter = 0u;

	void Time::Tick() noexcept
	{
		m_PreviousTimePoint = m_CurrentTimePoint;
		m_CurrentTimePoint = std::chrono::steady_clock::now();

		m_ElapsedTime += GetDeltaTime();
		m_ElapsedApplicationTime += GetDeltaTime();
		if (m_ElapsedTime >= 1.0f)
		{
			m_ElapsedTime = 0.0f;
			m_FramesPerSecond = m_FramesPerSecondCounter;
			m_FramesPerSecondCounter = 0u;
		}
		m_FramesPerSecondCounter++;
	}
}