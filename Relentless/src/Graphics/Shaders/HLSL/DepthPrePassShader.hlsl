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
    const float displacement = EvaluateDisplacement(material, adjustedTexCoords);
    const float3 positionLS = vertex.inPositionLS + (vertex.inNormalLS * displacement * material.HeightFactor);
    const float4 worldPos = mul(instanceData.LocalToWorld, float4(positionLS, 1.0f));
    
    vsOut.PositionCS = mul(cView.WorldToClip, worldPos);
    
    #ifdef ALPHA_MASK
        vsOut.TexCoords = adjustedTexCoords;
    #endif
    
    return vsOut;
}

void ps_main(VS_OUT psIn)
{
#ifdef ALPHA_MASK
    const InstanceData instanceData = GetInstance(perDrawData.InstanceIndex);
    const Material material = GetMaterial(instanceData.MaterialIndex);
    
    Texture2D albedoTexture = ResourceDescriptorHeap[material.AlbedoIndex];
    const float4 albedoColor = albedoTexture.Sample(sAnisoWrap, psIn.TexCoords) * material.BaseColorFactor;
    
    clip(albedoColor.a < 0.1f ? -1 : 1);
#endif
}