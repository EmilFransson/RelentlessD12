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
		constexpr Color OffRed = Color(0.8f, 0.0f, 0.0f, 1.0f);
		constexpr Color Green = Color(0.0f, 1.0f, 0.0f, 1.0f);
		constexpr Color OffGreen = Color(0.0f, 0.8f, 0.0f, 1.0f);
		constexpr Color Blue = Color(0.0f, 0.0f, 1.0f, 1.0f);
		constexpr Color OffBlue = Color(0.0f, 0.0f, 0.8f, 1.0f);
		constexpr Color Yellow = Color(1.0f, 1.0f, 0.0f, 1.0f);
		constexpr Color Magenta = Color(1.0f, 0.0f, 1.0f, 1.0f);
		constexpr Color Cyan = Color(0.0f, 1.0f, 1.0f, 1.0f);
		constexpr Color Gray = Color(0.5f, 0.5f, 0.5f, 1.0f);
		constexpr Color LightSkyBlue = Color(0.529411793f, 0.807843208f, 0.980392218f, 1.0f);

		inline static [[nodiscard]] Color Normalize(const Color& color) noexcept
		{
			return Color(color.R() / 255.0f, color.G() / 255.0f, color.B() / 255.0f, color.A() / 255.0f);
		}

		inline static [[nodiscard]] Color Normalize(float r, float g, float b, float a) noexcept
		{
			return Color(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
		}
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

		constexpr float PhotopicEfficacy = 683.0f; // lm per watt

		template<typename T>
		T AlignUp(T value, T alignment) noexcept
		{
			return (value + ((T)alignment - 1)) & ~(alignment - 1);
		}

		template<typename T>
		T AreValuesClose(T value, T alignment) noexcept
		{
			return std::abs(value - alignment) < EPSILON;
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

		inline [[nodiscard]] static Vector3 RadToDeg(Vector3 radians) noexcept
		{
			constexpr float DEGREES_PER_RADIANS = 180.0f / PI;
			return Vector3(radians.x * DEGREES_PER_RADIANS, radians.y * DEGREES_PER_RADIANS, radians.z * DEGREES_PER_RADIANS);
		}

		inline [[nodiscard]] static float RadToDeg360(float radians) noexcept
		{
			float degrees = RadToDeg(radians);
			degrees = fmodf(degrees, 360.0f);
			if (degrees < 0.0f)
				degrees += 360.0f;
			return degrees;
		}

		inline [[nodiscard]] constexpr static float DegToRad(float degrees) noexcept
		{
			constexpr float RADIANS_PER_DEGREE = PI / 180.0f;
			return degrees * RADIANS_PER_DEGREE;
		}

		inline [[nodiscard]] static float Log2f(float value) noexcept
		{
			return std::log2f(value);
		}

		inline [[nodiscard]] static float NormalizeDegrees(float degrees) noexcept
		{
			degrees = std::fmod(degrees, 360.0f);
			if (degrees < 0.0f) 
			{
				degrees += 360.0f;
			}
			return degrees;
		}

		inline [[nodiscard]] static float Pow2f(float exponent) noexcept
		{
			return std::powf(2, exponent);
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

		// Create left-handed DX style look-to quaternion
		inline [[nodiscard]] Quaternion CreateLookToRotation(const Vector3& eye, const Vector3& target)
		{
			Vector3 forward = target - eye;
			forward.Normalize();

			Vector3 right = Vector3::Up.Cross(forward);
			right.Normalize();

			const Vector3 correctedUp = forward.Cross(right);

			const Matrix orientationMatrix = Matrix(right, correctedUp, forward);
			Quaternion rotation = Quaternion::CreateFromRotationMatrix(orientationMatrix);
			rotation.Normalize();

			return rotation;
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

		inline constexpr [[nodiscard]] float CandelaToRadiantIntensity(float candela) noexcept
		{
			return candela / Math::PhotopicEfficacy;
		}

		inline constexpr [[nodiscard]] float LumenToRadiantIntensity(float lumen, float solidAngleSr = 4.0f * Math::PI) noexcept
		{
			return (lumen / Math::PhotopicEfficacy) / solidAngleSr;
		}

		inline constexpr [[nodiscard]] float LuxToRadiantIrradiance(float lux) noexcept
		{
			return lux / Math::PhotopicEfficacy;
		}

		inline constexpr [[nodiscard]] float RadiantIrradianceToLux(float radiantIrradiance) noexcept
		{
			return radiantIrradiance * Math::PhotopicEfficacy;
		}

		inline constexpr [[nodiscard]] float RadiantIntensityToCandela(float radiantIntensity) noexcept
		{
			return radiantIntensity * Math::PhotopicEfficacy;
		}

		inline constexpr [[nodiscard]] float RadiantIntensityToLumen(float radiantIntensity, float solidAngleSr = 4.0f * Math::PI) noexcept
		{
			return radiantIntensity * solidAngleSr * Math::PhotopicEfficacy;
		}

		[[nodiscard]] Color MakeFromColorTemperature(float Temp) noexcept;

		inline [[nodiscard]] float SpotLightHalfAngleToSolidAngle(float halfAngleRadians) noexcept
		{
			return 2.0f * Math::PI * (1.0f - std::cos(halfAngleRadians));
		}
	}
}