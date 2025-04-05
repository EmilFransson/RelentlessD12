#include "CommonBindings.hlsli"

struct PerDrawData
{
    float3 Color;
    uint InstanceIndex;
};

ConstantBuffer<PerDrawData> perDrawData : register(b0, space0);

struct VS_OUT
{
    float4 PositionCS : SV_Position;
};

VS_OUT vs_main(uint vertexID : SV_VertexID)
{
    VS_OUT vsOut = (VS_OUT) 0;
    
    InstanceData instanceData = GetInstance(perDrawData.InstanceIndex);
    MeshData meshData = GetMesh(instanceData.MeshDataIndex);
    Material material = GetMaterial(instanceData.MaterialIndex);
    Vertex vertex = LoadVertex(meshData, vertexID);

    float3 positionLS = vertex.inPositionLS;
    if (material.HeightMapIndex != INVALID_DESCRIPTOR_INDEX)
    {
        const float2 adjustedTexCoords = (vertex.inTexCoords * material.TilingFactor) + material.Offset;
        Texture2D heightMap = ResourceDescriptorHeap[material.HeightMapIndex];
        const float height = heightMap.SampleLevel(sLinearWrap, adjustedTexCoords, 0).r;
        positionLS += vertex.inNormalLS * height * material.HeightFactor;
    }
    
    const float4 worldPos = mul(instanceData.LocalToWorld, float4(positionLS, 1.0f));
    vsOut.PositionCS = mul(cView.WorldToClip, worldPos);
    
    return vsOut;
}

float4 ps_main(VS_OUT psIn) : SV_TARGET
{
    return float4(perDrawData.Color, 1.0f);
}