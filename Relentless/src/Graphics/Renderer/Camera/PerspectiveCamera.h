#pragma once
#include "Camera.h"

namespace Relentless
{
	class PerspectiveCamera : public Camera
	{
	public:
		PerspectiveCamera() noexcept;
		virtual ~PerspectiveCamera() noexcept override final = default;

		[[nodiscard]] static std::shared_ptr<PerspectiveCamera> Create() noexcept;
	};
}