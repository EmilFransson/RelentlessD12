SamplerState sampler_LINEAR : register(s1, space0);

struct Vertex
{
    float3 inPositionLS;
    float3 inNormalLS;
    float3 inTangentLS;
    float3 inBiTangentLS;
    float2 inTexCoords;
};

struct VS_OUT
{
    float4 outPositionCS : SV_Position;
};

StructuredBuffer<Vertex> vertices : register(t0, space0);
StructuredBuffer<unsigned int> indices : register(t1, space0);

struct Transform
{
    matrix worldMatrix;
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

struct Material
{
    float4 color;
    float4 emissionColor;
    
    float metallic;
    float3 padding;

    float emissionIntensity;
    float roughness;
    uint albedoIndex;
    uint metallicIndex;
    
    uint roughnessIndex;
    uint normalIndex;
    uint heightMapIndex;
    uint ambientOcclusionIndex;
    
    uint emissionIndex;
    float heightScale;
    float aoScale;
    uint combinedRoughnessMetalnessMap;

    float2 tilingFactor;
    float2 offset;
};

ConstantBuffer<VPConstantBuffer> vpConstantBuffer : register(b0, space0);
ConstantBuffer<PerDrawData> perDrawData : register(b3, space0);

static const uint NO_USE = 0xFFFFFFFF;

VS_OUT vs_main(uint vertexID : SV_VertexID)
{
    VS_OUT vsOut = (VS_OUT) 0;
    Vertex input = vertices[indices[vertexID]];

    ConstantBuffer<Transform> transform = ResourceDescriptorHeap[perDrawData.worldMatrixIndex];
    ConstantBuffer<Material> material = ResourceDescriptorHeap[perDrawData.materialIndex];
	
    float2 adjustedTexCoords = (input.inTexCoords * material.tilingFactor) + material.offset;

    float3 positionLS = input.inPositionLS;
    if (material.heightMapIndex != NO_USE)
    {
        Texture2D heightMap = ResourceDescriptorHeap[material.heightMapIndex];
        float height = heightMap.SampleLevel(sampler_LINEAR, adjustedTexCoords, 0).r;
        positionLS += input.inNormalLS * height * material.heightScale;
    }

    matrix wvp = mul(vpConstantBuffer.VPMatrix, transform.worldMatrix);
    vsOut.outPositionCS = mul(wvp, float4(positionLS, 1.0f));
    return vsOut;
}