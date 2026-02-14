#pragma once
#include "MathTypes.h"

namespace Relentless
{
	namespace Colors
	{
		NO_DISCARD inline static constexpr Color Normalize(const Color& color) noexcept
		{
			return Color(color.R() / 255.0f, color.G() / 255.0f, color.B() / 255.0f, color.A() / 255.0f);
		}

		NO_DISCARD inline static constexpr Color Normalize(float r, float g, float b, float a) noexcept
		{
			return Color(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
		}

		//General:
		constexpr Color Transparent		= Color(0.0f, 0.0f, 0.0f, 0.0f);
		constexpr Color White			= Color(1.0f, 1.0f, 1.0f, 1.0f);
		constexpr Color Black			= Color(0.0f, 0.0f, 0.0f, 1.0f);
		constexpr Color Red				= Color(1.0f, 0.0f, 0.0f, 1.0f);
		constexpr Color OffRed			= Color(0.8f, 0.0f, 0.0f, 1.0f);
		constexpr Color Green			= Color(0.0f, 1.0f, 0.0f, 1.0f);
		constexpr Color OffGreen		= Color(0.0f, 0.8f, 0.0f, 1.0f);
		constexpr Color Blue			= Color(0.0f, 0.0f, 1.0f, 1.0f);
		constexpr Color OffBlue			= Color(0.0f, 0.0f, 0.8f, 1.0f);
		constexpr Color Yellow			= Color(1.0f, 1.0f, 0.0f, 1.0f);
		constexpr Color Magenta			= Color(1.0f, 0.0f, 1.0f, 1.0f);
		constexpr Color Cyan			= Color(0.0f, 1.0f, 1.0f, 1.0f);
		constexpr Color Gray			= Color(0.5f, 0.5f, 0.5f, 1.0f);
		constexpr Color LightSkyBlue	= Color(0.529411793f, 0.807843208f, 0.980392218f, 1.0f);

		//Rows:
		constexpr Color EvenRowColorDefault					= Normalize(21.0f, 21.0f, 21.0f, 255.0f);
		constexpr Color OddRowColorDefault					= Normalize(26.0f, 26.0f, 26.0f, 255.0f);
		constexpr Color RowHoverColorDefault				= Normalize(36.0f, 36.0f, 36.0f, 255.0f);
		constexpr Color RowFocusedSelectionColorDefault		= Normalize(30.0f, 120.0f, 255.0f, 200.0f);
		constexpr Color RowUnfocusedSelectionColorDefault	= Normalize(64.0f, 87.0f, 111.0f, 255.0f);
		constexpr Color RowAncestorToSelectedColorDefault	= Normalize(44.0f, 50.0f, 58.0f, 255.0f);
		constexpr Color ContextMenuColorDefault				= Normalize(56.0f, 56.0f, 56.0f, 255.0f);

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


		NO_DISCARD inline static float RadToDeg(float radians) noexcept
		{
			constexpr float DEGREES_PER_RADIANS = 180.0f / PI;
			return radians * DEGREES_PER_RADIANS;
		}

		NO_DISCARD inline static Vector3 RadToDeg(const Vector3& radians) noexcept
		{
			constexpr float DEGREES_PER_RADIANS = 180.0f / PI;
			return Vector3(radians.x * DEGREES_PER_RADIANS, radians.y * DEGREES_PER_RADIANS, radians.z * DEGREES_PER_RADIANS);
		}

		NO_DISCARD inline static float RadToDeg360(float radians) noexcept
		{
			float degrees = RadToDeg(radians);
			degrees = fmodf(degrees, 360.0f);
			if (degrees < 0.0f)
				degrees += 360.0f;
			return degrees;
		}

		NO_DISCARD inline static Vector3 RadToDeg360(const Vector3& radians) noexcept
		{
			Vector3 degrees = RadToDeg(radians);

			degrees.x = fmodf(degrees.x, 360.0f);
			if (degrees.x < 0.0f) degrees.x += 360.0f;

			degrees.y = fmodf(degrees.y, 360.0f);
			if (degrees.y < 0.0f) degrees.y += 360.0f;

			degrees.z = fmodf(degrees.z, 360.0f);
			if (degrees.z < 0.0f) degrees.z += 360.0f;
			
			return degrees;
		}

		NO_DISCARD inline static int RandomRange(int min, int max) noexcept
		{
			return min + rand() % (max - min + 1);
		}

		NO_DISCARD inline constexpr static float DegToRad(float degrees) noexcept
		{
			constexpr float RADIANS_PER_DEGREE = PI / 180.0f;
			return degrees * RADIANS_PER_DEGREE;
		}

		NO_DISCARD inline static float Log2f(float value) noexcept
		{
			return std::log2f(value);
		}

		NO_DISCARD inline static float NormalizeDegrees(float degrees) noexcept
		{
			degrees = std::fmod(degrees, 360.0f);
			if (degrees < 0.0f) 
			{
				degrees += 360.0f;
			}
			return degrees;
		}

		NO_DISCARD inline static float Pow2f(float exponent) noexcept
		{
			return std::powf(2, exponent);
		}

		template<typename T>
		NO_DISCARD inline static T Sqrt(T aValue) noexcept
		{
			return static_cast<T>(std::sqrt(static_cast<double>(aValue)));
		}

		// Create left-handed DX style perspective matrix
		// FoV is vertical FoV in radians
		NO_DISCARD inline Matrix CreatePerspectiveMatrix(float FoV, float aspectRatio, float nearZ, float farZ) noexcept
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

		NO_DISCARD inline Matrix CreateLookToMatrix(const Vector3& position, const Vector3& direction, const Vector3& up)
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
		NO_DISCARD inline Quaternion CreateLookToRotation(const Vector3& eye, const Vector3& target)
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

		NO_DISCARD inline BoundingFrustum CreateBoundingFrustum(const Matrix& projection, const Matrix& view) noexcept
		{
			BoundingFrustum frustum;
			BoundingFrustum::CreateFromMatrix(frustum, projection);
			if (frustum.Far < frustum.Near)
				std::swap(frustum.Far, frustum.Near);
			frustum.Transform(frustum, view.Invert());
			return frustum;
		}

		NO_DISCARD inline float Cos(float aRadians) noexcept
		{
			return std::cos(aRadians);
		}

		namespace Photometry
		{
			NO_DISCARD inline float SolidAngle_Cone(float aOuterConeAngleRadians) noexcept
			{
				const float c = Math::Cos(aOuterConeAngleRadians);
				return 2.0f * Math::PI * (1.0f - c);
			}

			NO_DISCARD inline constexpr float CandelaToLumen_Point(float aCandelaValue) noexcept
			{
				return aCandelaValue * (4.0f * Math::PI);
			}

			NO_DISCARD inline float CandelaToLumen_Spot(float aCandelaValue, float aOuterHalfAngleRadians) noexcept
			{
				return aCandelaValue * SolidAngle_Cone(aOuterHalfAngleRadians);
			}

			NO_DISCARD inline float Luminance(const Vector3& rgb)
			{
				return rgb.Dot(Vector3(0.2126f, 0.7152f, 0.0722f));
			}

			NO_DISCARD inline float LumenToCandela_Point(float aLumenValue) noexcept
			{
				return aLumenValue / (4.0f * Math::PI);
			}

			NO_DISCARD inline float LumenToCandela_Spot(float aLumenValue, float aOuterConeAngleRadians) noexcept
			{
				const float omega = SolidAngle_Cone(aOuterConeAngleRadians);
				const float safeOmega = Math::Max(omega, 1e-6f);
				return aLumenValue / safeOmega;
			}
		}

		NO_DISCARD RLS_API Color MakeFromColorTemperature(float aTemp) noexcept;

		NO_DISCARD inline float SpotLightHalfAngleToSolidAngle(float halfAngleRadians) noexcept
		{
			return 2.0f * Math::PI * (1.0f - std::cos(halfAngleRadians));
		}
	}
}