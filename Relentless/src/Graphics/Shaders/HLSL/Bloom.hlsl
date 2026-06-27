#include "Common.hlsli"
#include "CommonBindings.hlsli"

// Based on
// https://github.com/simco50/D3D12_Research/blob/master/Resources/Shaders/PostProcessing/Bloom.hlsl
// http://www.iryoku.com/next-generation-post-processing-in-call-of-duty-advanced-warfare
// https://www.froyok.fr/blog/2021-12-ue4-custom-bloom/

#ifndef KARIS_AVERAGE
#define KARIS_AVERAGE 0
#endif

struct DownsampleParams
{
    float2 TargetDimensionsInv;
    uint SourceMip;
    uint SourceIndex;
    uint TargetIndex;
    float3 Padding;
};
ConstantBuffer<DownsampleParams> downsamplePassParams : register(b0, space0);

float3 ComputePartialAverage(float3 v0, float3 v1, float3 v2, float3 v3)
{
#if KARIS_AVERAGE
	float w0 = 1.0f / (1.0f + GetLuminance(v0));
	float w1 = 1.0f / (1.0f + GetLuminance(v1));
	float w2 = 1.0f / (1.0f + GetLuminance(v2));
	float w3 = 1.0f / (1.0f + GetLuminance(v3));
	return (v0 * w0 + v1 * w1 + v2 * w2 + v3 * w3) / (w0 + w1 + w2 + w3);
#else
    return 0.25f * (v0 + v1 + v2 + v3);
#endif
}

[numthreads(8, 8, 1)]
void cs_downsample(uint3 aThreadID : SV_DispatchThreadID)
{
    const float2 uv = TexelToUV(aThreadID.xy, downsamplePassParams.TargetDimensionsInv);
    const uint mip = downsamplePassParams.SourceMip;

    Texture2D<float4> source = ResourceDescriptorHeap[downsamplePassParams.SourceIndex];
    RWTexture2D<float4> target = ResourceDescriptorHeap[downsamplePassParams.TargetIndex];
    
    float3 outColor = float3(0.0f, 0.0f, 0.0f);
    const float3 M0 = source.SampleLevel(sLinearClamp, uv, mip, int2(-1.0f, 1.0f)).xyz;
    const float3 M1 = source.SampleLevel(sLinearClamp, uv, mip, int2(1.0f, 1.0f)).xyz;
    const float3 M2 = source.SampleLevel(sLinearClamp, uv, mip, int2(-1.0f, -1.0f)).xyz;
    const float3 M3 = source.SampleLevel(sLinearClamp, uv, mip, int2(1.0f, -1.0f)).xyz;

    const float3 TL = source.SampleLevel(sLinearClamp, uv, mip, int2(-2.0f, 2.0f)).xyz;
    const float3 T = source.SampleLevel(sLinearClamp, uv, mip, int2(0.0f, 2.0f)).xyz;
    const float3 TR = source.SampleLevel(sLinearClamp, uv, mip, int2(2.0f, 2.0f)).xyz;
    const float3 L = source.SampleLevel(sLinearClamp, uv, mip, int2(-2.0f, 0.0f)).xyz;
    const float3 C = source.SampleLevel(sLinearClamp, uv, mip, int2(0.0f, 0.0f)).xyz;
    const float3 R = source.SampleLevel(sLinearClamp, uv, mip, int2(2.0f, 0.0f)).xyz;
    const float3 BL = source.SampleLevel(sLinearClamp, uv, mip, int2(-2.0f, -2.0f)).xyz;
    const float3 B = source.SampleLevel(sLinearClamp, uv, mip, int2(0.0f, -2.0f)).xyz;
    const float3 BR = source.SampleLevel(sLinearClamp, uv, mip, int2(2.0f, -2.0f)).xyz;

    outColor += ComputePartialAverage(M0, M1, M2, M3) * 0.5f;
    outColor += ComputePartialAverage(TL, T, C, L) * 0.125f;
    outColor += ComputePartialAverage(TR, T, C, R) * 0.125f;
    outColor += ComputePartialAverage(BL, B, C, L) * 0.125f;
    outColor += ComputePartialAverage(BR, B, C, R) * 0.125f;

    target[aThreadID.xy] = float4(outColor, 1.0f);
}

struct UpsampleParams
{
    float2 TargetDimensionsInv;
    uint SourceCurrentMip;
    uint SourcePreviousMip;
    float Radius;
    uint SourceIndex;
    uint PreviousSourceIndex;
    uint TargetIndex;
};
ConstantBuffer<UpsampleParams> upsamplePassParams : register(b0, space0);

[numthreads(8, 8, 1)]
void cs_upscale(uint3 aThreadID : SV_DispatchThreadID)
{
    const float2 uv = TexelToUV(aThreadID.xy, upsamplePassParams.TargetDimensionsInv);
    
    Texture2D<float4> source = ResourceDescriptorHeap[upsamplePassParams.SourceIndex];
    Texture2D<float4> previousSource = ResourceDescriptorHeap[upsamplePassParams.PreviousSourceIndex];
    RWTexture2D<float4> target = ResourceDescriptorHeap[upsamplePassParams.TargetIndex];
    
    const float3 currentColor = source.SampleLevel(sPointClamp, uv, upsamplePassParams.SourceCurrentMip).xyz;
    const uint mip = upsamplePassParams.SourcePreviousMip;

    float3 outColor = float3(0.0f, 0.0f, 0.0f);
    outColor += 0.0625f * previousSource.SampleLevel(sLinearBorder, uv, mip, int2(-1.0f, 1.0f)).xyz;
    outColor += 0.125f * previousSource.SampleLevel(sLinearBorder, uv, mip, int2(0.0f, 1.0f)).xyz;
    outColor += 0.0625f * previousSource.SampleLevel(sLinearBorder, uv, mip, int2(1.0f, 1.0f)).xyz;
    outColor += 0.125f * previousSource.SampleLevel(sLinearBorder, uv, mip, int2(-1.0f, 0.0f)).xyz;
    outColor += 0.25f * previousSource.SampleLevel(sLinearBorder, uv, mip, int2(0.0f, 0.0f)).xyz;
    outColor += 0.125f * previousSource.SampleLevel(sLinearBorder, uv, mip, int2(1.0f, 0.0f)).xyz;
    outColor += 0.0625f * previousSource.SampleLevel(sLinearBorder, uv, mip, int2(-1.0f, -1.0f)).xyz;
    outColor += 0.125f * previousSource.SampleLevel(sLinearBorder, uv, mip, int2(0.0f, -1.0f)).xyz;
    outColor += 0.0625f * previousSource.SampleLevel(sLinearBorder, uv, mip, int2(1.0f, -1.0f)).xyz;
    
    const float3 mixedColor = lerp(currentColor, outColor, upsamplePassParams.Radius);
    target[aThreadID.xy] = float4(mixedColor, 1.0f);
}
