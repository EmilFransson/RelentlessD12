#include "Common.hlsli"

#define EPSILON 0.0001f
#define HISTOGRAM_THREADS_PER_DIMENSION 16
#define NUM_HISTOGRAM_BINS 256

struct PassParams
{
    uint Width;
    uint Height;
    float MinLogLuminance;
    float OneOverLogLuminanceRange;
    uint HDRTextureIndex;
    uint LuminanceHistogramIndex;
    float2 Padding;
};

ConstantBuffer<PassParams> passParams : register(b0, space0);

uint HDRToHistogramBin(float3 hdrColor)
{
    const float luminance = GetLuminance(hdrColor);

    if (luminance < EPSILON)
    {
        return 0;
    }

    const float logLuminance = saturate((log2(luminance) - passParams.MinLogLuminance) * passParams.OneOverLogLuminanceRange);
    return (uint)(logLuminance * (NUM_HISTOGRAM_BINS - 1) + 1.0);
}

groupshared uint HistogramShared[NUM_HISTOGRAM_BINS];

[numthreads(HISTOGRAM_THREADS_PER_DIMENSION, HISTOGRAM_THREADS_PER_DIMENSION, 1)]
void cs_main(uint groupIndex : SV_GroupIndex, uint3 threadId : SV_DispatchThreadID)
{
    HistogramShared[groupIndex] = 0;

    GroupMemoryBarrierWithGroupSync();

    if (threadId.x < passParams.Width && threadId.y < passParams.Height)
    {
        Texture2D hdrTexture = ResourceDescriptorHeap[passParams.HDRTextureIndex];
        const float3 hdrColor = hdrTexture.Load(int3(threadId.xy, 0)).rgb;
        const uint binIndex = HDRToHistogramBin(hdrColor);
        InterlockedAdd(HistogramShared[binIndex], 1);
    }

    GroupMemoryBarrierWithGroupSync();

    RWBuffer<uint> luminanceHistogram = ResourceDescriptorHeap[passParams.LuminanceHistogramIndex];
    InterlockedAdd(luminanceHistogram[groupIndex], HistogramShared[groupIndex]);
}