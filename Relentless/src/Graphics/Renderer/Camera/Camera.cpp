#include "Camera.h"
namespace Relentless
{
	void Camera::Update() noexcept
	{
		m_Transform.LocationPrev = m_Transform.Location;
		m_Transform.ViewProjectionPrev = m_Transform.ViewProjection;
		
		m_Transform.ViewInverse = Matrix::CreateFromQuaternion(m_Rotation) * Matrix::CreateTranslation(m_Location);
		m_Transform.ViewInverse.Invert(m_Transform.View);

		m_Transform.Projection = Math::CreatePerspectiveMatrix(m_Transform.FoV, m_Transform.Viewport.GetAspect(), m_Transform.NearPlane, m_Transform.FarPlane);
		m_Transform.Projection.Invert(m_Transform.ProjectionInverse);
		m_Transform.ViewProjection = m_Transform.View * m_Transform.Projection;

		m_Transform.PerspectiveFrustum = Math::CreateBoundingFrustum(m_Transform.Projection, m_Transform.View);
		m_Transform.Location = m_Location;
	}

	void Camera::SetLocation(const Vector3& location) noexcept
	{
		m_Transform.Location = location;
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