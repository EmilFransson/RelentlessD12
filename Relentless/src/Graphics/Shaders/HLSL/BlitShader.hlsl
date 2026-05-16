#include "Common.hlsli"
#include "CommonBindings.hlsli"

struct PassParameters
{
    uint SourceIndex;
    uint TargetIndex;
    float2 Padding;
};

ConstantBuffer<PassParameters> passData : register(b1, space0);

[numthreads(16, 16, 1)]
void cs_main(uint3 threadId : SV_DispatchThreadID)
{
    if (any(threadId.xy >= uint2(cView.ViewportDimensions)))
        return;

    Texture2D sourceTexture = ResourceDescriptorHeap[passData.SourceIndex];
    RWTexture2D<float4> targetTexture = ResourceDescriptorHeap[passData.TargetIndex];
    
    const float4 sdr = sourceTexture.Load(int3(threadId.xy, 0));
    const float3 sRGB = LinearToSRGB(sdr.xyz);
    
    targetTexture[threadId.xy] = float4(sRGB, sdr.a);
}