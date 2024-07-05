SamplerState Sampler_LINEAR : register(s0, space0);

struct PS_IN
{
    float4 inPositionSS : SV_POSITION;
    float3 inPositionLS : POSITIONWS;
};

struct CreateTextureCubePassData
{
    uint TextureIndex;
    uint VPIndex;
};

struct RoughnessData
{
    float Roughness;
};

struct MipRoughnessPassData
{
    uint RoughnessIndex;
};

ConstantBuffer<CreateTextureCubePassData> createTextureCubePassData : register(b0, space0);
ConstantBuffer<MipRoughnessPassData> mipRoughnessPassData : register(b1, space0);

static const float PI = 3.14159265359f;

float RadicalInverse_VdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10;
}

float3 ImportanceSampleGGX(uint i, uint sampleCount, float3 N, float roughness)
{
    const float2 hammersley = float2(float(i) / float(sampleCount), RadicalInverse_VdC(i));
    const float a = roughness * roughness;
    
    const float phi = 2.0f * PI * hammersley.x;
    const float cosTheta = sqrt((1.0f - hammersley.y) / (1.0f + (a*a - 1.0f) * hammersley.y));
    const float sinTheta = sqrt(1.0f - cosTheta*cosTheta);

    float3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;
    
    const float3 up = abs(N.z) < 0.999 ? float3(0.0f, 0.0f, 1.0f) : float3(1.0f, 0.0f, 0.0f);
    const float3 tangent = normalize(cross(up, N));
    const float3 biTangent = cross(N, tangent);

    const float3 sampleVec = tangent * H.x + biTangent * H.y + N * H.z;
    return normalize(sampleVec);
}

float4 ps_main(PS_IN psIn) : SV_Target
{
    const float3 normal = normalize(psIn.inPositionLS);
    const float3 view = normal;
    
    const uint SAMPLE_COUNT = 1024u;
    float totalWeight = 0.0f;
    float3 prefilteredColor = float3(0.0f, 0.0f, 0.0f);

    TextureCube environmentMap = ResourceDescriptorHeap[createTextureCubePassData.TextureIndex];
    ConstantBuffer<RoughnessData> roughnessData = ResourceDescriptorHeap[mipRoughnessPassData.RoughnessIndex];
    
    for (uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        const float3 H = ImportanceSampleGGX(i, SAMPLE_COUNT, normal, roughnessData.Roughness);
        const float3 L = reflect(-view, H);
        const float NDotL = max(dot(normal, L), 0.0f);

        if (NDotL > 0.0f)
        {
            prefilteredColor += environmentMap.Sample(Sampler_LINEAR, L).rgb * NDotL;
            totalWeight += NDotL;
        }
    }
    prefilteredColor = prefilteredColor / totalWeight;

    return float4(prefilteredColor, 1.0f);
}