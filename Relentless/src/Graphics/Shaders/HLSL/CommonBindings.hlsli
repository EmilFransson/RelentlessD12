#pragma once

#include "../Interop/ShaderInterop.h"

ConstantBuffer<ViewUniforms> cView : 						register(b2);

//Static samplers
SamplerState sLinearWrap :								  	register(s0, space1);
SamplerState sLinearClamp :								 	register(s1, space1);
SamplerState sLinearBorder :								register(s2, space1);
SamplerState sPointWrap :								   	register(s3, space1);
SamplerState sPointClamp :								  	register(s4, space1);
SamplerState sPointBorder :								 	register(s5, space1);
SamplerState sAnisoWrap :								   	register(s6, space1);
SamplerState sAnisoClamp :								  	register(s7, space1);
SamplerState sAnisoBorder :									register(s8, space1);
SamplerState sMaterialSampler :							 	register(s9, space1);
SamplerComparisonState sLinearClampComparisonGreater :		register(s10, space1);
SamplerComparisonState sLinearWrapComparisonGreater :		register(s11, space1);

InstanceData GetInstance(uint index)
{
    StructuredBuffer<InstanceData> instanceDatas = ResourceDescriptorHeap[cView.InstancesIndex];
    return instanceDatas[NonUniformResourceIndex(index)];
}

Material GetMaterial(uint index)
{
    StructuredBuffer<Material> materialDatas = ResourceDescriptorHeap[cView.MaterialsIndex];
    return materialDatas[NonUniformResourceIndex(index)];
}

MeshData GetMesh(uint index)
{
    StructuredBuffer<MeshData> meshDatas = ResourceDescriptorHeap[cView.MeshesIndex];
    return meshDatas[NonUniformResourceIndex(index)];
}

Light GetLight(uint index)
{
    StructuredBuffer<Light> lightDatas = ResourceDescriptorHeap[cView.LightsIndex];
    return lightDatas[NonUniformResourceIndex(index)];
}

struct Vertex
{
    float3 inPositionLS;
    float3 inNormalLS;
    float3 inTangentLS;
    float3 inBiTangentLS;
    float2 inTexCoords;
};

Vertex LoadVertex(MeshData meshData, uint vertexID)
{
    StructuredBuffer<Vertex> vertices = ResourceDescriptorHeap[NonUniformResourceIndex(meshData.VertexBufferIndex)];
    StructuredBuffer<unsigned int> indices = ResourceDescriptorHeap[NonUniformResourceIndex(meshData.IndexBufferIndex)];
    
    return vertices[indices[vertexID]];
}
