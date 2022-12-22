SamplerState pointSampler : register(s0, space0);

float3 LinearToSRGB(float3 color)
{
    return pow(abs(color), 1 / 2.2f);
}

float3 ToneMapACES(float3 hdrColor)
{
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return saturate((hdrColor * (a * hdrColor + b)) / (hdrColor * (c * hdrColor + d) + e));
}

struct TextureMetaData
{
    uint textureIndex;
};

struct PS_IN
{
    float4 inPositionSS : SV_Position;
    float2 inTexCoords : TEXCOORDS;
};

ConstantBuffer<TextureMetaData> TextureData : register(b2, space0);

float4 ps_main(in PS_IN psIn) : SV_TARGET
{
    Texture2D myTex = ResourceDescriptorHeap[TextureData.textureIndex];
    
    float4 hdrTextureColor = myTex.Sample(pointSampler, float2(psIn.inTexCoords.x, psIn.inTexCoords.y));
    float3 sdr = ToneMapACES(hdrTextureColor.xyz);
    float3 sRGB = LinearToSRGB(sdr);
    return float4(sRGB, hdrTextureColor.a);
}