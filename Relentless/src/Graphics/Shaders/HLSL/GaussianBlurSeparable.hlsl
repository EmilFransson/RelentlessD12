#include "Common.hlsli"
#include "CommonBindings.hlsli"

struct PassParameters
{
    uint SourceIndex;
    uint TargetIndex;
    uint Radius;
    uint IsHorizontal;
};

//Should be padded appropriately CPU-side!
struct GaussianBlurCB
{
    float Weights[65];
};

ConstantBuffer<GaussianBlurCB> blurData : register(b0, space0);
ConstantBuffer<PassParameters> passData : register(b1, space0);

[numthreads(16, 16, 1)]
void cs_main(uint3 threadId : SV_DispatchThreadID)
{
    if (any(threadId.xy >= uint2(cView.ViewportDimensions)))
        return;

    Texture2D<float> sourceTexture = ResourceDescriptorHeap[passData.SourceIndex];
    RWTexture2D<float> targetTexture = ResourceDescriptorHeap[passData.TargetIndex];
    
    float sum = 0.0f;

    const int radius = int(passData.Radius);
    for (int i = -radius; i <= radius; ++i)
    {
        int2 offset = passData.IsHorizontal != 0 ?
            int2(threadId.x + i, threadId.y) :
            int2(threadId.x, threadId.y + i);

        float sample = 0.0f;

        if (all(offset >= int2(0, 0)) && all(offset <= int2(cView.ViewportDimensions)))
        {
            sample = sourceTexture.Load(int3(offset, 0));
        }

        const float weight = blurData.Weights[i + passData.Radius];
        sum += sample * weight;
    }
    
    targetTexture[threadId.xy] = sum;
}