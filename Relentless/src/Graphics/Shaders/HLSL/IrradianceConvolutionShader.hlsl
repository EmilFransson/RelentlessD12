#include "Common.hlsli"
#include "CommonBindings.hlsli"

struct VS_OUT
{
    float4 PositionCS   : SV_POSITION;
    float3 TexCoord     : POSITIONWS;
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
    uint CubemapTextureIndex;
    float3 Padding;
};

ConstantBuffer<PerDrawData> PassData : register(b0, space0);

VS_OUT vs_main(uint vertexID : SV_VertexID)
{
    VS_OUT output = (VS_OUT) 0;
    
    const uint index = cubeIndices[vertexID];
    const float3 position = cubeVertices[index];
    
    output.PositionCS = mul(PassData.VPMatrix, float4(position, 1.0f));
    output.TexCoord = position;
    
    return output;
}

float4 ps_main(VS_OUT psIn) : SV_Target
{
    const float3 normal = normalize(psIn.TexCoord);
    const float3 upVector = abs(normal.y) < 0.999f ? float3(0.0f, 1.0f, 0.0f) : float3(0.0f, 0.0f, 1.0f);
    const float3 right = normalize(cross(upVector, normal));
    const float3 up = normalize(cross(normal, right));

    TextureCube environmentTextureCube = ResourceDescriptorHeap[PassData.CubemapTextureIndex];
    float nrOfSamples = 0.0f;
    const float sampleDelta = 0.025f;
    
    float3 irradiance = float3(0.0f, 0.0f, 0.0f);
    for (float phi = 0.0f; phi < 2.0f * PI; phi += sampleDelta)
    {
        for (float theta = 0.0f; theta < 0.5f * PI; theta += sampleDelta)
        {
            const float3 tangentSample = float3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            const float3 sampleVec = (tangentSample.x * right) + (tangentSample.y * up) + (tangentSample.z * normal);
            irradiance += environmentTextureCube.Sample(sLinearWrap, sampleVec).rgb * cos(theta) * sin(theta);
            nrOfSamples++;
        }
    }
    
    irradiance *= (PI / nrOfSamples);
    return float4(irradiance, 1.0f);
}
