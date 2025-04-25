#include "PerspectiveCamera.h"

namespace Relentless
{
	PerspectiveCamera::PerspectiveCamera() noexcept
	{
	}

	std::shared_ptr<PerspectiveCamera> PerspectiveCamera::Create() noexcept
	{
		return std::make_shared<PerspectiveCamera>();
	}
}