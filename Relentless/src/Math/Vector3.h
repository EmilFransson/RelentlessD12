#pragma once
namespace Relentless
{
	template<typename DataType>
	concept Numeric = std::is_arithmetic_v<DataType>;

	template<Numeric DataType>
	class Vector3
	{
	public:
		Vector3(DataType x = DataType(), DataType y = DataType(), DataType z = DataType()) noexcept
			: x{x}, y{y}, z{z}{}

		template<typename OtherType>
		Vector3(const Vector3<OtherType>& other)
			: x(static_cast<DataType>(other.x)), y(static_cast<DataType>(other.y)), z(static_cast<DataType>(other.z)) {}

		[[nodiscard]] DirectX::XMVECTOR AsXMVector() const noexcept
		{
			return DirectX::XMVectorSet(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), 0.0f);
		}

		static [[nodiscard]] Vector3<DataType> FromXMVector(const DirectX::XMVECTOR& xmVector) noexcept
		{
			Vector3<DataType> toReturn;
			toReturn.x = static_cast<DataType>(DirectX::XMVectorGetX(xmVector));
			toReturn.y = static_cast<DataType>(DirectX::XMVectorGetY(xmVector));
			toReturn.z = static_cast<DataType>(DirectX::XMVectorGetZ(xmVector));
			
			return toReturn;
		}

		[[nodiscard]] Vector3<DataType> operator+(const Vector3<DataType>& otherVector) const noexcept
		{
			const DirectX::XMVECTOR toReturn = DirectX::XMVectorAdd(AsXMVector(), otherVector.AsXMVector());
			return FromXMVector(toReturn);
		}

		[[nodiscard]] Vector3<DataType> operator-(const Vector3<DataType>& otherVector) const noexcept
		{
			const DirectX::XMVECTOR toReturn = DirectX::XMVectorSubtract(AsXMVector(), otherVector.AsXMVector());
			return FromXMVector(toReturn);
		}

		[[nodiscard]] Vector3<DataType> operator*(DataType scalar) const noexcept
		{
			return Vector3<DataType>(x * scalar, y * scalar, z * scalar);
		}

		[[nodiscard]] Vector3<DataType> operator/(DataType scalar) const noexcept
		{
			return Vector3<DataType>(x / scalar, y / scalar, z / scalar);
		}

		[[nodiscard]] bool operator==(const Vector3<DataType>& otherVector) const noexcept
		{
			return IsEqual(otherVector);
		}

		[[nodiscard]] bool operator!=(const Vector3<DataType>& otherVector) const noexcept
		{
			return !IsEqual(otherVector);
		}

		void Scale(DataType scalar) noexcept
		{
			x *= scalar;
			y *= scalar;
			z *= scalar;
		}

		void Normalize() noexcept
		{
			AssignSelfFromXMVector(DirectX::XMVector3Normalize(AsXMVector()));
		}

		[[nodiscard]] DataType Length() const noexcept
		{
			return static_cast<DataType>(DirectX::XMVectorGetX(DirectX::XMVector3Length(AsXMVector())));
		}

