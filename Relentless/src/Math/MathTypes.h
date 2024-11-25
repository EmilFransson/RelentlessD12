#pragma once
#include "../../vendor/includes/DirectXTK/SimpleMath.h"

namespace Relentless
{
	using BoundingBox			= DirectX::BoundingBox;
	using OrientedBoundingBox	= DirectX::BoundingOrientedBox;
	using BoundingFrustum		= DirectX::BoundingFrustum;
	using BoundingSphere		= DirectX::BoundingSphere;
	using Vector2				= DirectX::SimpleMath::Vector2;
	using Vector3				= DirectX::SimpleMath::Vector3;
	using Vector4				= DirectX::SimpleMath::Vector4;
	using Matrix				= DirectX::SimpleMath::Matrix;
	using Quaternion			= DirectX::SimpleMath::Quaternion;
	using Color					= DirectX::SimpleMath::Color;
	using Ray					= DirectX::SimpleMath::Ray;

	template<typename T>
	struct TRect
	{
		TRect()
			: Left(T()), Top(T()), Right(T()), Bottom(T())
		{}

		TRect(const T left, const T top, const T right, const T bottom)
			: Left(left), Top(top), Right(right), Bottom(bottom)
		{}

		template<typename U>
		TRect(const TRect<U>& other)
			: Left((T)other.Left), Top((T)other.Top), Right((T)other.Right), Bottom((T)other.Bottom)
		{}

		T Left;
		T Top;
		T Right;
		T Bottom;

		T GetWidth() const { return Right - Left; }
		T GetHeight() const { return Bottom - Top; }
		T GetAspect() const
		{
			return GetWidth() / GetHeight();
		}

		TRect Scale(const float scale) const
		{
			return TRect(Left * scale, Top * scale, Right * scale, Bottom * scale);
		}

		TRect Scale(const float scaleX, const float scaleY) const
		{
			return TRect(Left * scaleX, Top * scaleY, Right * scaleX, Bottom * scaleY);
		}

		bool operator==(const TRect& other) const
		{
			return Left == other.Left && Top == other.Top && Right == other.Right && Bottom == other.Bottom;
		}

		bool operator!=(const TRect& other) const
		{
			return Left != other.Left || Top != other.Top || Right != other.Right || Bottom != other.Bottom;
		}
	};

	using FloatRect = TRect<float>;
	using IntRect = TRect<int>;

	struct Transform
	{
		Vector3 Location	= Vector3::Zero;
		Quaternion Rotation = Quaternion::Identity;
		Vector3 Scale		= Vector3::One;

		Matrix Matrix		= Matrix::Identity;
	};

	struct ViewTransform
	{
		Matrix Projection			= Matrix::Identity;
		Matrix View					= Matrix::Identity;
		Matrix ViewProjection		= Matrix::Identity;
		Matrix ViewProjectionPrev	= Matrix::Identity;
		Matrix ViewInverse			= Matrix::Identity;
		Matrix ProjectionInverse	= Matrix::Identity;

		Vector3 Location			= Vector3::Zero;
		Vector3 LocationPrev		= Vector3::Zero;

		FloatRect Viewport;
		float FoV					= 60.0f * 3.14159265358979323846f / 180.0f;
		float NearPlane				= 0.1f;
		float FarPlane				= 1'000.0f;

		bool IsPerspective			= true;
		
		BoundingFrustum PerspectiveFrustum;
	};
}