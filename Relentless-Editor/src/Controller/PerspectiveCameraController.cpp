#include "PerspectiveCameraController.h"

namespace Relentless
{
	PerspectiveCameraController::PerspectiveCameraController(PerspectiveCamera* pCamera) noexcept
		: m_pCamera{pCamera}
	{
	}

	void PerspectiveCameraController::Update() noexcept
	{
		if (!m_pCamera || !m_Enabled)
			return;

		const bool wasMoving = m_Velocity.LengthSquared() > Math::EPSILON;

		if (AllowsMovement())
		{
			const ECameraControllerNavigationState state = DetermineState();
			if (state != m_CurrentState)
				SetState(state);

			switch (m_CurrentState)
			{
			case ECameraControllerNavigationState::Fly:		OnFly(); break;
			case ECameraControllerNavigationState::Orbit:	OnOrbit(); break;
			case ECameraControllerNavigationState::Dolly:	OnDollyForward(); break;
			case ECameraControllerNavigationState::Pan:		OnPan(); break;
			default:										break;
			}
		}

		const bool isMoving = m_CurrentState != ECameraControllerNavigationState::None && (Mouse::GetDeltaCoordinates() != Vector2i::Zero() || m_Velocity.LengthSquared() > Math::EPSILON);

		if (!wasMoving && isMoving)
			OnBeginTransform();
		else if (wasMoving && !isMoving)
			OnEndTransform();
	}

	bool PerspectiveCameraController::AllowsMovement() const noexcept
	{
		return m_AllowMovement;
	}

	float PerspectiveCameraController::GetFarPlane() const noexcept
	{
		return m_pCamera->GetViewTransform().FarPlane;
	}

	float PerspectiveCameraController::GetHorizontalFoV() const noexcept
	{
		return m_HorizontalFoV;
	}

	float PerspectiveCameraController::GetMaxSpeedMultiplierLimit() const noexcept
	{
		return m_MaxSpeedMultiplierLimit;
	}

	float PerspectiveCameraController::GetMinSpeedMultiplierLimit() const noexcept
	{
		return m_MinSpeedMultiplierLimit;
	}

	float PerspectiveCameraController::GetNearPlane() const noexcept
	{
		return m_pCamera->GetViewTransform().NearPlane;
	}

	float PerspectiveCameraController::GetOrbitDistance() const noexcept
	{
		return m_OrbitDistance;
	}

	ECameraControllerNavigationState PerspectiveCameraController::GetState() const noexcept
	{
		return m_CurrentState;
	}

	float PerspectiveCameraController::GetSpeedMultiplier() const noexcept
	{
		return m_Speed;
	}

	void PerspectiveCameraController::SetCamera(PerspectiveCamera* pCamera) noexcept
	{
		if (m_pCamera == pCamera)
			return;

		m_pCamera = pCamera;
	}

	void PerspectiveCameraController::SetEnabled(bool state) noexcept
	{
		if (m_Enabled == state)
			return;

		m_Enabled = state;
		m_Velocity = Vector3::Zero;

		if (m_Enabled)
			OnEnabled();
		else
			OnDisabled();
	}

	void PerspectiveCameraController::SetAllowMovement(bool state) noexcept
	{
		m_AllowMovement = state;
	}

	void PerspectiveCameraController::SetFarPlane(float farPlane) noexcept
	{
		RLS_ASSERT(m_pCamera, "[PerspectiveCameraController::SetFarPlane] Camera Is Invalid.");
		
		if (m_pCamera->GetViewTransform().NearPlane > farPlane)
			m_pCamera->SetNearPlane(farPlane);

		m_pCamera->SetFarPlane(farPlane);
	}

	void PerspectiveCameraController::SetFoV(float fov) noexcept
	{
		RLS_ASSERT(m_pCamera, "[PerspectiveCameraController::SetFarPlane] Camera Is Invalid.");
		m_pCamera->SetFoV(fov);
	}

	void PerspectiveCameraController::SetHorizontalFoV(float horizontalFoV) noexcept
	{
		m_HorizontalFoV = horizontalFoV;

		const FloatRect viewport = m_pCamera->GetViewTransform().Viewport;
		const float aspectRatio = viewport.GetWidth() / viewport.GetHeight();
		const float verticalFoV = 2.0f * std::atan(std::tan(m_HorizontalFoV * 0.5f) * (1.0f / aspectRatio));
		SetFoV(verticalFoV);
	}

	void PerspectiveCameraController::SetNearPlane(float nearPlane) noexcept
	{
		RLS_ASSERT(m_pCamera, "[PerspectiveCameraController::SetFarPlane] Camera Is Invalid.");
		
		if (m_pCamera->GetViewTransform().FarPlane < nearPlane)
			m_pCamera->SetFarPlane(nearPlane);
		
		m_pCamera->SetNearPlane(nearPlane);
	}

