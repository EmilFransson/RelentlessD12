#pragma once
namespace Relentless
{
	using TimeStamp = uint64;

	class Time
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

		[[nodiscard]] static float GetDeltaTime() noexcept
		{
			return std::chrono::duration<float>(m_CurrentTimePoint - m_PreviousTimePoint).count();
		}
		[[nodiscard]] static float GetElapsedTime() noexcept
		{
			return m_ElapsedApplicationTime;
		}

		[[nodiscard]] static uint32 GetFrameCount() noexcept;
		[[nodiscard]] static uint32 GetFramesPerSecond() noexcept
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