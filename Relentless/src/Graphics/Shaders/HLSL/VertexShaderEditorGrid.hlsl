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

struct ViewProjectionBufferIndex
{
    uint Index;
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

ConstantBuffer<ViewProjectionBufferIndex> vpConstantBuffer : register(b0, space0);
ConstantBuffer<BatchData> batchData : register(b2, space0);
ConstantBuffer<InstanceDataSBIndex> instanceDataSBIndex : register(b3, space0);

VS_OUT vs_main(uint vertexID : SV_VertexID, uint instanceID : SV_InstanceID)
{
    VS_OUT vsOut = (VS_OUT)0;
  
    float3 vertices[2] =
    {
        float3(-0.5f, 0.0f, 0.0f),
        float3(0.5f, 0.0f, 0.0f)
    };
    
    float3 inputPosition = vertices[vertexID];

    StructuredBuffer<InstanceData> instanceDataSB = ResourceDescriptorHeap[instanceDataSBIndex.Index];
    InstanceData instanceData = instanceDataSB[instanceID];
    inputPosition += instanceData.Position;
    
    matrix worldMatrix = (matrix)0;
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

    ConstantBuffer<VPConstantBuffer> vp = ResourceDescriptorHeap[vpConstantBuffer.Index];
    
    matrix wvp = mul(vp.VPMatrix, worldMatrix);
    vsOut.outPositionCS = mul(wvp, float4(inputPosition, 1.0f));
    vsOut.outPositionWS = mul(worldMatrix, float4(inputPosition, 1.0f)).xyz;
    vsOut.outColor = instanceData.Color;

    return vsOut;
}