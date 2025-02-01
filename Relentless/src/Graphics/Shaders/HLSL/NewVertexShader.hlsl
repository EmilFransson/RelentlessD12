const static float3 positions[] =
{
    float3(-1.0f, 3.0f, 0.0f),
    float3(3.0f, -1.0f, 0.0f),
    float3(-1.0f, -1.0f, 0.0f)
};

const static float2 uvs[] =
{
    float2(0.0f, -1.0f),
    float2(2.0f, 1.0f),
    float2(0.0f, 1.0f)
};

struct VS_OUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORDS;
};

VS_OUT vs_main(uint id : SV_VertexID)
{
    VS_OUT output = (VS_OUT)0;
    output.pos = float4(positions[id], 1.f);
    output.uv = uvs[id];

    return output;
}