#pragma once
#include "Camera.h"

#include "Core/DLLExport.h"

namespace Relentless
{
	class RLS_API PerspectiveCamera : public Camera
	{
	public:
		PerspectiveCamera() noexcept;
		virtual ~PerspectiveCamera() noexcept override = default;

		NO_DISCARD static std::shared_ptr<PerspectiveCamera> Create() noexcept;
	};
}