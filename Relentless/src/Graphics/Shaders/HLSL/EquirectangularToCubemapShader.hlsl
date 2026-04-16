#include "CommonBindings.hlsli"

struct VS_OUT
{
    float4 PositionCS : SV_POSITION;
    float3 PositionWS : POSITIONWS;
};

static const float3 cubeVertices[8] =
{
    float3(-1.0f, 1.0f, -1.0f), // 0
    float3(1.0f, 1.0f, -1.0f), // 1
    float3(-1.0f, -1.0f, -1.0f), // 2
    float3(1.0f, -1.0f, -1.0f), // 3
    float3(-1.0f, 1.0f, 1.0f), // 4
    float3(1.0f, 1.0f, 1.0f), // 5
    float3(-1.0f, -1.0f, 1.0f), // 6
    float3(1.0f, -1.0f, 1.0f) // 7
};

static const uint cubeIndices[36] =
{
    0, 1, 2, // side 1
    2, 1, 3,
    4, 0, 6, // side 2
    6, 0, 2,
    7, 5, 6, // side 3
    6, 5, 4,
    3, 1, 7, // side 4
    7, 1, 5,
    4, 5, 0, // side 5
    0, 5, 1,
    3, 7, 2, // side 6
    2, 7, 6,
};

struct PerDrawData
{
    float4x4 VPMatrix;
    uint EquirectTextureIndex;
    float3 Padding;
};

ConstantBuffer<PerDrawData> PassData : register(b0, space0);

VS_OUT vs_main(uint vertexID : SV_VertexID)
{
    VS_OUT output = (VS_OUT) 0;
    
    const uint index = cubeIndices[vertexID];
    const float3 position = cubeVertices[index];
    
    output.PositionCS = mul(PassData.VPMatrix, float4(position, 1.0f));
    output.PositionWS = position;
    
    return output;
}

static const float2 invAtan = float2(0.1591, 0.3183);
float2 SampleSphericalMap(float3 direction)
{
    float2 uv = float2(atan2(direction.z, direction.x), asin(direction.y));
    uv *= invAtan;
    uv += 0.5f;
    
    uv.y = 1.0f - uv.y;
    return uv;
}

float4 ps_main(VS_OUT psIn) : SV_Target
{
    const float2 uv = SampleSphericalMap(normalize(psIn.PositionWS));
    Texture2D equirectangularMap = ResourceDescriptorHeap[PassData.EquirectTextureIndex];
    const float3 color = equirectangularMap.Sample(sLinearWrapClamp, uv).rgb;
    return float4(color, 1.0f);
}