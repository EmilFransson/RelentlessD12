#include "Timer.h"
namespace Relentless
{
	std::chrono::steady_clock::time_point Timer::m_CurrentTimePoint = std::chrono::steady_clock::now();
	std::chrono::steady_clock::time_point Timer::m_PreviousTimePoint = std::chrono::steady_clock::now();
	float Timer::m_ElapsedTime = 0.0f;
	float Timer::m_ElapsedApplicationTime = 0.0f;
	uint32_t Timer::m_FramesPerSecond = 0u;
	uint32_t Timer::m_FramesPerSecondCounter = 0u;

	void Timer::Update() noexcept
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