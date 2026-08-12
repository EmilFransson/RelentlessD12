#pragma once
//Need to be kept in sync with CommonBindings.hlsli

namespace Relentless
{
	enum class ERenderViewMode : uint32 { Lit = 0u, Unlit, GeometricNormals, ShadingNormals, Metallic, Roughness, AmbientOcclusion, Opacity };
}