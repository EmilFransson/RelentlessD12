#pragma once

namespace Relentless
{
	namespace Math
	{
		constexpr float PI = 3.14159265358979323846f;
		constexpr float TAU = PI * 2.0f;
		constexpr float EPSILON = 1e-6f;

		inline [[nodiscard]] static float RadToDeg(float radians) noexcept
		{
			constexpr float DEGREES_PER_RADIANS = 180.0f / PI;
			return radians * DEGREES_PER_RADIANS;
		}

		inline [[nodiscard]] static float DegToRad(float degrees) noexcept
		{
			constexpr float RADIANS_PER_DEGREE = PI / 180.0f;
			return degrees * RADIANS_PER_DEGREE;
		}

		inline [[nodiscard]] static float NormalizeDegrees(float degrees) 
		{
			degrees = std::fmod(degrees, 360.0f);
			if (degrees < 0.0f) 
			{
				degrees += 360.0f;
			}
			return degrees;
		}
	}
}