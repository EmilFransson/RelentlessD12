#include "Common.hlsli"
#include "CommonBindings.hlsli"
#include "Lighting.hlsli"

struct PerDrawData
{
    uint InstanceIndex;
};

ConstantBuffer<PerDrawData> perDrawData : register(b0, space0);

struct VS_OUT
{
    float4 PositionCS   : SV_Position;
    float3 PositionWS   : POSITIONWS;
    float3 NormalWS     : NORMALWS;
    float3 TangentWS    : TANGENTWS;
    float3 BiTangentWS  : BITANGENTWS;
    float2 TexCoords    : TEXCOORDS;
};

VS_OUT vs_main(uint vertexID : SV_VertexID, uint instanceID : SV_InstanceID)
{
    VS_OUT vsOut = (VS_OUT)0;
    
    const uint instanceIndex = perDrawData.InstanceIndex + instanceID;
    const InstanceData instanceData = GetInstance(instanceIndex /*perDrawData.InstanceIndex*/);
    const MeshData meshData = GetMesh(instanceData.MeshDataIndex);
    const Material material = GetMaterial(instanceData.MaterialIndex);
    const Vertex vertex = LoadVertex(meshData, vertexID);

    const float2 adjustedTexCoords = (vertex.inTexCoords * material.TilingFactor) + material.Offset;

    const float displacement = EvaluateDisplacement(material, adjustedTexCoords);
    const float3 positionLS = vertex.inPositionLS + (vertex.inNormalLS * displacement * material.HeightFactor);
    
    const float4 worldPos = mul(instanceData.LocalToWorld, float4(positionLS, 1.0f));
    vsOut.PositionCS = mul(cView.WorldToClip, worldPos);
    vsOut.PositionWS = worldPos.xyz;
    vsOut.NormalWS = mul(instanceData.LocalToWorld, float4(vertex.inNormalLS, 0.0f)).xyz;
    vsOut.TangentWS = mul(instanceData.LocalToWorld, float4(vertex.inTangentLS, 0.0f)).xyz;
    vsOut.BiTangentWS = mul(instanceData.LocalToWorld, float4(vertex.inBiTangentLS, 0.0f)).xyz;
    vsOut.TexCoords = adjustedTexCoords;
    
    return vsOut;
}

float4 ps_main(VS_OUT psIn) : SV_TARGET
{
    const InstanceData instanceData = GetInstance(perDrawData.InstanceIndex);
    const Material material = GetMaterial(instanceData.MaterialIndex);
    
    MaterialEvaluationInputs inputs = (MaterialEvaluationInputs)0;
    inputs.UV = psIn.TexCoords;
    inputs.NormalWS = psIn.NormalWS;
    inputs.TangentWS = psIn.TangentWS;
    inputs.BitangentWS = psIn.BiTangentWS;
    
    const MaterialSurface surface = EvaluateMaterial(material, inputs);
    const float3 outgoingRadiance = EvaluateLights(surface, psIn.PositionWS);
    const float3 ibl = EvaluateIBL(surface, psIn.PositionWS);
    const float3 ambientColor = (ibl * surface.AmbientOcclusion) + surface.EmissiveColor;
   
    const float3 outColor = ambientColor + outgoingRadiance;
    return float4(outColor, surface.Alpha);
}