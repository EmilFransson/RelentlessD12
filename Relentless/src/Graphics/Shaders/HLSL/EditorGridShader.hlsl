#include "Common.hlsli"
#include "CommonBindings.hlsli"

struct VS_OUT
{
    float4 PositionCS   : SV_Position;
    float3 PositionWS   : POSITIONWS;
    float3 Color        : COLOR;
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
    vsOut.PositionCS = mul(wvp, float4(inputPosition, 1.0f));
    vsOut.PositionWS = mul(worldMatrix, float4(inputPosition, 1.0f)).xyz;
    vsOut.Color = instanceData.Color;

    return vsOut;
}

//float Lerp(float a, float b, float t)
//{
//    return (1.0f - t) * a + b * t;
//}
//
//float InverseLerp(float a, float b, float v)
//{
//    return (v - a) / (b - a);
//}
//
//float Remap(float iMin, float iMax, float oMin, float oMax, float v)
//{
//    float t = InverseLerp(iMin, iMax, v);
//    return Lerp(oMin, oMax, t);
//}

static const float BOUNDARY_EPSILON = 0.01f;
static const float3 RED_GRID_COLOR = float3(0.63f, 0.0f, 0.0f);
static const float3 BLUE_GRID_COLOR = float3(0.0f, 0.0f, 0.63f);
static const float MAX_DISTANCE_TO_CAMERA = 10.0f;

float4 ps_main(in VS_OUT psIn) : SV_TARGET
{
    const float distanceToCamera = length(psIn.PositionWS - cView.ViewLocation);
    const float t = distanceToCamera / MAX_DISTANCE_TO_CAMERA;
    const float finalAlpha = saturate(Remap(MAX_DISTANCE_TO_CAMERA, 0.0f, 0.0f, 0.6f, t));
   
   /*This if-else clause checks whether the grid line to be rendered is at x = 0 or z = 0. These lines
     are colored red or blue to highlight the x-axis and z-axis in the world.*/
    float3 finalColor = 0.0f;
    if (psIn.PositionWS.x > -BOUNDARY_EPSILON && psIn.PositionWS.x < BOUNDARY_EPSILON)
    {
        finalColor = BLUE_GRID_COLOR;
    }
    else if (psIn.PositionWS.z > -BOUNDARY_EPSILON && psIn.PositionWS.z < BOUNDARY_EPSILON)
    {
        finalColor = RED_GRID_COLOR;
    }
    else
    {
        finalColor = float3(psIn.Color);
    }
    
    return float4(finalColor, finalAlpha);
}