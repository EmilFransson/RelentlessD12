#pragma once
#include "Graphics/RHI/Buffer.h"

namespace Relentless
{
	struct MeshRenderProxy
	{
		UUID ID = {};

		Ref<Buffer> VertexBuffer = nullptr;
		Ref<Buffer> IndexBuffer = nullptr;
	};
}