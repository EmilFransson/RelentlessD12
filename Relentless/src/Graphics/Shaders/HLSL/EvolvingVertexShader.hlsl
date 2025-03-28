#include "CommonBindings.hlsli"

struct PerDrawData
{
    uint InstanceIndex;
};

ConstantBuffer<PerDrawData> perDrawData : register(b0, space0);

struct VS_OUT
{
    float4 outPositionCS    : SV_Position;
    float3 outPositionWS    : POSITIONWS;
    float3 outNormalWS      : NORMALWS;
    float3 outTangentWS     : TANGENTWS;
    float3 outBiTangentWS   : BITANGENTWS;
    float2 outTexCoords     : TEXCOORDS;
};

VS_OUT vs_main(uint vertexID : SV_VertexID)
{
	VS_OUT vsOut = (VS_OUT)0;
    
    InstanceData instanceData = GetInstance(perDrawData.InstanceIndex);
    MeshData meshData = GetMesh(instanceData.MeshDataIndex);
    Vertex vertex = LoadVertex(meshData, vertexID);

    float4 worldPos = mul(instanceData.LocalToWorld, float4(vertex.inPositionLS, 1.0f));
    vsOut.outPositionCS = mul(cView.WorldToClip, worldPos);
    vsOut.outPositionWS = mul(instanceData.LocalToWorld, float4(vertex.inPositionLS, 1.0f)).xyz;
    vsOut.outNormalWS = mul(instanceData.LocalToWorld, float4(vertex.inNormalLS, 0.0f)).xyz;
    vsOut.outTangentWS = mul(instanceData.LocalToWorld, float4(vertex.inTangentLS, 0.0f)).xyz;
    vsOut.outBiTangentWS = mul(instanceData.LocalToWorld, float4(vertex.inBiTangentLS, 0.0f)).xyz;
    vsOut.outTexCoords = vertex.inTexCoords;
    
    return vsOut;
}