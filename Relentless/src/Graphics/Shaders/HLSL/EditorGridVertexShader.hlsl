#include "CommonBindings.hlsli"

struct VS_OUT
{
    float4 outPositionCS    : SV_Position;
    float3 outPositionWS    : POSITIONWS;
    float3 outColor         : COLOR;
};

struct EditorGridInstanceData
{
    float3 Position;
    float Pad;
    float3 Color;
    float Pad2;
};

struct VertexShaderPerFrameData
{
    float4x4 BatchDataTransformVerticalMatrix;
    float4x4 BatchDataTransformHorizontalMatrix;
    uint InstanceDataSBIndex;
};

ConstantBuffer<VertexShaderPerFrameData> vsPerFrameData : register(b1, space0);

static const float3 vertices[2] =
{
    float3(-0.5f, 0.0f, 0.0f),
    float3(0.5f, 0.0f, 0.0f)
};

inline matrix GetInstanceWorldMatrix(uint instanceID)
{
    if (instanceID > 399)
    {
        return vsPerFrameData.BatchDataTransformHorizontalMatrix;
    }
    else
    {
        return vsPerFrameData.BatchDataTransformVerticalMatrix;
    }
    //const uint instanceTransformIndex = (instanceID > 399) ? vsPerFrameData.BatchDataTransformHorizontalCBIndex : vsPerFrameData.BatchDataTransformVerticalCBIndex;
    //const ConstantBuffer<Transform> transform = ResourceDescriptorHeap[instanceTransformIndex];
    //return transform.WorldMatrix;
}

inline float3 GetInstanceInputPosition(uint vertexID, float3 instancePosition)
{
    float3 inputPosition = vertices[vertexID];
    inputPosition += instancePosition;
    return inputPosition;
}

VS_OUT vs_main(uint vertexID : SV_VertexID, uint instanceID : SV_InstanceID)
{
    const StructuredBuffer<EditorGridInstanceData> instanceDataSB = ResourceDescriptorHeap[vsPerFrameData.InstanceDataSBIndex];
    const EditorGridInstanceData instanceData = instanceDataSB[instanceID];
    
    const float3 inputPosition = GetInstanceInputPosition(vertexID, instanceData.Position);
    const matrix worldMatrix = GetInstanceWorldMatrix(instanceID);
    
    const matrix wvp = mul(cView.WorldToClip, worldMatrix);
    
    VS_OUT vsOut = (VS_OUT) 0;
    vsOut.outPositionCS = mul(wvp, float4(inputPosition, 1.0f));
    vsOut.outPositionWS = mul(worldMatrix, float4(inputPosition, 1.0f)).xyz;
    vsOut.outColor = instanceData.Color;

    return vsOut;
}