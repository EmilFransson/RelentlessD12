#include <Relentless.h>

namespace Relentless
{
	enum class ECameraControllerNavigationState: uint8 { None, Fly, Orbit, Dolly, Pan };

	class PerspectiveCameraController
	{
	public:
		PerspectiveCameraController(PerspectiveCamera* pCamera) noexcept;
		void Update() noexcept;

		Broadcaster<void()> OnBeginTransform;
		Broadcaster<void()> OnEndTransform;

		Broadcaster<void()> OnEnabled;
		Broadcaster<void()> OnDisabled;

		Broadcaster<void(ECameraControllerNavigationState newState)> OnStateChanged;

		[[nodiscard]] bool AllowsMovement() const noexcept;

		[[nodiscard]] float GetOrbitDistance() const noexcept;
		[[nodiscard]] ECameraControllerNavigationState GetState() const noexcept;

		void SetCamera(PerspectiveCamera* pCamera) noexcept;
		void SetDamping(float damping) noexcept;
		void SetEnabled(bool state) noexcept;
		void SetAllowMovement(bool state) noexcept;
		void SetFarPlane(float farPlane) noexcept;
		void SetFoV(float fov) noexcept;
		void SetNearPlane(float nearPlane) noexcept;
		void SetOrbitDistance(float distance) noexcept;
		void SetSpeed(float speed) noexcept;
		void SetVelocity(const Vector3& velocity) noexcept;
		void SetViewport(const FloatRect& viewport) noexcept;
		void StepSpeed(bool forward) noexcept;
		void ZoomOrbit(float delta) noexcept;
	private:
		[[nodiscard]] ECameraControllerNavigationState DetermineState() noexcept;

		void OnDollyForward() noexcept;
		void OnFly() noexcept;
		void OnOrbitBegin() noexcept;
		void OnOrbit() noexcept;
		void OnPan() noexcept;

		void SetState(ECameraControllerNavigationState newState) noexcept;
	private:
		PerspectiveCamera* m_pCamera = nullptr;

		Vector3 m_Velocity = Vector3::Zero;
		Vector3 m_OrbitTarget = Vector3::Zero;
		Quaternion m_OrbitRotation = Quaternion::Identity;

		ECameraControllerNavigationState m_CurrentState = ECameraControllerNavigationState::None;

		float m_Damping = 0.2f;
		float m_Speed = 10.0f;
		float m_OrbitDistance = 10.0f;
		float m_OrbitYaw = 0.0f;
		float m_OrbitPitch = 0.0f;

		bool m_Enabled = true;
		bool m_AllowMovement = true;
	};
}