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

struct PerDrawData
{
    uint materialIndex;
    uint worldMatrixIndex;
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
ConstantBuffer<PerDrawData> perDrawData : register(b3, space0);

ConstantBuffer<InstanceDataSBIndex> instanceDataSBIndex : register(b0, space1);

StructuredBuffer<Vertex> vertices : register(t0, space0);

VS_OUT vs_main(uint vertexID : SV_VertexID, uint instanceID : SV_InstanceID)
{
    VS_OUT vsOut = (VS_OUT)0;
    Vertex input = vertices[vertexID];

    ConstantBuffer<Transform> transform = ResourceDescriptorHeap[perDrawData.worldMatrixIndex];
    
    //Instancing shenaningans:
    StructuredBuffer<InstanceData> instanceDataSB = ResourceDescriptorHeap[instanceDataSBIndex.Index];
    InstanceData instanceData = instanceDataSB[instanceID];
    input.inPositionLS += instanceData.Position;

    matrix wvp = mul(vpConstantBuffer.VPMatrix, transform.worldMatrix);
    vsOut.outPositionCS = mul(wvp, float4(input.inPositionLS, 1.0f));
    vsOut.outPositionWS = mul(transform.worldMatrix, float4(input.inPositionLS, 1.0f)).xyz;
    vsOut.outColor = instanceData.Color;

    return vsOut;
}