	void PerspectiveCameraController::SetOrbitDistance(float distance) noexcept
	{
		m_OrbitDistance = Math::Max(distance, 0.1f);
	}

	void PerspectiveCameraController::SetSpeed(float speed) noexcept
	{
		m_Speed = speed;
	}

	void PerspectiveCameraController::SetVelocity(const Vector3& velocity) noexcept
	{
		const bool wasMoving = m_Velocity.LengthSquared() > Math::EPSILON;
		m_Velocity = velocity;

		if (wasMoving && velocity == Vector3::Zero)
			OnEndTransform();
	}

	void PerspectiveCameraController::SetViewport(const FloatRect& viewport) noexcept
	{
		RLS_ASSERT(m_pCamera, "[PerspectiveCameraController::SetFarPlane] Camera Is Invalid.");
		m_pCamera->SetViewport(viewport);

		const float aspectRatio = viewport.GetWidth() / viewport.GetHeight();
		const float horizontalFoV = m_HorizontalFoV;
		const float verticalFoV = 2.0f * std::atan(std::tan(horizontalFoV * 0.5f) * (1.0f / aspectRatio));

		SetFoV(verticalFoV);
	}

	void PerspectiveCameraController::StepSpeed(bool forward) noexcept
	{
		const float sign = forward ? 1.0f : -1.0f;

		if (m_SpeedMultiplier < 1.0f)
			m_SpeedMultiplier += 0.1f * sign;
		else if(Math::AreValuesClose(m_Speed, 1.0f))
			m_SpeedMultiplier = forward ? m_Speed + 0.2f : m_Speed - 0.1f;
		else if (m_Speed < 2.0f)
			m_SpeedMultiplier += 0.2f * sign;
		else if (Math::AreValuesClose(m_Speed, 2.0f))
			m_SpeedMultiplier = forward ? m_Speed + 0.5f : m_Speed - 0.2f;
		else
			m_SpeedMultiplier += 0.5f * sign;

		m_SpeedMultiplier = Math::Clamp(m_SpeedMultiplier, m_MinSpeedMultiplierLimit, m_MaxSpeedMultiplierLimit);
	}

	void PerspectiveCameraController::ZoomOrbit(float delta) noexcept
	{
		SetOrbitDistance(m_OrbitDistance + delta);
	}

	void PerspectiveCameraController::SetDamping(float damping) noexcept
	{
		m_Damping = damping;
	}

	ECameraControllerNavigationState PerspectiveCameraController::DetermineState() noexcept
	{
		ECameraControllerNavigationState state = ECameraControllerNavigationState::None;

		if (Mouse::IsButtonDown(RLS_Button::Right))
			state = ECameraControllerNavigationState::Fly;
		else if (Mouse::IsButtonDown(RLS_Button::Left) && Keyboard::IsKeyDown(RLS_Key::Alt))
			state = ECameraControllerNavigationState::Orbit;
		else if (Mouse::IsButtonDown(RLS_Button::Left))
			state = ECameraControllerNavigationState::Dolly;
		else if (Mouse::IsButtonDown(RLS_Button::Wheel))
			state = ECameraControllerNavigationState::Pan;

		return state;
	}

	void PerspectiveCameraController::OnDollyForward() noexcept
	{
		const Quaternion& currentRotation = m_pCamera->GetRotation();

		const Vector2i mouseDelta = Mouse::GetDeltaCoordinates();

		const Quaternion pr = Quaternion::CreateFromYawPitchRoll(mouseDelta.x * Time::GetDeltaTime() * 0.3f, 0, 0);
		Quaternion rotation = currentRotation * pr;
		rotation.Normalize();
		m_pCamera->SetRotation(rotation);

		Vector3 movement = Vector3::Zero;
		movement.z -= static_cast<float>(mouseDelta.y);
		movement = Vector3::Transform(movement, rotation);
		movement.y = 0.0f;

		const float speedMultiplier = 10.0f;

		const Vector3 translation = movement * Time::GetDeltaTime() * speedMultiplier;
		const Vector3& currentLocation = m_pCamera->GetLocation();
		m_pCamera->SetLocation(currentLocation + translation);
	}

