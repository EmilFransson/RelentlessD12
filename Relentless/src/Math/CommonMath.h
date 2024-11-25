#pragma once
#include "MathTypes.h"

namespace Relentless
{
	namespace Math
	{
		constexpr float PI = 3.14159265358979323846f;
		constexpr float PI_DIV_4 = 0.78539816339744830961f;
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

		inline [[nodiscard]] BoundingFrustum CreateBoundingFrustum(const Matrix& projection, const Matrix& view) noexcept
		{
			BoundingFrustum frustum;
			BoundingFrustum::CreateFromMatrix(frustum, projection);
			if (frustum.Far < frustum.Near)
				std::swap(frustum.Far, frustum.Near);
			frustum.Transform(frustum, view.Invert());
			return frustum;
		}
	}
}