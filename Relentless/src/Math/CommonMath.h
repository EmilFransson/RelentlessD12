#pragma once
#include "MathTypes.h"

namespace Relentless
{
	namespace Colors
	{
		constexpr Color Transparent = Color(0.0f, 0.0f, 0.0f, 0.0f);
		constexpr Color White = Color(1.0f, 1.0f, 1.0f, 1.0f);
		constexpr Color Black = Color(0.0f, 0.0f, 0.0f, 1.0f);
		constexpr Color Red = Color(1.0f, 0.0f, 0.0f, 1.0f);
		constexpr Color Green = Color(0.0f, 1.0f, 0.0f, 1.0f);
		constexpr Color Blue = Color(0.0f, 0.0f, 1.0f, 1.0f);
		constexpr Color Yellow = Color(1.0f, 1.0f, 0.0f, 1.0f);
		constexpr Color Magenta = Color(1.0f, 0.0f, 1.0f, 1.0f);
		constexpr Color Cyan = Color(0.0f, 1.0f, 1.0f, 1.0f);
		constexpr Color Gray = Color(0.5f, 0.5f, 0.5f, 1.0f);
		constexpr Color LightSkyBlue = Color(0.529411793f, 0.807843208f, 0.980392218f, 1.0f);
	};

	namespace Math
	{
		constexpr float PI = 3.14159265358979323846f;
		constexpr float PI_DIV_4 = 0.78539816339744830961f;
		constexpr float TAU = PI * 2.0f;
		constexpr float EPSILON = 1e-6f;

		constexpr float BytesToKiloBytes = 1.0f / (1 << 10);
		constexpr float BytesToMegaBytes = 1.0f / (1 << 20);
		constexpr float BytesToGigaBytes = 1.0f / (1 << 30);

		constexpr uint32_t KilobytesToBytes = 1 << 10;
		constexpr uint32_t MegaBytesToBytes = 1 << 20;
		constexpr uint32_t GigaBytesToBytes = 1 << 30;

		template<typename T>
		T AlignUp(T value, T alignment) noexcept
		{
			return (value + ((T)alignment - 1)) & ~(alignment - 1);
		}

		template<typename T, typename U, typename V>
		constexpr T Clamp(const T value, const U low, const V high)
		{
			if (value > high)
				return (T)high;
			else if (value < low)
				return (T)low;
			return value;
		}

		constexpr inline uint32 DivideAndRoundUp(uint32 nominator, uint32 denominator) noexcept
		{
			return (nominator + denominator - 1) / denominator;
		}

		template<typename T>
		constexpr T Max(const T& a, const T& b) noexcept
		{
			return a < b ? b : a;
		}

		template<typename T>
		constexpr T Min(const T& a, const T& b) noexcept
		{
			return a < b ? a : b;
		}

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

		// Create left-handed DX style perspective matrix
		// FoV is vertical FoV in radians
		inline [[nodiscard]] Matrix CreatePerspectiveMatrix(float FoV, float aspectRatio, float nearZ, float farZ) noexcept
		{
			const float sinFov = sinf(FoV * 0.5f);
			const float cosFov = cosf(FoV * 0.5f);

			const float B = cosFov / sinFov;
			const float A = B / aspectRatio;
			const float C = farZ / (farZ - nearZ);
			const float D = 1.0f; // Needs to be -1 for right handed
			const float E = -nearZ * C; // Positive in right handed

			return Matrix(
				A, 0, 0, 0,
				0, B, 0, 0,
				0, 0, C, D,
				0, 0, E, 0
			);
		}

		inline [[nodiscard]] Matrix CreateLookToMatrix(const Vector3& position, const Vector3& direction, const Vector3& up)
		{
			Vector3 z;
			direction.Normalize(z);
			Vector3 x = up.Cross(z);
			x.Normalize();
			Vector3 y = z.Cross(x);

			Vector3 p(
				x.Dot(-position),
				y.Dot(-position),
				z.Dot(-position)
			);

			return Matrix(
				x.x, y.x, z.x, 0,
				x.y, y.y, z.y, 0,
				x.z, y.z, z.z, 0,
				p.x, p.y, p.z, 1
			);
		}

		inline [[nodiscard]] BoundingFrustum CreateBoundingFrustum(const Matrix& projection, const Matrix& view) noexcept
		{
			BoundingFrustum frustum;
			BoundingFrustum::CreateFromMatrix(frustum, projection);
			if (frustum.Far < frustum.Near)
				std::swap(frustum.Far, frustum.Near);
			frustum.Transform(frustum, view.Invert());
			return frustum;
		}

		[[nodiscard]] Color MakeFromColorTemperature(float Temp) noexcept;
	}
}