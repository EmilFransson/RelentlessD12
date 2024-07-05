SamplerState SkyboxSampler_LINEAR : register(s0, space0);

struct PS_IN
{
    float4 inPositionSS : SV_POSITION;
    float3 inTexCoord   : TEXCOORD0;
};

struct SkyBoxPassData
{
    uint VPIndex;
    uint SkyboxTextureIndex;
};

ConstantBuffer<SkyBoxPassData> skyboxPassData : register(b0, space0);

float4 ps_main(PS_IN psIn) : SV_Target
{
    TextureCube skyboxTexture = ResourceDescriptorHeap[skyboxPassData.SkyboxTextureIndex];
    return skyboxTexture.Sample(SkyboxSampler_LINEAR, psIn.inTexCoord);
}