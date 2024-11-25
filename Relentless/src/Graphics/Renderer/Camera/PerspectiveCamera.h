#pragma once
#include "Camera.h"
#include "Graphics/Resources/ConstantBufferSet.h"
namespace Relentless
{
	class PerspectiveCamera : public Camera
	{
	public:
		PerspectiveCamera() noexcept;
		virtual ~PerspectiveCamera() noexcept override final = default;

		void Update() noexcept;
		[[nodiscard]] static std::shared_ptr<PerspectiveCamera> Create() noexcept;

		std::unique_ptr<ConstantBufferSet> m_pConstantBufferSet = nullptr;
	private:
		Vector3 m_Velocity = Vector3::Zero;
	};
}