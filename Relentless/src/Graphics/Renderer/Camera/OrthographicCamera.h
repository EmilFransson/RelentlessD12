#pragma once
#include "ICamera.h"
namespace Relentless
{
	class OrthographicCamera : public ICamera
	{
	public:
		OrthographicCamera(const DirectX::XMVECTOR& position, const uint32_t width, const uint32_t height) noexcept;
		virtual ~OrthographicCamera() noexcept override final = default;
		[[nodiscard]] virtual constexpr const ProjectionType GetProjectionType() const noexcept { return ProjectionType::Orthographic; }
		[[nodiscard]] static std::shared_ptr<OrthographicCamera> Create(const DirectX::XMVECTOR& position, const uint32_t width, const uint32_t height) noexcept;
		[[nodiscard]] constexpr DirectX::XMFLOAT4X4& GetViewMatrix() noexcept { return m_ViewMatrix; }
		[[nodiscard]] constexpr DirectX::XMFLOAT4X4& GetViewProjectionMatrix() noexcept { return m_ViewProjectionMatrix; }
		[[nodiscard]] constexpr const DirectX::XMFLOAT3& GetPosition() const noexcept { return m_Position; }
		void SetPosition(const DirectX::XMFLOAT3& newPosition) noexcept;
	private:
		DirectX::XMFLOAT3 m_Position;
		DirectX::XMFLOAT3 m_UpVector;
		DirectX::XMFLOAT3 m_FrontVector;

		DirectX::XMFLOAT4X4 m_ViewMatrix;
		DirectX::XMFLOAT4X4 m_ViewProjectionMatrix;
	};
}