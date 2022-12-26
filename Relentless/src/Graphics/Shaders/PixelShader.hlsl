struct PS_IN
{
	float4 inPositionSS	: SV_Position;
    float2 inTexCoords  : TEXCOORDS;
};

struct PerDrawData
{
    uint colorIndex;
};

struct Color
{
    float3 col;
};

ConstantBuffer<PerDrawData> perDrawData : register(b3, space0);

float4 ps_main(in PS_IN psIn) : SV_TARGET
{
    ConstantBuffer<Color> color = ResourceDescriptorHeap[perDrawData.colorIndex];

    return float4(color.col, 1.0f);
}