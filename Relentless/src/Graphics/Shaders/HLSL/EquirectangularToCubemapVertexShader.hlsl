struct VS_OUT
{
    float4 outPositionCS : SV_POSITION;
    float3 outPositionWS : POSITIONWS;
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

struct CreateTextureCubePassData
{
    uint TextureIndex;
    uint VPIndex;
};

ConstantBuffer<CreateTextureCubePassData> createTextureCubePassData : register(b0, space0);

VS_OUT vs_main(uint vertexID : SV_VertexID)
{
    VS_OUT output = (VS_OUT)0;
    
    const uint index = cubeIndices[vertexID];
    const float3 position = cubeVertices[index];
    
    const ConstantBuffer<VPConstantBuffer> vpMatrixCB = ResourceDescriptorHeap[createTextureCubePassData.VPIndex];
    output.outPositionCS = mul(vpMatrixCB.VPMatrix, float4(position, 1.0f));
    output.outPositionWS = position;
    
    return output;
}