		[[nodiscard]] DataType SquaredLength() const noexcept
		{
			return static_cast<DataType>(DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(AsXMVector())));
		}

		static inline [[nodiscard]] DataType Distance(const Vector3<DataType>& vec1, const Vector3<DataType>& vec2) noexcept
		{
			const DirectX::XMVECTOR delta = DirectX::XMVectorSubtract(vec1.AsXMVector(), vec2.AsXMVector());
			return static_cast<DataType>(DirectX::XMVectorGetX(DirectX::XMVector3Length(delta)));
		}

		static inline [[nodiscard]] DataType SquaredDistance(const Vector3<DataType>& vec1, const Vector3<DataType>& vec2) noexcept
		{
			const DirectX::XMVECTOR delta = DirectX::XMVectorSubtract(vec1.AsXMVector(), vec2.AsXMVector());
			return static_cast<DataType>(DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(delta)));
		}

		static Vector3<DataType> Lerp(const Vector3<DataType>& a, const Vector3<DataType>& b, float t) noexcept 
		{
			return a + (b - a) * t;
		}

		[[nodiscard]] DataType Dot(const Vector3<DataType>& otherVector) const noexcept
		{
			return DirectX::XMVectorGetX(DirectX::XMVector3Dot(AsXMVector(), otherVector.AsXMVector()));
		}

		[[nodiscard]] Vector3<DataType> Cross(const Vector3<DataType>& otherVector) const noexcept
		{
			return FromXMVector(DirectX::XMVector3Cross(AsXMVector(), otherVector.AsXMVector()));
		}

		[[nodiscard]] bool IsParallel(const Vector3<DataType>& otherVector) const noexcept
		{
			const DirectX::XMVECTOR vec1 = DirectX::XMVector3Normalize(AsXMVector());
			const DirectX::XMVECTOR vec2 = DirectX::XMVector3Normalize(otherVector.AsXMVector());

			const float dotProduct = DirectX::XMVectorGetX(DirectX::XMVector3Dot(vec1, vec2));

			constexpr float threshold = 0.999995f;
			return std::fabs(dotProduct) > threshold;
		}

		[[nodiscard]] bool IsOrthogonal(const Vector3<DataType>& otherVector) const noexcept
		{
			const float dotProduct = DirectX::XMVectorGetX(DirectX::XMVector3Dot(AsXMVector(), otherVector.AsXMVector()));
			return std::fabs(dotProduct) < Math::EPSILON;
		}

		[[nodiscard]] float AngleToDegrees(const Vector3<DataType>& otherVector) const noexcept
		{
			return Math::RadToDeg(AngleToRadian(otherVector));
		}

		//xDoty = ||x||*||y||*cos(@), however we use the normalized versions of the vectors, so ||x|| = ~1.0f
		[[nodiscard]] float AngleToRadian(const Vector3<DataType>& otherVector) const noexcept
		{
			Vector3<DataType> vec1Normalized = *this;
			vec1Normalized.Normalize();

			Vector3<DataType> vec2Normalized = otherVector;
			vec2Normalized.Normalize();

			const float dot = vec1Normalized.Dot(vec2Normalized);
			return std::acos(std::clamp(dot, -1.0f, 1.0f));
		}

		[[nodiscard]] Vector3<DataType> Reflect(const Vector3<DataType>& normal) const noexcept
		{
			DirectX::XMVECTOR normalVec = normal.AsXMVector();
			normalVec = DirectX::XMVector3Normalize(normalVec);

			return FromXMVector(DirectX::XMVector3Reflect(AsXMVector(), normalVec));
		}

		[[nodiscard]] Vector3<DataType> Refract(const Vector3<DataType>& normal, float eta) const noexcept 
		{
			const DirectX::XMVECTOR normalVec = DirectX::XMVector3Normalize(normal.AsXMVector());
			const DirectX::XMVECTOR refractedVector = DirectX::XMVector3Refract(AsXMVector(), normalVec, eta);
			return FromXMVector(refractedVector);
		}

		void RotateAroundAxis(const Vector3<DataType>& axis, float angle) noexcept 
		{
			Vector3<DataType> axisNormalized = axis;
			axisNormalized.Normalize();
			
			const DataType cosTheta = static_cast<DataType>(std::cos(angle));
			const DataType sinTheta = static_cast<DataType>(std::sin(angle));
			const Vector3<DataType> result = (*this * cosTheta) + (axis.Cross(*this) * sinTheta) + axisNormalized * (axisNormalized.Dot(*this) * (DataType(1) - cosTheta));
			x = result.x;
			y = result.y;
			z = result.z;
		}

		Vector3<DataType> ProjectOnto(const Vector3<DataType>& other) const 
		{
			const DataType dot = Dot(other);
			const DataType squaredLength = other.SquaredLength();
			return other * (dot / squaredLength);
		}

		[[nodiscard]] bool IsEqual(const Vector3<DataType>& otherVector, DataType epsilon = std::numeric_limits<DataType>::epsilon()) const noexcept
		{
			return	(std::abs(x - otherVector.x) <= epsilon) &&
					(std::abs(y - otherVector.y) <= epsilon) &&
					(std::abs(z - otherVector.z) <= epsilon);
		}

		[[nodiscard]] bool IsZero() const noexcept
		{
			if constexpr (std::is_floating_point<DataType>::value)
			{
				const DataType tolerance = static_cast<DataType>(Math::EPSILON);
				return std::fabs(x) < tolerance &&
					std::fabs(y) < tolerance &&
					std::fabs(z) < tolerance;
			}
			else
			{
				return x == DataType(0) && y == DataType(0) && z == DataType(0);
			}
		}

		[[nodiscard]] bool IsNormalized() const noexcept
		{
			return std::fabs(static_cast<float>(Length()) - 1.0f) < Math::EPSILON;
		}

	public:
		DataType x;
		DataType y;
		DataType z;

		static const Vector3 Up;
		static const Vector3 Down;
		static const Vector3 Right;
		static const Vector3 Left;
		static const Vector3 Forward;
		static const Vector3 Back;
	
	private:
		void AssignSelfFromXMVector(const DirectX::XMVECTOR& xmVector) noexcept
		{
			x = static_cast<DataType>(DirectX::XMVectorGetX(xmVector));
			y = static_cast<DataType>(DirectX::XMVectorGetY(xmVector));
			z = static_cast<DataType>(DirectX::XMVectorGetZ(xmVector));
		}
	};

	template<>
	const Vector3<float> Vector3<float>::Up(0.0f, 1.0f, 0.0f);

	template<>
	const Vector3<float> Vector3<float>::Down(0.0f, -1.0f, 0.0f);
	
	template<>
	const Vector3<float> Vector3<float>::Right(1.0f, 0.0f, 0.0f);
	
	template<>
	const Vector3<float> Vector3<float>::Left(-1.0f, 0.0f, 0.0f);
	
	template<>
	const Vector3<float> Vector3<float>::Forward(0.0f, 0.0f, 1.0f);
	
	template<>
	const Vector3<float> Vector3<float>::Back(0.0f, 0.0f, -1.0f);

	template<>
	const Vector3<double> Vector3<double>::Up(0.0, 1.0, 0.0);

	template<>
	const Vector3<double> Vector3<double>::Down(0.0, -1.0, 0.0);

	template<>
	const Vector3<double> Vector3<double>::Right(1.0, 0.0, 0.0);

	template<>
	const Vector3<double> Vector3<double>::Left(-1.0, 0.0, 0.0);

	template<>
	const Vector3<double> Vector3<double>::Forward(0.0, 0.0, 1.0);

	template<>
	const Vector3<double> Vector3<double>::Back(0.0, 0.0, -1.0);

	template<>
	const Vector3<int> Vector3<int>::Up(0, 1, 0);

	template<>
	const Vector3<int> Vector3<int>::Down(0, -1, 0);

	template<>
	const Vector3<int> Vector3<int>::Right(1, 0, 0);

	template<>
	const Vector3<int> Vector3<int>::Left(-1, 0, 0);

	template<>
	const Vector3<int> Vector3<int>::Forward(0, 0, 1);

	template<>
	const Vector3<int> Vector3<int>::Back(0, 0, -1);

	using Vector3i = Vector3<int>;
	using Vector3f = Vector3<float>;
	using Vector3d = Vector3<double>;

	class Quaternion
	{
	public:
		Quaternion(float x = 0.0f, float y = 0.0f, float z = 0.0f, float w = 1.0f)
			: Q(x,y,z,w){}

		[[nodiscard]] static Quaternion Identity() noexcept
		{
			return Quaternion(0.0f, 0.0f, 0.0f, 1.0f);
		}

		[[nodiscard]] Quaternion operator*(const Quaternion& other) const noexcept 
		{
			const DirectX::XMVECTOR resultVec = DirectX::XMQuaternionMultiply(AsXMVector(), other.AsXMVector());
			return FromXMVector(DirectX::XMQuaternionNormalize(resultVec));
		}

		void Normalize() noexcept
		{
			auto vec = DirectX::XMLoadFloat4(&Q);
			vec = DirectX::XMQuaternionNormalize(vec);
			DirectX::XMStoreFloat4(&Q, vec);
		}

		[[nodiscard]] DirectX::XMMATRIX ToRotationXMMatrix() const noexcept
		{
			auto quat = DirectX::XMLoadFloat4(&Q);
			return DirectX::XMMatrixRotationQuaternion(quat);
		}

		[[nodiscard]] DirectX::XMVECTOR AsXMVector() const noexcept
		{
			return DirectX::XMVectorSet(Q.x, Q.y, Q.z, Q.w);
		}

		[[nodiscard]] static Quaternion FromXMVector(const DirectX::XMVECTOR& xmVector) noexcept
		{
			Quaternion quat;
			quat.Q.x = DirectX::XMVectorGetX(xmVector);
			quat.Q.y = DirectX::XMVectorGetY(xmVector);
			quat.Q.z = DirectX::XMVectorGetZ(xmVector);
			quat.Q.w = DirectX::XMVectorGetW(xmVector);
			return quat;
		}

		[[nodiscard]] static Quaternion FromEuler(float roll, float pitch, float yaw) noexcept 
		{
			auto quat = DirectX::XMQuaternionRotationRollPitchYaw(pitch, yaw, roll);
			DirectX::XMFLOAT4 result;
			DirectX::XMStoreFloat4(&result, quat);
			return Quaternion(result.x, result.y, result.z, result.w);
		}

		[[nodiscard]] Vector3f ToEulerAngles() const noexcept
		{
			DirectX::XMFLOAT4X4 matrix;
			DirectX::XMStoreFloat4x4(&matrix, ToRotationXMMatrix());

			const float pitch = DirectX::XMScalarASin(-matrix._32);

			const DirectX::XMVECTOR from(DirectX::XMVectorSet(matrix._12, matrix._31, 0.0f, 0.0f));
			const DirectX::XMVECTOR to(DirectX::XMVectorSet(matrix._22, matrix._33, 0.0f, 0.0f));
			const DirectX::XMVECTOR res(DirectX::XMVectorATan2(from, to));

			const float roll = DirectX::XMVectorGetX(res);
			const float yaw = DirectX::XMVectorGetY(res);

			Vector3f euler;
			euler.x = Math::RadToDeg(pitch);
			euler.y = Math::RadToDeg(yaw);
			euler.z = Math::RadToDeg(roll);

			return euler;
		}

		[[nodiscard]] static Quaternion AngleAxis(float angleRadians, const Vector3f& axis) noexcept
		{
			const DirectX::XMVECTOR rotation = DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(axis.x, axis.y, axis.z, 0.0f), angleRadians);
			return FromXMVector(rotation);
		}

		[[nodiscard]] static Quaternion Slerp(const Quaternion& start, const Quaternion& end, float t) 
		{
			auto startVec = DirectX::XMLoadFloat4(&start.Q);
			auto endVec = DirectX::XMLoadFloat4(&end.Q);
			auto resultVec = DirectX::XMQuaternionSlerp(startVec, endVec, t);
			DirectX::XMFLOAT4 result;
			DirectX::XMStoreFloat4(&result, resultVec);
			return Quaternion(result.x, result.y, result.z, result.w);
		}
	public:
		DirectX::XMFLOAT4 Q;
	};

	class Matrix4x4
	{
	public:
		explicit Matrix4x4(const DirectX::XMFLOAT4X4& matrix) noexcept
			: M{matrix}{}

		explicit Matrix4x4(const DirectX::XMMATRIX& xmMatrix) noexcept
		{
			DirectX::XMStoreFloat4x4(&M, xmMatrix);
		}

		Matrix4x4() noexcept
		{
			DirectX::XMStoreFloat4x4(&M, DirectX::XMMatrixIdentity());
		}

		[[nodiscard]] Matrix4x4 operator*(const Matrix4x4& rhs) const noexcept
		{
			return FromXMMatrix(DirectX::XMMatrixMultiply(AsXMMatrix(), rhs.AsXMMatrix()));
		}

		[[nodiscard]] DirectX::XMMATRIX AsXMMatrix() const noexcept
		{
			return DirectX::XMLoadFloat4x4(&M);
		}

		static [[nodiscard]] Matrix4x4 FromXMMatrix(const DirectX::XMMATRIX& xmMatrix) noexcept
		{
			DirectX::XMFLOAT4X4 m;
			DirectX::XMStoreFloat4x4(&m, xmMatrix);
			return Matrix4x4(m);
		}

		static [[nodiscard]] Matrix4x4 Identity() noexcept
		{
			return Matrix4x4(DirectX::XMMatrixIdentity());
		}

		static [[nodiscard]] Matrix4x4 TransposeFrom(const Matrix4x4& matrix) noexcept
		{
			return FromXMMatrix(DirectX::XMMatrixTranspose(matrix.AsXMMatrix()));
		}

		static [[nodiscard]] Matrix4x4 InverseFrom(const Matrix4x4& matrix) noexcept
		{
			return FromXMMatrix(DirectX::XMMatrixInverse(nullptr, matrix.AsXMMatrix()));
		}

		static [[nodiscard]] float DeterminantFrom(const Matrix4x4& matrix) noexcept
		{
			return DirectX::XMVectorGetX(DirectX::XMMatrixDeterminant(matrix.AsXMMatrix()));
		}

		[[nodiscard]] bool IsIdentity() const noexcept
		{
			return DirectX::XMMatrixIsIdentity(AsXMMatrix());
		}

		void SetTranslation(float x, float y, float z) noexcept
		{
			M._41 = x;
			M._42 = y;
			M._43 = z;
		}

		void SetTranslation(const Vector3f& translation) noexcept
		{
			M._41 = translation.x;
			M._42 = translation.y;
			M._43 = translation.z;
		}

		void SetUniformTranslation(float scalar) noexcept
		{
			M._41 = scalar;
			M._42 = scalar;
			M._43 = scalar;
		}

		[[nodiscard]] bool Decompose(Vector3f& outScale, Quaternion& outRotation, Vector3f& outTranslation) noexcept 
		{
			DirectX::XMVECTOR scale;
			DirectX::XMVECTOR translation;
			DirectX::XMVECTOR rotation;

			DirectX::XMMATRIX mat = DirectX::XMLoadFloat4x4(&M);
			const bool result = DirectX::XMMatrixDecompose(&scale, &rotation, &translation, mat);
			if (!result) 
				return false;

			outScale = Vector3f::FromXMVector(scale);
			outRotation = Quaternion::FromXMVector(rotation);
			outTranslation = Vector3f::FromXMVector(translation);
			return true;
		}

		void Recompose(const Vector3f& scale, const Quaternion& rotation, const Vector3f& translation) noexcept
		{
			DirectX::XMVECTOR s = DirectX::XMVectorSet(scale.x, scale.y, scale.z, 0.0f);
			DirectX::XMVECTOR t = DirectX::XMVectorSet(translation.x, translation.y, translation.z, 0.0f);
			DirectX::XMMATRIX mat = DirectX::XMMatrixAffineTransformation(s, DirectX::XMVectorZero(), rotation.AsXMVector(), t);
			DirectX::XMStoreFloat4x4(&M, mat);
		}

		void RotateX(float angleRadians) noexcept 
		{
			Vector3f currentScale;
			Vector3f currentTranslation;
			Quaternion currentRotation;
			if (Decompose(currentScale, currentRotation, currentTranslation))
			{
				const Quaternion rotationAroundX = Quaternion::AngleAxis(angleRadians, Vector3f::Right);
				const Quaternion newRotation = currentRotation * rotationAroundX;
				Recompose(currentScale, newRotation, currentTranslation);
			}
		}

		void RotateY(float angleRadians) noexcept
		{
			Vector3f currentScale;
			Vector3f currentTranslation;
			Quaternion currentRotation;
			if (Decompose(currentScale, currentRotation, currentTranslation))
			{
				const Quaternion rotationAroundY = Quaternion::AngleAxis(angleRadians, Vector3f::Up);
				const Quaternion newRotation = currentRotation * rotationAroundY;
				Recompose(currentScale, newRotation, currentTranslation);
			}
		}

		void RotateZ(float angleRadians) noexcept
		{
			Vector3f currentScale;
			Vector3f currentTranslation;
			Quaternion currentRotation;
			if (Decompose(currentScale, currentRotation, currentTranslation))
			{
				const Quaternion rotationAroundZ = Quaternion::AngleAxis(angleRadians, Vector3f::Forward);
				const Quaternion newRotation = currentRotation * rotationAroundZ;
				Recompose(currentScale, newRotation, currentTranslation);
			}
		}

		void RotateRollPitchYaw(float roll, float pitch, float yaw) noexcept 
		{
			Vector3f currentScale;
			Vector3f currentTranslation;
			Quaternion currentRotation;
			if (Decompose(currentScale, currentRotation, currentTranslation))
			{
				const Quaternion rotationToApply = Quaternion::FromEuler(roll, pitch, yaw);
				const Quaternion newRotation = currentRotation * rotationToApply;
				Recompose(currentScale, newRotation, currentTranslation);
			}
		}

		void RotateByVector(const Vector3f& rotationVector, float angleRadians) noexcept
		{
			Vector3f currentScale;
			Vector3f currentTranslation;
			Quaternion currentRotation;
			if (Decompose(currentScale, currentRotation, currentTranslation))
			{
				const Quaternion rotationToApply = Quaternion::AngleAxis(angleRadians, rotationVector);
				const Quaternion newRotation = currentRotation * rotationToApply;
				Recompose(currentScale, newRotation, currentTranslation);
			}
		}

		void RotateByQuaternion(const Quaternion& rotationToApply) noexcept
		{
			Vector3f currentScale;
			Vector3f currentTranslation;
			Quaternion currentRotation;
			if (Decompose(currentScale, currentRotation, currentTranslation))
			{
				const Quaternion newRotation = currentRotation * rotationToApply;
				Recompose(currentScale, newRotation, currentTranslation);
			}
		}

		void SetRotationX(float angleRadians) noexcept 
		{
			Vector3f scale;
			Vector3f translation;
			Quaternion rotation;
			if (!Decompose(scale, rotation, translation)) 
				return;

			rotation = Quaternion::FromEuler(0.0f, angleRadians, 0.0f);
			Recompose(scale, rotation, translation);
		}

		void SetRotationY(float angleRadians) noexcept
		{
			Vector3f scale;
			Vector3f translation;
			Quaternion rotation;
			if (!Decompose(scale, rotation, translation))
				return;

			rotation = Quaternion::FromEuler(0.0f, 0.0f, angleRadians);
			Recompose(scale, rotation, translation);
		}

		void SetRotationZ(float angleRadians) noexcept
		{
			Vector3f scale;
			Vector3f translation;
			Quaternion rotation;
			if (!Decompose(scale, rotation, translation))
				return;

			rotation = Quaternion::FromEuler(angleRadians, 0.0f, 0.0f);
			Recompose(scale, rotation, translation);
		}

		void SetRotationFromVector(const Vector3f& rotationVector) noexcept
		{
			Vector3f scale;
			Vector3f translation;
			Quaternion rotation;
			if (!Decompose(scale, rotation, translation))
				return;

			rotation = Quaternion::FromEuler(rotationVector.z, rotationVector.x, rotationVector.y);
			Recompose(scale, rotation, translation);
		}

		void SetRotationRollPitchYaw(float roll, float pitch, float yaw) noexcept
		{
			Vector3f scale;
			Vector3f translation;
			Quaternion rotation;
			if (!Decompose(scale, rotation, translation))
				return;

			rotation = Quaternion::FromEuler(roll, pitch, yaw);
			Recompose(scale, rotation, translation);
		}

		void SetScale(float scaleX, float scaleY, float scaleZ) noexcept 
		{
			DirectX::XMMATRIX current = DirectX::XMLoadFloat4x4(&M);

			current.r[0] = DirectX::XMVector3Normalize(current.r[0]);
			current.r[1] = DirectX::XMVector3Normalize(current.r[1]);
			current.r[2] = DirectX::XMVector3Normalize(current.r[2]);

			current.r[0] = DirectX::XMVectorScale(current.r[0], scaleX);
			current.r[1] = DirectX::XMVectorScale(current.r[1], scaleY);
			current.r[2] = DirectX::XMVectorScale(current.r[2], scaleZ);

			DirectX::XMStoreFloat4x4(&M, current);
		}

		void SetUniformScale(float scale) noexcept
		{
			
			DirectX::XMMATRIX current = DirectX::XMLoadFloat4x4(&M);
			
			current.r[0] = DirectX::XMVector3Normalize(current.r[0]);
			current.r[1] = DirectX::XMVector3Normalize(current.r[1]);
			current.r[2] = DirectX::XMVector3Normalize(current.r[2]);

			current.r[0] = DirectX::XMVectorScale(current.r[0], scale);
			current.r[1] = DirectX::XMVectorScale(current.r[1], scale);
			current.r[2] = DirectX::XMVectorScale(current.r[2], scale);

			DirectX::XMStoreFloat4x4(&M, current);
		}

		void Scale(float scaleFactorX, float scaleFactorY, float scaleFactorZ) noexcept 
		{
			DirectX::XMMATRIX current = DirectX::XMLoadFloat4x4(&M);
			
			current.r[0] = DirectX::XMVectorScale(current.r[0], scaleFactorX);
			current.r[1] = DirectX::XMVectorScale(current.r[1], scaleFactorY);
			current.r[2] = DirectX::XMVectorScale(current.r[2], scaleFactorZ);

			DirectX::XMStoreFloat4x4(&M, current);
		}

	public:
		DirectX::XMFLOAT4X4 M;
	};


}