	void PerspectiveCameraController::OnFly() noexcept
	{
		const float dt = Time::GetDeltaTime();

		const Quaternion& currentRotation = m_pCamera->GetRotation();

		const Vector2i mouseDelta = Mouse::GetDeltaCoordinates();
		const float mouseSensitivity = 0.7f; 

		const Quaternion pitchRot = Quaternion::CreateFromYawPitchRoll(0.0f, mouseDelta.y * dt * mouseSensitivity, 0.0f);
		const Quaternion yawRot = Quaternion::CreateFromYawPitchRoll(mouseDelta.x * dt * mouseSensitivity, 0.0f, 0.0f);

		Quaternion rotation = pitchRot * currentRotation * yawRot;
		rotation.Normalize();
		m_pCamera->SetRotation(rotation);

		Vector3 inputDir = Vector3::Zero;
		inputDir.x -= static_cast<float>(Keyboard::IsKeyDown(RLS_Key::A));
		inputDir.x += static_cast<float>(Keyboard::IsKeyDown(RLS_Key::D));
		inputDir.z -= static_cast<float>(Keyboard::IsKeyDown(RLS_Key::S));
		inputDir.z += static_cast<float>(Keyboard::IsKeyDown(RLS_Key::W));
		inputDir.y -= static_cast<float>(Keyboard::IsKeyDown(RLS_Key::Q));
		inputDir.y += static_cast<float>(Keyboard::IsKeyDown(RLS_Key::E));

		const bool hasInput = inputDir.LengthSquared() > Math::EPSILON;

		if (hasInput)
			inputDir.Normalize();

		const Vector3 moveDirWorld = hasInput ? Vector3::Transform(inputDir, rotation) : Vector3::Zero;

		const float shiftMultiplier = Keyboard::IsKeyDown(RLS_Key::LShift) ? 2.0f : 1.0f;
		const float targetSpeed = m_Speed * m_SpeedMultiplier * shiftMultiplier;

		const Vector3 desiredVelocity = hasInput ? moveDirWorld * targetSpeed : Vector3::Zero;              

		constexpr float tauAccel = 0.01f;
		constexpr float tauDecel = 0.1f;

		const float currentSpeedSq = m_Velocity.LengthSquared();
		const float desiredSpeedSq = desiredVelocity.LengthSquared();

		const float tau = (desiredSpeedSq > currentSpeedSq) ? tauAccel : tauDecel;

		const float factor = 1.0f - std::exp(-dt / tau);

		m_Velocity += (desiredVelocity - m_Velocity) * factor;

		if (!hasInput)
		{
			constexpr float minSpeed = 0.01f;
			constexpr float minSpeedSq = minSpeed * minSpeed;
			if (m_Velocity.LengthSquared() < minSpeedSq)
				m_Velocity = Vector3::Zero;
		}

		const Vector3 translation = m_Velocity * dt;
		const Vector3& currentLocation = m_pCamera->GetLocation();
		m_pCamera->SetLocation(currentLocation + translation);
	}

	void PerspectiveCameraController::OnOrbitBegin() noexcept
	{
		const Vector3 cameraLocation = m_pCamera->GetLocation();

		Vector3 cameraForward = Vector3::Transform(-Vector3::Forward, m_pCamera->GetRotation());
		cameraForward.Normalize();

		m_OrbitTarget = cameraLocation + (cameraForward * m_OrbitDistance);

		m_OrbitDistance = Vector3::Distance(cameraLocation, m_OrbitTarget);
		m_OrbitRotation = Math::CreateLookToRotation(m_pCamera->GetLocation(), m_OrbitTarget);
		m_OrbitRotation.Normalize();
	}

	void PerspectiveCameraController::OnOrbit() noexcept
	{
		const Vector2i delta = Mouse::GetDeltaCoordinates();

		const float yaw = delta.x * Time::GetDeltaTime();
		const float pitch = delta.y * Time::GetDeltaTime();

		const Quaternion yr = Quaternion::CreateFromYawPitchRoll(0, pitch, 0);
		const Quaternion pr = Quaternion::CreateFromYawPitchRoll(yaw, 0, 0);

		m_OrbitRotation = yr * m_OrbitRotation * pr;
		m_OrbitRotation.Normalize();

		const Vector3 offset = Vector3(0, 0, -m_OrbitDistance);
		const Vector3 worldOffset = Vector3::Transform(offset, m_OrbitRotation);

		const Vector3 newPosition = m_OrbitTarget + worldOffset;

		m_pCamera->SetLocation(newPosition);
		m_pCamera->SetRotation(m_OrbitRotation);
	}

	void PerspectiveCameraController::OnPan() noexcept
	{
		const Vector2i mouseDelta = Mouse::GetDeltaCoordinates();
		Vector3 movement = Vector3::Zero;
		const Quaternion& rotation = m_pCamera->GetRotation();

		movement.x += static_cast<float>(mouseDelta.x);
		movement = Vector3::Transform(movement, rotation);
		movement.y -= static_cast<float>(mouseDelta.y);
		movement.Normalize();

		const float speedMultiplier = 10.0f;

		const Vector3 translation = movement * Time::GetDeltaTime() * speedMultiplier;
		const Vector3& currentLocation = m_pCamera->GetLocation();
		m_pCamera->SetLocation(currentLocation + translation);
	}

	void PerspectiveCameraController::SetState(ECameraControllerNavigationState newState) noexcept
	{
		m_CurrentState = newState;
		
		if (m_CurrentState == ECameraControllerNavigationState::Orbit)
			OnOrbitBegin();
		
		if (m_CurrentState != ECameraControllerNavigationState::Fly)
			m_Velocity = Vector3::Zero;

		OnStateChanged(m_CurrentState);
	}
}
