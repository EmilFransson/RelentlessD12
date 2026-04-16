// Calculate sampling coords for equirectangular texture
// https://en.wikipedia.org/wiki/Spherical_coordinate_system#Cartesian_coordinates

#include "Common.hlsli"
#include "CommonBindings.hlsli"
#include "EnvironmentMapping.hlsli"

struct PassData
{
    uint EquirectangularTextureIndex;
    uint OutputCubeIndex;
    uint Dimensions;
    float Padding;
};

ConstantBuffer<PassData> passData : register(b0, space0);

[numthreads(32,32,1)]
void cs_main(uint3 threadID : SV_DispatchThreadID)
{
    if (any(threadID >= uint3(passData.Dimensions, passData.Dimensions, 6)))
        return;
    
    RWTexture2DArray<float4> outputCube = ResourceDescriptorHeap[passData.OutputCubeIndex];
    Texture2D<float4> equirectangularTexture = ResourceDescriptorHeap[passData.EquirectangularTextureIndex];
    
    const float3 dir = GetCubeMapTexCoord(threadID, uint2(passData.Dimensions, passData.Dimensions));

    const float phi = atan2(dir.z, dir.x);
    const float theta = acos(clamp(dir.y, -1.0f, 1.0f));
    const float2 uv = float2(phi / (2.0 * PI) + 0.5, theta / PI);

    const float4 color = equirectangularTexture.SampleLevel(sLinearWrapClamp, uv, 0.0f);
    outputCube[threadID] = color;
}