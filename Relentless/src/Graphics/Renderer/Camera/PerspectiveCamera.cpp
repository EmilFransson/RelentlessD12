#include "PerspectiveCamera.h"
#include "../../../Input/Keyboard.h"
#include "../../../Input/Mouse.h"
namespace Relentless
{
	PerspectiveCamera::PerspectiveCamera(const DirectX::XMVECTOR& position, const uint32_t width, const uint32_t height) noexcept
		: m_UpVector{ DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f) },
		m_RightVector{ DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f) },
		m_Yaw{ 134.0f },
		m_Pitch{ -35.0f },
		m_CameraSpeed{ 10.0f },
		m_TiltSensitivity{ 0.25f },
		m_FieldOfViewDegrees{ 60.0f },
		m_NearPlane{0.3f},
		m_FarPlane{ 1000.f }
	{
		DirectX::XMStoreFloat3(&m_Position, position);
		DirectX::XMStoreFloat3(&m_FrontVector, DirectX::XMVector3Normalize(DirectX::XMVectorSubtract({ 0.0f, 0.0f, 0.0f }, position)));
		DirectX::XMVECTOR lookAt = DirectX::XMVectorAdd(position, DirectX::XMLoadFloat3(&m_FrontVector));

		/*View matrix*/
		DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH(position, lookAt, DirectX::XMLoadFloat3(&m_UpVector));

		/*Perspective matrix*/
		m_AspectRatio = static_cast<float>(width) / static_cast<float>(height);
		DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(m_FieldOfViewDegrees),
			m_AspectRatio,
			m_NearPlane,
			m_FarPlane);

		DirectX::XMStoreFloat4x4(&m_ViewProjectionMatrix, viewMatrix * projectionMatrix);
		DirectX::XMStoreFloat4x4(&m_ViewMatrix, viewMatrix);
		DirectX::XMStoreFloat4x4(&m_ProjectionMatrix, projectionMatrix);

		m_pConstantBuffer = std::make_unique<ConstantBuffer>(sizeof(DirectX::XMFLOAT3));
	}

	void PerspectiveCamera::RecalculateViewProjectionMatrix() noexcept
	{
		DirectX::XMVECTOR lookAt = DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&m_Position), DirectX::XMLoadFloat3(&m_FrontVector));
		DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat3(&m_Position),
			lookAt,
			DirectX::XMLoadFloat3(&m_UpVector));

		DirectX::XMStoreFloat4x4(&m_ViewMatrix, viewMatrix);
		DirectX::XMStoreFloat4x4(&m_ViewProjectionMatrix, viewMatrix * DirectX::XMLoadFloat4x4(&m_ProjectionMatrix));
	}

	void PerspectiveCamera::SetPosition(const DirectX::XMFLOAT3& position) noexcept
	{
		m_Position = position;
		RecalculateViewProjectionMatrix();
	}

	void PerspectiveCamera::RecalculateProjectionMatrix(const uint32_t width, const uint32_t height) noexcept
	{
		m_AspectRatio = static_cast<float>(width) / static_cast<float>(height);
		DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(m_FieldOfViewDegrees),
			m_AspectRatio,
			m_NearPlane,
			m_FarPlane);
		DirectX::XMStoreFloat4x4(&m_ProjectionMatrix, projectionMatrix);

		RecalculateViewProjectionMatrix();
	}

	void PerspectiveCamera::RecalculateProjectionMatrix() noexcept
	{
		DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(m_FieldOfViewDegrees),
			m_AspectRatio,
			m_NearPlane,
			m_FarPlane);
		DirectX::XMStoreFloat4x4(&m_ProjectionMatrix, projectionMatrix);

		RecalculateViewProjectionMatrix();
	}

	void PerspectiveCamera::Update(const float deltaTime) noexcept
	{
		if (Mouse::IsButtonPressed(RLS_BUTTON::Right))
		{
			DirectX::XMVECTOR position = DirectX::XMLoadFloat3(&m_Position);
			if (Keyboard::IsKeyPressed(RLS_KEY::W))
			{
				DirectX::XMVECTOR frontVector = DirectX::XMLoadFloat3(&m_FrontVector);
				frontVector = DirectX::XMVectorScale(frontVector, m_CameraSpeed * deltaTime);
				position = DirectX::XMVectorAdd(position, frontVector);
			}
			else if (Keyboard::IsKeyPressed(RLS_KEY::S))
			{
				DirectX::XMVECTOR frontVector = DirectX::XMLoadFloat3(&m_FrontVector);
				frontVector = DirectX::XMVectorScale(frontVector, m_CameraSpeed * deltaTime);
				position = DirectX::XMVectorSubtract(position, frontVector);
			}
			if (Keyboard::IsKeyPressed(RLS_KEY::D))
			{
				DirectX::XMVECTOR rightVector = DirectX::XMLoadFloat3(&m_RightVector);
				rightVector = DirectX::XMVectorScale(rightVector, m_CameraSpeed * deltaTime);
				position = DirectX::XMVectorAdd(position, rightVector);
			}
			else if (Keyboard::IsKeyPressed(RLS_KEY::A))
			{
				DirectX::XMVECTOR rightVector = DirectX::XMLoadFloat3(&m_RightVector);
				rightVector = DirectX::XMVectorScale(rightVector, m_CameraSpeed * deltaTime);
				position = DirectX::XMVectorSubtract(position, rightVector);
			}
			if (Keyboard::IsKeyPressed(RLS_KEY::Q))
			{
				DirectX::XMVECTOR upVector = DirectX::XMLoadFloat3(&m_UpVector);
				upVector = DirectX::XMVectorScale(upVector, m_CameraSpeed * deltaTime);
				position = DirectX::XMVectorSubtract(position, upVector);
			}
			else if (Keyboard::IsKeyPressed(RLS_KEY::E))
			{
				DirectX::XMVECTOR upVector = DirectX::XMLoadFloat3(&m_UpVector);
				upVector = DirectX::XMVectorScale(upVector, m_CameraSpeed * deltaTime);
				position = DirectX::XMVectorAdd(position, upVector);
			}

			DirectX::XMStoreFloat3(&m_Position, position);
			RecalculateViewProjectionMatrix();
		}
	}

	void PerspectiveCamera::OnMouseMove() noexcept
	{
		auto [xOffset, yOffset] = Mouse::GetDeltaCoordinates();
		m_Yaw -= static_cast<float>(xOffset) * m_TiltSensitivity;
		m_Pitch -= static_cast<float>(yOffset) * m_TiltSensitivity;

		if (m_Pitch > 89.0f)
			m_Pitch = 89.0f;
		else if (m_Pitch < -89.0f)
			m_Pitch = -89.0f;

		float yawRadians = DirectX::XMConvertToRadians(m_Yaw);
		float pitchRadians = DirectX::XMConvertToRadians(m_Pitch);
		float forwardX = std::cos(yawRadians) * std::cos(pitchRadians);
		float forwardY = std::sin(pitchRadians);
		float forwardZ = std::sin(yawRadians) * std::cos(pitchRadians);

		DirectX::XMVECTOR forwardVector = { forwardX, forwardY, forwardZ, 0.0f };
		forwardVector = DirectX::XMVector3Normalize(forwardVector);
		DirectX::XMStoreFloat3(&m_FrontVector, forwardVector);

		DirectX::XMVECTOR rightVector = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(DirectX::XMLoadFloat3(&m_WorldUp), forwardVector));
		DirectX::XMVECTOR upVector = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(forwardVector, rightVector));
		DirectX::XMStoreFloat3(&m_RightVector, rightVector);
		DirectX::XMStoreFloat3(&m_UpVector, upVector);

		RecalculateViewProjectionMatrix();
	}

	std::shared_ptr<PerspectiveCamera> PerspectiveCamera::Create(const DirectX::XMVECTOR& position, const uint32_t width, const uint32_t height) noexcept
	{
		return std::make_shared<PerspectiveCamera>(position, width, height);
	}

	void PerspectiveCamera::SetFieldOfViewDegrees(const float fieldOfViewDegrees) noexcept
	{
		m_FieldOfViewDegrees = fieldOfViewDegrees;
		RecalculateProjectionMatrix();
	}

	void PerspectiveCamera::SetNearPlane(const float nearPlane) noexcept
	{
		m_NearPlane = nearPlane;
		RecalculateProjectionMatrix();
	}

	void PerspectiveCamera::SetFarPlane(const float farPlane) noexcept
	{
		m_FarPlane = farPlane;
		RecalculateProjectionMatrix();
	}
}