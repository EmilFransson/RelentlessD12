#include "CommonBindings.hlsli"
#include "EnvironmentMapping.hlsli"

struct PassData
{
    uint SrcTextureCubeIndex;
    uint DstTextureCubeIndex;
    uint SrcSize;
    uint DstSize;
};

ConstantBuffer<PassData> passData : register(b0, space0);

[numthreads(8, 8, 1)]
void cs_main(uint3 threadID : SV_DispatchThreadID)
{
    if (any(threadID >= uint3(passData.DstSize, passData.DstSize, 6)))
        return;

    TextureCube<float4> srcTextureCube = ResourceDescriptorHeap[passData.SrcTextureCubeIndex];
    RWTexture2DArray<float4> dstTextureCube = ResourceDescriptorHeap[passData.DstTextureCubeIndex];
    
    const float3 dir = GetCubeMapTexCoord(int3(threadID), uint2(passData.DstSize, passData.DstSize));
    const float lod = log2((float) passData.SrcSize / (float)passData.DstSize);

    const float4 color = srcTextureCube.SampleLevel(sLinearClamp, dir, lod);
    dstTextureCube[threadID] = color;
}