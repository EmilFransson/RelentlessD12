struct VS_OUT
{
    float4 outPositionCS	: SV_Position;
    float3 outPositionWS    : POSITIONWS;
    float3 outColor         : COLOR;
};

struct VPConstantBuffer
{
    matrix VPMatrix;
};

struct Transform
{
    matrix WorldMatrix;
};

struct InstanceData
{
    float3 Position;
    float Pad;
    float3 Color;
    float Pad2;
};

struct VertexShaderPerFrameData
{
    uint InstanceDataSBIndex;
    uint VPMatrixCBIndex;
    uint BatchDataTransformHorizontalCBIndex;
    uint BatchDataTransformVerticalCBIndex;
};

ConstantBuffer<VertexShaderPerFrameData> vsPerFrameData : register(b0, space0);

static const float3 vertices[2] =
{
    float3(-0.5f, 0.0f, 0.0f),
    float3(0.5f, 0.0f, 0.0f)
};

inline matrix GetInstanceWorldMatrix(uint instanceID)
{
    const uint instanceTransformIndex = (instanceID > 399) ? vsPerFrameData.BatchDataTransformHorizontalCBIndex : vsPerFrameData.BatchDataTransformVerticalCBIndex;
    const ConstantBuffer<Transform> transform = ResourceDescriptorHeap[instanceTransformIndex];
    return transform.WorldMatrix;
}

inline float3 GetInstanceInputPosition(uint vertexID, float3 instancePosition)
{
    float3 inputPosition = vertices[vertexID];
    inputPosition += instancePosition;
    return inputPosition;
}

VS_OUT vs_main(uint vertexID : SV_VertexID, uint instanceID : SV_InstanceID)
{
    const StructuredBuffer<InstanceData> instanceDataSB = ResourceDescriptorHeap[vsPerFrameData.InstanceDataSBIndex];
    const InstanceData instanceData = instanceDataSB[instanceID];
    
    const float3 inputPosition = GetInstanceInputPosition(vertexID, instanceData.Position);
    const matrix worldMatrix = GetInstanceWorldMatrix(instanceID);
    
    const ConstantBuffer<VPConstantBuffer> vp = ResourceDescriptorHeap[vsPerFrameData.VPMatrixCBIndex];
    const matrix wvp = mul(vp.VPMatrix, worldMatrix);
    
    VS_OUT vsOut = (VS_OUT) 0;
    vsOut.outPositionCS = mul(wvp, float4(inputPosition, 1.0f));
    vsOut.outPositionWS = mul(worldMatrix, float4(inputPosition, 1.0f)).xyz;
    vsOut.outColor = instanceData.Color;

    return vsOut;
}