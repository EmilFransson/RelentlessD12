struct Vertex
{
    float3 inPositionLS;
};

struct VS_OUT
{
    float4 outPositionCS	: SV_Position;
    float3 outPositionWS    : POSITIONWS;
    float3 outColor         : COLOR;
};

struct BatchData
{
    uint worldMatrixIndex1;
    uint worldMatrixIndex2;
};

struct VPConstantBuffer
{
    matrix VPMatrix;
};

struct Transform
{
    matrix worldMatrix;
};

struct InstanceData
{
    float3 Position;
    float Pad;
    float3 Color;
    float Pad2;
};

struct InstanceDataSBIndex
{
    uint Index;
};

ConstantBuffer<VPConstantBuffer> vpConstantBuffer : register(b0, space0);
ConstantBuffer<BatchData> batchData : register(b2, space0);
ConstantBuffer<InstanceDataSBIndex> instanceDataSBIndex : register(b3, space0);

StructuredBuffer<Vertex> vertices : register(t0, space0);

VS_OUT vs_main(uint vertexID : SV_VertexID, uint instanceID : SV_InstanceID)
{
    VS_OUT vsOut = (VS_OUT)0;
    Vertex input = vertices[vertexID];

    //Instancing shenaningans:
    StructuredBuffer<InstanceData> instanceDataSB = ResourceDescriptorHeap[instanceDataSBIndex.Index];
    InstanceData instanceData = instanceDataSB[instanceID];
    input.inPositionLS += instanceData.Position;

    matrix worldMatrix;
    if (instanceID > 399)
    {
        ConstantBuffer<Transform> transform = ResourceDescriptorHeap[batchData.worldMatrixIndex2];
        worldMatrix = transform.worldMatrix;
    }
    else
    {
        ConstantBuffer<Transform> transform = ResourceDescriptorHeap[batchData.worldMatrixIndex1];
        worldMatrix = transform.worldMatrix;
    }

    matrix wvp = mul(vpConstantBuffer.VPMatrix, worldMatrix);
    vsOut.outPositionCS = mul(wvp, float4(input.inPositionLS, 1.0f));
    vsOut.outPositionWS = mul(worldMatrix, float4(input.inPositionLS, 1.0f)).xyz;
    vsOut.outColor = instanceData.Color;

    return vsOut;
}