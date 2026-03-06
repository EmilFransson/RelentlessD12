#pragma once

#include "Controller/PerspectiveCameraController.h"

namespace Relentless
{
	struct ViewportDetailsContext
	{
		PerspectiveCameraController* CameraController = nullptr;
	};
}