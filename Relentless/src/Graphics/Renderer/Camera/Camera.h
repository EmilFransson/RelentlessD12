#pragma once
#include "Core/DLLExport.h"

#include "Graphics/Renderer/RenderTypes.h"

#include "Math/MathTypes.h"

namespace Relentless
{
	class RLS_API Camera
	{
	public:
		Camera() noexcept = default;
		virtual ~Camera() noexcept = default;

		void Update() noexcept;
		void SetLocation(const Vector3& location) noexcept;
		void SetRotation(const Quaternion& rotation) noexcept;
		void SetViewport(const FloatRect& viewport) noexcept;
		void SetFoV(float fov) noexcept;
		void SetNearPlane(float near) noexcept;
		void SetFarPlane(float far) noexcept;

		[[nodiscard]] Vector3 GetForwardVector() const noexcept;
		[[nodiscard]] const Vector3& GetLocation() const noexcept;
		[[nodiscard]] const Quaternion& GetRotation() const noexcept;
		[[nodiscard]] const ViewTransform& GetViewTransform() const noexcept;
		[[nodiscard]] ViewTransform& GetViewTransform() noexcept;
	protected:
		Vector3 m_Location = Vector3::Zero;
		Quaternion m_Rotation = Quaternion::Identity;
	private:
		ViewTransform m_Transform;
	};
}