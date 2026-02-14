#pragma once
#include <Relentless.h>

namespace Relentless
{
	enum class ECameraControllerNavigationMode: uint8 { None, Fly, Orbit, Dolly, Pan };

	class PerspectiveCameraController
	{
	public:
		struct Input
		{
			Vector3 MoveAxis		= Vector3::Zero;
			Vector2i MouseDelta		= Vector2i::Zero();
			float MouseWheelDelta	= 0.0f;
		};

		PerspectiveCameraController(PerspectiveCamera* pCamera) noexcept;

		NO_DISCARD float GetFarPlane() const noexcept;
		NO_DISCARD float GetHorizontalFoV() const noexcept;
		NO_DISCARD float GetMaxSpeedMultiplierLimit() const noexcept;
		NO_DISCARD float GetMinSpeedMultiplierLimit() const noexcept;
		NO_DISCARD float GetNearPlane() const noexcept;
		NO_DISCARD float GetOrbitDistance() const noexcept;
		NO_DISCARD ECameraControllerNavigationMode GetMode() const noexcept;
		NO_DISCARD float GetSpeedMultiplier() const noexcept;

		void SetCamera(PerspectiveCamera* pCamera) noexcept;
		void SetDamping(float damping) noexcept;
		void SetEnabled(bool state) noexcept;
		void SetFarPlane(float farPlane) noexcept;
		void SetFoV(float fov) noexcept;
		void SetHorizontalFoV(float horizontalFoV) noexcept;
		void SetMode(ECameraControllerNavigationMode aMode) noexcept;
		void SetNearPlane(float nearPlane) noexcept;
		void SetOrbitDistance(float distance) noexcept;
		void SetSpeed(float speed) noexcept;
		void SetSpeedMultiplier(float aSpeedMultiplier) noexcept;
		void SetVelocity(const Vector3& velocity) noexcept;
		void SetViewport(const FloatRect& viewport) noexcept;
		void StepSpeed(bool forward) noexcept;
		
		void Update(const Input& aInput) noexcept;
		
		void ZoomOrbit(float delta) noexcept;

		Broadcaster<void()> OnBeginTransform;
		Broadcaster<void()> OnEndTransform;
		Broadcaster<void()> OnEnabled;
		Broadcaster<void()> OnDisabled;
		Broadcaster<void(ECameraControllerNavigationMode)> OnModeChanged;
	private:
		void OnDollyForward(const Input& aInput) noexcept;
		void OnFly(const Input& aInput) noexcept;
		void OnOrbitBegin() noexcept;
		void OnOrbit(const Input& aInput) noexcept;
		void OnPan(const Input& aInput) noexcept;
	private:
		PerspectiveCamera* m_pCamera = nullptr;

		Vector3 m_Velocity = Vector3::Zero;
		Vector3 m_OrbitTarget = Vector3::Zero;
		Quaternion m_OrbitRotation = Quaternion::Identity;

		ECameraControllerNavigationMode m_CurrentMode = ECameraControllerNavigationMode::None;

		float m_Damping = 0.2f;
		float m_Speed = 25.0f;
		float m_SpeedMultiplier = 1.0f;
		float m_OrbitDistance = 10.0f;
		float m_HorizontalFoV = Math::DegToRad(60.0f);
		float m_MinSpeedMultiplierLimit = 0.0f;
		float m_MaxSpeedMultiplierLimit = 10.0f;

		bool m_Enabled = true;
	};
}