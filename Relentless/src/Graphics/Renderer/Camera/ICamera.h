#pragma once
namespace Relentless
{
	enum class ProjectionType { Perspective = 0, Orthographic };
	class ICamera
	{
	public:
		ICamera(const DirectX::XMFLOAT4X4& projectionMatrix) noexcept;
		ICamera();
		virtual ~ICamera() noexcept = default;
		[[nodiscard]] virtual constexpr const ProjectionType GetProjectionType() const noexcept = 0;
		[[nodiscard]] const DirectX::XMFLOAT4X4& GetProjectionMatrix() const noexcept { return m_ProjectionMatrix; }
	protected:
		DirectX::XMFLOAT4X4 m_ProjectionMatrix;
		DirectX::XMFLOAT3 m_WorldUp;
	};
}