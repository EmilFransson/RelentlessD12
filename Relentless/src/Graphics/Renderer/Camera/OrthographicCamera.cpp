#include "OrthographicCamera.h"
namespace Relentless
{
	OrthographicCamera::OrthographicCamera(const DirectX::XMVECTOR& position, const uint32_t, const uint32_t) noexcept
		: m_UpVector{ DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f) }, m_FrontVector{0.0f, 0.0f, 1.0f}
	{
		DirectX::XMStoreFloat3(&m_Position, position);
		DirectX::XMVECTOR lookAt = DirectX::XMVector3Normalize(DirectX::XMVectorAdd(position, DirectX::XMLoadFloat3(&m_FrontVector)));
		
		DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH(position, lookAt, DirectX::XMLoadFloat3(&m_UpVector));

		DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixOrthographicLH(1.0f, 1.0f,  0.0f, 100.0f);

		DirectX::XMStoreFloat4x4(&m_ViewProjectionMatrix, viewMatrix * projectionMatrix);
		DirectX::XMStoreFloat4x4(&m_ViewMatrix, viewMatrix);
		DirectX::XMStoreFloat4x4(&m_ProjectionMatrix, projectionMatrix);
	}

	std::shared_ptr<OrthographicCamera> OrthographicCamera::Create(const DirectX::XMVECTOR& position, const uint32_t width, const uint32_t height) noexcept
	{
		return std::make_shared<OrthographicCamera>(position, width, height);
	}

	void OrthographicCamera::SetPosition(const DirectX::XMFLOAT3& newPosition) noexcept
	{
		m_Position = newPosition;
		DirectX::XMVECTOR lookAt = DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&m_Position), DirectX::XMLoadFloat3(&m_FrontVector));
		DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat3(&newPosition), lookAt, DirectX::XMLoadFloat3(&m_UpVector));

		DirectX::XMMATRIX proj = DirectX::XMLoadFloat4x4(&m_ProjectionMatrix);

		DirectX::XMStoreFloat4x4(&m_ViewMatrix, viewMatrix);
		DirectX::XMStoreFloat4x4(&m_ViewProjectionMatrix, viewMatrix * proj);
	}
}