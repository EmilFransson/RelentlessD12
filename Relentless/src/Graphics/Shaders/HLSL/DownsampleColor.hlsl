#include "Common.hlsli"
#include "CommonBindings.hlsli"

struct PassParams
{
    uint2 TargetDimensions;
    float2 TargetDimensionsInv;
    uint InputIndex;
    uint OutputIndex;
    float2 Padding;
};

ConstantBuffer<PassParams> passParams : register(b0, space0);

[numthreads(8, 8, 1)]
void cs_main(uint3 threadId : SV_DispatchThreadID)
{
    if (!all(threadId.xy < passParams.TargetDimensions))
        return;
    
    Texture2D<float4> input = ResourceDescriptorHeap[passParams.InputIndex];
    RWTexture2D<float4> output = ResourceDescriptorHeap[passParams.OutputIndex];
    
    const float2 uv = TexelToUV(threadId.xy, passParams.TargetDimensionsInv);
    const float4 color = input.SampleLevel(sLinearClamp, uv, 0.0f);
    
    output[threadId.xy] = color;

}