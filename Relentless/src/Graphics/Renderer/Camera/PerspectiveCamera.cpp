#include "PerspectiveCamera.h"
#include "Core/Time.h"
#include "Input/Keyboard.h"
#include "Input/Mouse.h"
namespace Relentless
{
	using namespace DirectX;

	PerspectiveCamera::PerspectiveCamera() noexcept
	{
		m_pConstantBufferSet = std::make_unique<ConstantBufferSet>("Perspective Camera CB Set", sizeof(Vector3));
	}

	void PerspectiveCamera::Update() noexcept
	{
		Vector3 movement = Vector3::Zero;
		bool ease = false;

		if (Mouse::IsButtonPressed(RLS_BUTTON::Right))
		{
			if (!ImGui::IsAnyItemActive())
			{
				const Vector2i mouseDelta = Mouse::GetDeltaCoordinates();
				const Quaternion yr = Quaternion::CreateFromYawPitchRoll(0, mouseDelta.y * Time::GetDeltaTime() * 0.3f, 0);
				const Quaternion pr = Quaternion::CreateFromYawPitchRoll(mouseDelta.x * Time::GetDeltaTime() * 0.3f, 0, 0);
				m_Rotation = yr * m_Rotation * pr;
				m_Rotation.Normalize();
			}

			movement.x -= static_cast<float>(Keyboard::IsKeyPressed(RLS_KEY::A));
			movement.x += static_cast<float>(Keyboard::IsKeyPressed(RLS_KEY::D));
			movement.z -= static_cast<float>(Keyboard::IsKeyPressed(RLS_KEY::S));
			movement.z += static_cast<float>(Keyboard::IsKeyPressed(RLS_KEY::W));
			movement.y -= static_cast<float>(Keyboard::IsKeyPressed(RLS_KEY::Q));
			movement.y += static_cast<float>(Keyboard::IsKeyPressed(RLS_KEY::E));
			movement.Normalize();

			movement = Vector3::Transform(movement, m_Rotation);
			ease = true;
		}
		else if (Mouse::IsButtonPressed(RLS_BUTTON::Left))
		{
			const Vector2i mouseDelta = Mouse::GetDeltaCoordinates();
			if (!ImGui::IsAnyItemActive())
			{
				const Quaternion pr = Quaternion::CreateFromYawPitchRoll(mouseDelta.x * Time::GetDeltaTime() * 0.3f, 0, 0);
				m_Rotation = m_Rotation * pr;
				m_Rotation.Normalize();

				movement.z -= static_cast<float>(mouseDelta.y);
				movement = Vector3::Transform(movement, m_Rotation);
				movement.y = 0.0f;
			}
		}
		else if (Mouse::IsButtonPressed(RLS_BUTTON::Wheel))
		{
			const Vector2i mouseDelta = Mouse::GetDeltaCoordinates();

			movement.x += static_cast<float>(mouseDelta.x);
			movement = Vector3::Transform(movement, m_Rotation);
			movement.y -= static_cast<float>(mouseDelta.y);
			movement.Normalize();
		}
		
		float speedMultiplier = Keyboard::IsKeyPressed(RLS_KEY::LShift) ? 2.0f : 1.0f;
		speedMultiplier *= 5.0f;

		if (ease)
			m_Velocity = Vector3::SmoothStep(m_Velocity, movement, 0.1f);
		else
			m_Velocity = movement;
		
		m_Location += m_Velocity * Time::GetDeltaTime() * speedMultiplier;

		Camera::Update();
	}

	std::shared_ptr<PerspectiveCamera> PerspectiveCamera::Create() noexcept
	{
		return std::make_shared<PerspectiveCamera>();
	}
}