SamplerState sampler_LINEAR : register(s0, space0);

struct PS_IN
{
    float4 inPositionSS : SV_POSITION;
    float3 inTexCoord   : POSITIONWS;
};

struct CreateTextureCubePassData
{
    uint TextureIndex;
    uint VPIndex;
};

ConstantBuffer<CreateTextureCubePassData> createTextureCubePassData : register(b0, space0);

static const float PI = 3.14159265359f;

float4 ps_main(PS_IN psIn) : SV_Target
{
    const float3 normal = normalize(psIn.inTexCoord);
    const float3 right = normalize(cross(float3(0.0f, 1.0f, 0.0f), normal));
    const float3 up = normalize(cross(normal, right));

    TextureCube environmentTextureCube = ResourceDescriptorHeap[createTextureCubePassData.TextureIndex];
    float nrOfSamples = 0.0f;
    const float sampleDelta = 0.025f;
    
    float3 irradiance = float3(0.0f, 0.0f, 0.0f);
    for (float phi = 0.0f; phi < 2.0f * PI; phi += sampleDelta)
    {
        for (float theta = 0.0f; theta < 0.5f * PI; theta += sampleDelta)
        {
            const float3 tangentSample = float3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            const float3 sampleVec = (tangentSample.x * right) + (tangentSample.y * up) + (tangentSample.z * normal);
            irradiance += environmentTextureCube.Sample(sampler_LINEAR, sampleVec).rgb * cos(theta) * sin(theta);
            nrOfSamples++;
        }
    }
    
    irradiance *= (PI / nrOfSamples);
    return float4(irradiance, 1.0f);
}
