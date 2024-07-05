#pragma once
#include "Vector3.h"

namespace Relentless
{
	struct Transform
	{
		Matrix4x4 Matrix = Matrix4x4::Identity();
		Vector3f Translation = Vector3f(0.0f, 0.0f, 0.0f);
		Vector3f Rotation = Vector3f(0.0f, 0.0f, 0.0f);
		Vector3f Scale = Vector3f(1.0f, 1.0f, 1.0f);
	};
}