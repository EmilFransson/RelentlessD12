#include "CommonBindings.hlsli"
#include "EnvironmentMapping.hlsli"

struct PassData
{
    float4 LowerHemisphereColor;
    uint SrcTextureCubeIndex;
    uint DstTextureCubeIndex;
    uint DstSize;
    float Padding;
};

ConstantBuffer<PassData> passData : register(b0, space0);

[numthreads(8, 8, 1)]
void cs_main(uint3 threadID : SV_DispatchThreadID)
{
    if (any(threadID >= uint3(passData.DstSize, passData.DstSize, 6)))
        return;

    Texture2DArray<float4> srcTextureCube = ResourceDescriptorHeap[passData.SrcTextureCubeIndex];
    RWTexture2DArray<float4> dstTextureCube = ResourceDescriptorHeap[passData.DstTextureCubeIndex];
    
    const float3 dir = GetCubeMapTexCoord(int3(threadID), uint2(passData.DstSize, passData.DstSize));
    const float4 sourceColor = srcTextureCube.Load(int4(threadID.xy, threadID.z, 0));
    
    const float t = smoothstep(-0.08f, 0.08f, -dir.y);
    const float4 finalColor = lerp(sourceColor, passData.LowerHemisphereColor, t);

    dstTextureCube[threadID] = finalColor;
}