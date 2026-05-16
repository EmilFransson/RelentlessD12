#include "Common.hlsli"
#include "CommonBindings.hlsli"

struct PassParameters
{
    uint SourceIndex;
    uint TargetIndex;
    uint AverageLuminanceIndex;
    float Padding;
};

float3 ToneMapACES(float3 hdrColor)
{
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return saturate((hdrColor * (a * hdrColor + b)) / (hdrColor * (c * hdrColor + d) + e));
}

ConstantBuffer<PassParameters> passData : register(b1, space0);

static const float3x3 ACESInputMat =
{
    { 0.59719, 0.35458, 0.04823 },
    { 0.07600, 0.90834, 0.01566 },
    { 0.02840, 0.13383, 0.83777 }
};

// ODT_SAT => XYZ => D60_2_D65 => sRGB
static const float3x3 ACESOutputMat =
{
    { 1.60475, -0.53108, -0.07367 },
    { -0.10208, 1.10813, -0.00605 },
    { -0.00327, -0.07276, 1.07602 }
};

float3 RRTAndODTFit(float3 v)
{
    float3 a = v * (v + 0.0245786f) - 0.000090537f;
    float3 b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
    return a / b;
}

float3 ACESFitted(float3 color)
{
    color = mul(ACESInputMat, color);

    // Apply RRT and ODT
    color = RRTAndODTFit(color);

    color = mul(ACESOutputMat, color);

    // Clamp to [0, 1]
    color = saturate(color);

    return color;
}

[numthreads(16, 16, 1)]
void cs_main(uint3 threadId : SV_DispatchThreadID)
{
    if (any(threadId.xy >= uint2(cView.ViewportDimensions)))
        return;

    Texture2D sourceHDRTexture = ResourceDescriptorHeap[passData.SourceIndex];
    RWTexture2D<float4> targetTexture = ResourceDescriptorHeap[passData.TargetIndex];
    
    float4 hdrTextureColor = sourceHDRTexture.Load(int3(threadId.xy, 0));
    
    StructuredBuffer<float> averageLuminanceBuffer = ResourceDescriptorHeap[passData.AverageLuminanceIndex];
    const float exposure = averageLuminanceBuffer[2];
    hdrTextureColor.xyz *= exposure;
    
    const float3 sdr = ACESFitted(hdrTextureColor.xyz);
    
    targetTexture[threadId.xy] = float4(sdr, hdrTextureColor.a);
}