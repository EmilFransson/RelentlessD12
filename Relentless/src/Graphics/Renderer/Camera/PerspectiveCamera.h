#pragma once
#include "ICamera.h"
#include "../../Resources/ConstantBufferSet.h"
namespace Relentless
{
	class PerspectiveCamera : public ICamera
	{
	public:
		PerspectiveCamera(const DirectX::XMVECTOR& position, const uint32_t width, const uint32_t height) noexcept;
		virtual ~PerspectiveCamera() noexcept override final;
		void RecalculateViewProjectionMatrix() noexcept;

		[[nodiscard]] constexpr DirectX::XMFLOAT4X4& GetViewMatrix() noexcept { return m_ViewMatrix; }
		[[nodiscard]] constexpr DirectX::XMFLOAT4X4& GetViewProjectionMatrix() noexcept { return m_ViewProjectionMatrix; }

		[[nodiscard]] constexpr DirectX::XMFLOAT3& GetPosition() noexcept { return m_Position; }
		void SetPosition(const DirectX::XMFLOAT3& position) noexcept;

		[[nodiscard]] constexpr DirectX::XMFLOAT3& GetUpVector() noexcept { return m_UpVector; }
		[[nodiscard]] constexpr DirectX::XMFLOAT3& GetRightVector() noexcept { return m_RightVector; }
		[[nodiscard]] constexpr DirectX::XMFLOAT3& GetFrontVector() noexcept { return m_FrontVector; }

		void RecalculateProjectionMatrix(const uint32_t width, const uint32_t height) noexcept;
		void RecalculateProjectionMatrix() noexcept;

		void Update(const float deltaTime) noexcept;
		void OnMouseMove() noexcept;
		[[nodiscard]] static std::shared_ptr<PerspectiveCamera> Create(const DirectX::XMVECTOR& position, const uint32_t width, const uint32_t height) noexcept;

		[[nodiscard]] virtual constexpr const ProjectionType GetProjectionType() const noexcept { return ProjectionType::Perspective; }
		void SetFieldOfViewDegrees(const float fieldOfViewDegrees) noexcept;
		void SetNearPlane(const float nearPlane) noexcept;
		void SetFarPlane(const float farPlane) noexcept;
		[[nodiscard]] constexpr const float& GetFieldOfViewInDegrees() const noexcept { return m_FieldOfViewDegrees; }
		[[nodiscard]] constexpr const float& GetNearPlane() const noexcept { return m_NearPlane; }
		[[nodiscard]] constexpr const float& GetFarPlane() const noexcept { return m_FarPlane; }
		[[nodiscard]] constexpr const float& GetYaw() const noexcept { return m_Yaw; }
		[[nodiscard]] constexpr const float& GetPitch() const noexcept { return m_Pitch; }

		std::unique_ptr<ConstantBufferSet> m_pConstantBufferSet = nullptr;
	private:
		DirectX::XMFLOAT3 m_Position;
		DirectX::XMFLOAT3 m_UpVector;
		DirectX::XMFLOAT3 m_FrontVector;
		DirectX::XMFLOAT3 m_RightVector;

		DirectX::XMFLOAT4X4 m_ViewMatrix;
		DirectX::XMFLOAT4X4 m_ViewProjectionMatrix;

		float m_Yaw;
		float m_Pitch;
		float m_CameraSpeed;
		float m_TiltSensitivity;

		float m_FieldOfViewDegrees;
		float m_AspectRatio;
		float m_NearPlane;
		float m_FarPlane;
	};
}