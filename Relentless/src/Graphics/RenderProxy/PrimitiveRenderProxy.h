#pragma once

#include "Graphics/Renderer/RenderTypes.h"

namespace Relentless
{
	struct PrimitiveRenderProxy
	{
		Matrix LocalToWorld				= Matrix::Identity;
		UUID MaterialUUID				= {};
		UUID MeshUUID					= {};
		uint32 EntityID					= 0u;
		ELightChannel LightChannelMask	= ELightChannel::Default;
		bool CastShadows				= true;
		bool Visible					= false;
	};
}