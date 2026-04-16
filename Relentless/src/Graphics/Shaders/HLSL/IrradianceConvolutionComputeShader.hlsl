#include "Common.hlsli"
#include "CommonBindings.hlsli"
#include "EnvironmentMapping.hlsli"

struct PassData
{
    uint RadianceMapIndex;
    uint IrradianceMapIndex;
    uint Dimensions;
    uint Samples;
    float4 LowerHemisphereColor;
};

ConstantBuffer<PassData> passData : register(b0, space0);

[numthreads(32,32,1)]
void cs_main(int3 threadID : SV_DispatchThreadID)
{
    if (any(threadID >= uint3(passData.Dimensions, passData.Dimensions, 6)))
        return;
    
    TextureCube<float4> radianceMap = ResourceDescriptorHeap[passData.RadianceMapIndex];
    RWTexture2DArray<float4> irradianceMap = ResourceDescriptorHeap[passData.IrradianceMapIndex];
    
    const float3 N = GetCubeMapTexCoord(threadID, uint2(passData.Dimensions, passData.Dimensions));
    
    float3 S;
    float3 T;
    ComputeBasisVectors(N, S, T);

    const uint samples = 64 * passData.Samples;

	// Monte Carlo integration of hemispherical irradiance.
	// As a small optimization this also includes Lambertian BRDF assuming perfectly white surface (albedo of 1.0)
	// so we don't need to normalize in PBR fragment shader (so technically it encodes exitant radiance rather than irradiance).
    float3 irradiance = float3(0.0f, 0.0f, 0.0f);
    for (uint i = 0; i < samples; i++)
    {
        const float2 u = SampleHammersley(i, samples);
        const float3 Li = TangentToWorld(SampleHemisphere(u.x, u.y), N, S, T);
        const float cosTheta = max(0.0, dot(Li, N));

        float3 radianceSample = float3(0.0f, 0.0f, 0.0f);
        #if LOWER_HEMISPHERE_SOLID_COLOR
        if (Li.y < 0.0f)
            radianceSample = passData.LowerHemisphereColor.xyz;
        else
            radianceSample = radianceMap.SampleLevel(sLinearWrap, Li, 0).xyz;
        #else
        radianceSample = radianceMap.SampleLevel(sLinearWrap, Li, 0.0f).xyz;
        #endif
        
        irradiance += 2.0f * radianceSample * cosTheta;
    }
    irradiance /= float3(samples, samples, samples);

    irradianceMap[threadID] = float4(irradiance, 1.0f);
}