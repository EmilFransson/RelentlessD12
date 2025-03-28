#pragma once
#include "Camera.h"
namespace Relentless
{
	class PerspectiveCamera : public Camera
	{
	public:
		PerspectiveCamera() noexcept;
		virtual ~PerspectiveCamera() noexcept override final = default;

		void Update(bool updateMovement = true) noexcept;
		[[nodiscard]] static std::shared_ptr<PerspectiveCamera> Create() noexcept;
	private:
		Vector3 m_Velocity = Vector3::Zero;
	};
}