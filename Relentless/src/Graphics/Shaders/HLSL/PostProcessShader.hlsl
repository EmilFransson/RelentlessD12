#include "Common.hlsli"
#include "CommonBindings.hlsli"

struct PassParameters
{
    uint SourceIndex;
    uint TargetIndex;
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

static const float3 aces_input_matrix[] =
{
    float3(0.59719f, 0.35458f, 0.04823f),
    float3(0.07600f, 0.90834f, 0.01566f),
    float3(0.02840f, 0.13383f, 0.83777f)
};

static const float3 aces_output_matrix[] =
{
    float3(1.60475f, -0.53108f, -0.07367f),
    float3(-0.10208f, 1.10813f, -0.00605f),
    float3(-0.00327f, -0.07276f, 1.07602f)
};

float3 mul(float3 v)
{
    float x = aces_input_matrix[0][0] * v[0] + aces_input_matrix[0][1] * v[1] + aces_input_matrix[0][2] * v[2];
    float y = aces_input_matrix[1][0] * v[1] + aces_input_matrix[1][1] * v[1] + aces_input_matrix[1][2] * v[2];
    float z = aces_input_matrix[2][0] * v[1] + aces_input_matrix[2][1] * v[1] + aces_input_matrix[2][2] * v[2];
    return float3(x, y, z);
}

float3 mul2(float3 v)
{
    float x = aces_output_matrix[0][0] * v[0] + aces_output_matrix[0][1] * v[1] + aces_output_matrix[0][2] * v[2];
    float y = aces_output_matrix[1][0] * v[1] + aces_output_matrix[1][1] * v[1] + aces_output_matrix[1][2] * v[2];
    float z = aces_output_matrix[2][0] * v[1] + aces_output_matrix[2][1] * v[1] + aces_output_matrix[2][2] * v[2];
    return float3(x, y, z);
}

float3 rtt_and_odt_fit(float3 v)
{
    float3 a = v * (v + 0.0245786f) - 0.000090537f;
    float3 b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
    return a / b;
}

float3 aces_fitted(float3 v)
{
    v = mul(v);
    v = rtt_and_odt_fit(v);
    return mul2(v);
}

[numthreads(16, 16, 1)]
void cs_main(uint3 threadId : SV_DispatchThreadID)
{
    if (any(threadId.xy >= uint2(cView.ViewportDimensions)))
        return;

    Texture2D sourceHDRTexture = ResourceDescriptorHeap[passData.SourceIndex];
    RWTexture2D<float4> targetTexture = ResourceDescriptorHeap[passData.TargetIndex];
    
    float4 hdrTextureColor = sourceHDRTexture.Load(int3(threadId.xy, 0));
    float3 sdr = saturate(aces_fitted(hdrTextureColor.xyz));
    float3 sRGB = LinearToSRGB(sdr);
    
    targetTexture[threadId.xy] = float4(sRGB, hdrTextureColor.a);
}