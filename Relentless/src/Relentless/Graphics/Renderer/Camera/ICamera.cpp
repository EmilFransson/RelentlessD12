#include "ICamera.h"
namespace Relentless
{
	ICamera::ICamera(const DirectX::XMFLOAT4X4& projectionMatrix) noexcept
		: m_ProjectionMatrix{projectionMatrix},
		  m_WorldUp{ 0.0f, 1.0f, 0.0f }
	{}

	ICamera::ICamera()
		: m_ProjectionMatrix{},
		  m_WorldUp{0.0f, 1.0f, 0.0f}
	{

	}
}