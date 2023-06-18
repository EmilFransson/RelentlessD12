#pragma once
namespace Relentless
{
	class Timer
	{
	public:
		Timer() noexcept = default;
		~Timer() noexcept = default;
		static void Update() noexcept;
		[[nodiscard]] static constexpr float GetDeltaTime() noexcept
		{
			return std::chrono::duration<float>(m_CurrentTimePoint - m_PreviousTimePoint).count();
		}
		[[nodiscard]] static constexpr uint32_t GetFramesPerSecond() noexcept
		{
			return m_FramesPerSecond;
		}
	private:
		static std::chrono::steady_clock::time_point m_PreviousTimePoint;
		static std::chrono::steady_clock::time_point m_CurrentTimePoint;
		static float m_ElapsedTime;
		static float m_ElapsedApplicationTime;
		static uint32_t m_FramesPerSecond;
		static uint32_t m_FramesPerSecondCounter;
	};
}