#include "Common.hlsli"
#include "CommonBindings.hlsli"

#include "Fog.hlsli"

struct PassData
{
    uint DepthIndex;
    uint ColorIndex;
    float2 Padding;
};

ConstantBuffer<PassData> passData : register(b1, space0);

[numthreads(16, 16, 1)]
void cs_main(uint3 aThreadID : SV_DispatchThreadID)
{
    if (any(aThreadID.xy >= uint2(cView.ViewportDimensions)))
        return;
    
    Texture2D<float4> depthTexture = ResourceDescriptorHeap[passData.DepthIndex];
    const float depth = depthTexture.Load(int3(aThreadID.xy, 0)).r;
    
    const float2 uv = TexelToUV(aThreadID.xy, cView.ViewportDimensionsInv);
    const float3 worldPosition = WorldPositionFromDepth(uv, depth, cView.ClipToWorld);
    const FogData fogData = GetFog();
    const FogResult fogResult = EvaluateExponentialHeightFog(worldPosition, cView.ViewLocation, fogData);

    RWTexture2D<float4> colorTexture = ResourceDescriptorHeap[passData.ColorIndex];
    const float4 sceneColor = colorTexture[aThreadID.xy];
    const float3 fogged = lerp(fogResult.Inscatter, sceneColor.rgb, fogResult.Transmittance);
    colorTexture[aThreadID.xy] = float4(fogged, sceneColor.a);
}