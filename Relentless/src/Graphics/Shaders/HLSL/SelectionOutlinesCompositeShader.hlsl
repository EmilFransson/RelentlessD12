#include "Common.hlsli"
#include "CommonBindings.hlsli"

struct PassParameters
{
    uint TargetIndex;
    uint OutlinesSolidIndex;
    uint OutlinesBlurredIndex;
    float Padding;
};

ConstantBuffer<PassParameters> passData : register(b1, space0);

[numthreads(16, 16, 1)]
void cs_main(uint3 threadId : SV_DispatchThreadID)
{
    if (any(threadId.xy >= uint2(cView.ViewportDimensions)))
        return;

    RWTexture2D<float4> targetTexture = ResourceDescriptorHeap[passData.TargetIndex];
    Texture2D<float> solidTexture = ResourceDescriptorHeap[passData.OutlinesSolidIndex];
    Texture2D<float> blurredTexture = ResourceDescriptorHeap[passData.OutlinesBlurredIndex];
    
    const float blurredEntityID = blurredTexture.Load(int3(threadId.xy, 0));
    const float EntityID = solidTexture.Load(int3(threadId.xy, 0));
    
    if (abs(blurredEntityID - EntityID) > 0.15f)
    {
        targetTexture[threadId.xy] = float4(float3(0.9, 0.35, 0.0), 1.0f);
    }
}