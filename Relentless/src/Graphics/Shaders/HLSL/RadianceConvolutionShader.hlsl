#include "Common.hlsli"
#include "CommonBindings.hlsli"
#include "EnvironmentMapping.hlsli"

struct PassData
{
    uint InputTextureIndex;
    uint OutputTextureIndex;
    uint InputDimensions;
    uint OutputDimensions;
    float4 LowerHemisphereColor;
    float Roughness;
    float3 Padding;
};

ConstantBuffer<PassData> passData : register(b0, space0);

static const uint NUM_SAMPLES = 1024u;
static const float INV_NUM_SAMPLES = 1.0f / float(NUM_SAMPLES);

float2 SampleHammersley(uint i)
{
    return float2(i * INV_NUM_SAMPLES, RadicalInverse_VdC(i));
}

[numthreads(32,32,1)]
void cs_main(uint3 threadID : SV_DispatchThreadID)
{
    if (any(threadID >= uint3(passData.OutputDimensions, passData.OutputDimensions, 6)))
        return;
    
    TextureCube<float4> inputTexture = ResourceDescriptorHeap[passData.InputTextureIndex];
    RWTexture2DArray<float4> outputTexture = ResourceDescriptorHeap[passData.OutputTextureIndex];
    
    const float2 inputSize = float2(passData.InputDimensions, passData.InputDimensions);
    const float wt = 4.0f * PI / (6.0f * inputSize.x * inputSize.y);
    
    // Approximation: Assume zero viewing angle (isotropic reflections).
    float3 N = GetCubeMapTexCoord(threadID, uint2(passData.OutputDimensions, passData.OutputDimensions));
    float3 Lo = N;
	
    float3 S;
    float3 T;
    ComputeBasisVectors(N, S, T);

    float3 color = float3(0.0f, 0.0f, 0.0f);
    float weight = 0;
    
    // Convolve environment map using GGX NDF importance sampling.
	// Weight by cosine term since Epic claims it generally improves quality.
    for (uint i = 0; i < NUM_SAMPLES; i++)
    {
        float2 u = SampleHammersley(i);
        float3 Lh = TangentToWorld(SampleGGX(u.x, u.y, passData.Roughness), N, S, T);

		// Compute incident direction (Li) by reflecting viewing direction (Lo) around half-vector (Lh).
        const float3 Li = 2.0f * dot(Lo, Lh) * Lh - Lo;
        
        const float cosLi = dot(N, Li);
        if (cosLi > 0.0f)
        {
			// Use Mipmap Filtered Importance Sampling to improve convergence.
			// See: https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch20.html, section 20.4

            const float cosLh = max(dot(N, Lh), 0.0f);

			// GGX normal distribution function (D term) probability density function.
			// Scaling by 1/4 is due to change of density in terms of Lh to Li (and since N=V, rest of the scaling factor cancels out).
            const float pdf = NdfGGX(cosLh, passData.Roughness) * 0.25f;

			// Solid angle associated with this sample.
            const float ws = 1.0f / (NUM_SAMPLES * pdf);

			// Mip level to sample from.
            const float mipLevel = max(0.5f * log2(ws / wt) + 1.0f, 0.0f);

            color += inputTexture.SampleLevel(sLinearWrap, Li, mipLevel).rgb * cosLi;
            weight += cosLi;
        }
    }
    color /= weight;

    outputTexture[threadID] = float4(color, 1.0f);
}