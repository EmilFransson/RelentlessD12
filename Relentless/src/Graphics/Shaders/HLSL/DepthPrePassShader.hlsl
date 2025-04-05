#include "CommonBindings.hlsli"

struct PerDrawData
{
    uint InstanceIndex;
};

ConstantBuffer<PerDrawData> perDrawData : register(b0, space0);

struct VS_OUT
{
    float4 PositionCS   : SV_Position;
#ifdef ALPHA_MASK
    float2 TexCoords    : TEXCOORDS;
#endif
};

VS_OUT vs_main(uint vertexID : SV_VertexID)
{
    VS_OUT vsOut = (VS_OUT)0;
    
    const InstanceData instanceData = GetInstance(perDrawData.InstanceIndex);
    const MeshData meshData = GetMesh(instanceData.MeshDataIndex);
    const Material material = GetMaterial(instanceData.MaterialIndex);
    const Vertex vertex = LoadVertex(meshData, vertexID);

    const float2 adjustedTexCoords = (vertex.inTexCoords * material.TilingFactor) + material.Offset;

    float3 positionLS = vertex.inPositionLS;
    if (material.HeightMapIndex != INVALID_DESCRIPTOR_INDEX)
    {
        Texture2D heightMap = ResourceDescriptorHeap[material.HeightMapIndex];
        const float height = heightMap.SampleLevel(sLinearWrap, adjustedTexCoords, 0).r;
        positionLS += vertex.inNormalLS * height * material.HeightFactor;
    }
    
    const float4 worldPos = mul(instanceData.LocalToWorld, float4(positionLS, 1.0f));
    vsOut.PositionCS = mul(cView.WorldToClip, worldPos);
    
    #ifdef ALPHA_MASK
        vsOut.TexCoords = adjustedTexCoords;
    #endif
    
    return vsOut;
}