SamplerState sampler_LINEAR : register(s0, space0);

struct PS_IN
{
    float4 inPositionSS : SV_POSITION;
    float3 inPositionWS : POSITIONWS;
};

struct CreateTextureCubePassData
{
    uint TextureIndex;
    uint VPIndex;
};

ConstantBuffer<CreateTextureCubePassData> createTextureCubePassData : register(b0, space0);

static const float2 invAtan = float2(0.1591, 0.3183);
float2 SampleSphericalMap(float3 direction)
{
    float2 uv = float2(atan2(direction.z, direction.x), asin(direction.y));
    uv *= invAtan;
    uv += 0.5f;
    
    uv.y = 1.0f - uv.y;
    return uv;
}

float4 ps_main(PS_IN psIn) : SV_Target
{
    const float2 uv = SampleSphericalMap(normalize(psIn.inPositionWS));
    Texture2D equirectangularMap = ResourceDescriptorHeap[createTextureCubePassData.TextureIndex];
    const float3 color = equirectangularMap.Sample(sampler_LINEAR, uv).rgb;
    
    return float4(color, 1.0f);
}