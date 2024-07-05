struct VS_OUT
{
    float4 outPositionCS    : SV_POSITION;
    float3 outTexCoord      : TEXCOORD0;
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

struct VPConstantBuffer
{
    matrix VPMatrix;
};

struct SkyBoxPassData
{
    uint VPIndex;
    uint SkyboxTextureIndex;
};

ConstantBuffer<SkyBoxPassData> skyboxPassData : register(b0, space0);

VS_OUT vs_main(uint vertexID : SV_VertexID)
{
    VS_OUT vsOut = (VS_OUT)0;
    
    const float3 position = cubeVertices[cubeIndices[vertexID]];
    
    const ConstantBuffer<VPConstantBuffer> viewProjectionData = ResourceDescriptorHeap[skyboxPassData.VPIndex];
    
    vsOut.outPositionCS = mul(viewProjectionData.VPMatrix, float4(position, 0.0f));
    vsOut.outPositionCS.z = vsOut.outPositionCS.w;
    vsOut.outTexCoord = position;
    
    return vsOut;
}