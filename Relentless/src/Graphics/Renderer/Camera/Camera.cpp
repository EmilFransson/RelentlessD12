#include "Camera.h"
namespace Relentless
{
	void Camera::Update() noexcept
	{
		m_Transform.LocationPrev = m_Transform.Location;

		m_Transform.ViewToWorld = Matrix::CreateFromQuaternion(m_Rotation) * Matrix::CreateTranslation(m_Location);
		m_Transform.WorldToView = m_Transform.ViewToWorld.Invert();

		m_Transform.ViewToClip = Math::CreatePerspectiveMatrix(m_Transform.FoV, m_Transform.Viewport.GetAspect(), m_Transform.NearPlane, m_Transform.FarPlane);
		m_Transform.ClipToView = m_Transform.ViewToClip.Invert();

		m_Transform.WorldToClipPrev = m_Transform.WorldToClip;
		m_Transform.WorldToClip = m_Transform.WorldToView * m_Transform.ViewToClip;

		m_Transform.PerspectiveFrustum = Math::CreateBoundingFrustum(m_Transform.ViewToClip, m_Transform.WorldToView);
		m_Transform.Location = m_Location;
	}

	Vector3 Camera::GetForwardVector() const noexcept
	{
		// Extract forward vector from ViewToWorld matrix (third column)
		Vector3 forward(
			m_Transform.ViewToWorld.m[2][0],
			m_Transform.ViewToWorld.m[2][1],
			m_Transform.ViewToWorld.m[2][2]
		);
		forward.Normalize();

		return forward;
	}

	void Camera::SetLocation(const Vector3& location) noexcept
	{
		m_Location = location;
	}

	void Camera::SetRotation(const Quaternion& rotation) noexcept
	{
		m_Rotation = rotation;
	}

	void Camera::SetViewport(const FloatRect& viewport) noexcept
	{
		m_Transform.Viewport = viewport;
	}

	void Camera::SetFoV(float fov) noexcept
	{
		m_Transform.FoV = fov;
	}

	void Camera::SetNearPlane(float nearPlane) noexcept
	{
		m_Transform.NearPlane = nearPlane;
	}

	void Camera::SetFarPlane(float farPlane) noexcept
	{
		m_Transform.FarPlane = farPlane;
	}

	const Vector3& Camera::GetLocation() const noexcept
	{
		return m_Transform.Location;
	}

	const Quaternion& Camera::GetRotation() const noexcept
	{
		return m_Rotation;
	}

	const ViewTransform& Camera::GetViewTransform() const noexcept
	{
		return m_Transform;
	}

}