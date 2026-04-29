#pragma once

namespace Relentless
{
	struct PrimitiveRenderProxy
	{
		Matrix LocalToWorld = Matrix::Identity;
		UUID MaterialUUID = {};
		UUID MeshUUID = {};
		uint32 EntityID = 0u;
		bool Visible = false;
	};
}