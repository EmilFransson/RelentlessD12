#ifndef COMMON_BINDINGS_HLSLI
#define COMMON_BINDINGS_HLSLI

#include "../Interop/ShaderInterop.h"

ConstantBuffer<ViewUniforms> cView : 						register(b2);

//Static samplers
SamplerState sLinearWrap :								  	register(s0, space1);
SamplerState sLinearClamp :								 	register(s1, space1);
SamplerState sLinearBorder :								register(s2, space1);
SamplerState sLinearWrapClamp :                             register(s3, space1);

SamplerState sPointWrap :								   	register(s4, space1);
SamplerState sPointClamp :								  	register(s5, space1);
SamplerState sPointBorder :								 	register(s6, space1);

SamplerState sAnisoWrap :								   	register(s7, space1);
SamplerState sAnisoClamp :								  	register(s8, space1);
SamplerState sAnisoBorder :									register(s9, space1);

SamplerState sMaterialSampler :							 	register(s10, space1);
SamplerComparisonState sLinearClampComparisonGreater :		register(s11, space1);
SamplerComparisonState sLinearWrapComparisonGreater :		register(s12, space1);

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

SkyLightData GetSkyLight()
{
    StructuredBuffer<SkyLightData> skyLightDatas = ResourceDescriptorHeap[cView.SkyLightIndex];
    return skyLightDatas[0];
}

SkyboxData GetSkyBox()
{
    StructuredBuffer<SkyboxData> skyBoxDatas = ResourceDescriptorHeap[cView.SkyBoxIndex];
    return skyBoxDatas[0];
}

struct Vertex
{
    float3 inPositionLS;
    float3 inNormalLS;
    float3 inTangentLS;
    float3 inBiTangentLS;
    float2 inTexCoords;
};

struct MaterialSurface
{
    float3 AlbedoColor;
    float3 EmissiveColor;
    float3 Normal;
    float Alpha;
    float Roughness;
    float Metalness;
    float AmbientOcclusion;
    float IOR;
};

struct MaterialEvaluationInputs
{
    float2 UV;
    float3 NormalWS;
    float3 TangentWS;
    float3 BitangentWS;
};

Vertex LoadVertex(MeshData meshData, uint vertexID)
{
    StructuredBuffer<Vertex> vertices = ResourceDescriptorHeap[NonUniformResourceIndex(meshData.VertexBufferIndex)];
    StructuredBuffer<unsigned int> indices = ResourceDescriptorHeap[NonUniformResourceIndex(meshData.IndexBufferIndex)];
    
    return vertices[indices[vertexID]];
}

float AdjustRoughnessForSpecularAA(float aRoughness, float3 aWorldNormal)
{
    // Estimate normal variance within pixel footprint
    const float3 dndu = ddx(aWorldNormal);
    const float3 dndv = ddy(aWorldNormal);
    
    // Variance of the normal distribution
    const float variance = dot(dndu, dndu) + dot(dndv, dndv);
    
    // Convert to roughness adjustment (in alpha = roughness^2 space)
    const float kernelRoughness2 = min(2.0f * variance, 0.18f); // clamp prevents over-blurring
    
    // Combine with existing roughness (add in alpha^2 space)
    const float adjustedAlpha = sqrt(aRoughness * aRoughness + kernelRoughness2);
    return adjustedAlpha;
}

float4 EvaluateAlbedo(Material aMaterial, float2 aUV)
{
    Texture2D albedoTexture = ResourceDescriptorHeap[aMaterial.AlbedoIndex];
    const float4 albedoColor = albedoTexture.Sample(sAnisoWrap, aUV) * aMaterial.BaseColorFactor;
    return albedoColor;
}

float EvaluateOpacity(Material aMaterial, float aAlbedoAlpha, float2 aUV)
{
    Texture2D opacityTexture = ResourceDescriptorHeap[aMaterial.OpacityIndex];
    const float opacity = opacityTexture.Sample(sAnisoWrap, aUV).r * aAlbedoAlpha;
    return opacity;
}

float EvaluateRoughness(Material aMaterial, float2 aUV)
{
    Texture2D roughnessTexture = ResourceDescriptorHeap[aMaterial.RoughnessIndex];
    float roughness = roughnessTexture.Sample(sAnisoWrap, aUV).r * aMaterial.RoughnessFactor;
    return saturate(roughness);
}

float EvaluateMetalness(Material aMaterial, float2 aUV)
{
    Texture2D metallicTexture = ResourceDescriptorHeap[aMaterial.MetalnessIndex];
    float metalness = metallicTexture.Sample(sAnisoWrap, aUV).r * aMaterial.MetalnessFactor;
    return saturate(metalness);
}

float EvaluateDisplacement(Material aMaterial, float2 aUV)
{
    Texture2D heightMap = ResourceDescriptorHeap[aMaterial.HeightMapIndex];
    const float displacement = heightMap.SampleLevel(sLinearWrap, aUV, 0).r;
    return displacement;
}

float EvaluateAmbientOcclusion(Material aMaterial, float2 aUV)
{
    Texture2D aoMap = ResourceDescriptorHeap[aMaterial.AOIndex];
    float ambientOcclusion = aoMap.Sample(sAnisoWrap, aUV).r;
    ambientOcclusion = lerp(1.0f, ambientOcclusion, aMaterial.AOFactor);
    return ambientOcclusion;
}

float3 EvaluateEmission(Material aMaterial, float2 aUV)
{
    Texture2D emissionTexture = ResourceDescriptorHeap[aMaterial.EmissiveIndex];
    const float3 emissionColor = emissionTexture.Sample(sAnisoWrap, aUV).xyz * aMaterial.EmissiveFactor.xyz * aMaterial.EmissionIntensity;
    return emissionColor;
}

float3 EvaluateNormal(Material aMaterial, MaterialEvaluationInputs aEvaluationInputs)
{
    float3 normal = normalize(aEvaluationInputs.NormalWS);
    const float3 tangent = normalize(aEvaluationInputs.TangentWS);
    const float3 biTangent = normalize(aEvaluationInputs.BitangentWS);
    const float3x3 tbn = float3x3(tangent, biTangent, normal);
    
    Texture2D normalMap = ResourceDescriptorHeap[aMaterial.NormalIndex];
    float3 sampledNormal = normalMap.Sample(sAnisoWrap, aEvaluationInputs.UV).rgb;
    sampledNormal = sampledNormal * 2.0f - 1.0f;
    
    normal = normalize(mul(sampledNormal, tbn).xyz);
    return normal;
}

float3 EvaluateRefraction(MaterialSurface aSurface, float aRefractionScale, float2 aScreenUV)
{
    // 1. Compute view-space normal (refraction works in view space)
    const float3 viewSpaceNormal = mul((float3x3)cView.WorldToView, aSurface.Normal);

    // 2. Compute refraction offset based on view-space normal XY
    // Normal pointing away from camera in XY direction = larger offset
    const float refractionStrength = (aSurface.IOR - 1.0f) * aRefractionScale;
    const float2 refractionOffset = viewSpaceNormal.xy * refractionStrength;

    // 4. Sample scene color at clamped offset UV
    float2 refractedUV = aScreenUV + refractionOffset;
    refractedUV = saturate(refractedUV);

    Texture2D sceneColorCopy = ResourceDescriptorHeap[cView.SceneColorCopyIndex];
    const float3 refractedBackground = sceneColorCopy.Sample(sLinearClamp, refractedUV).rgb;
    return refractedBackground;
}

MaterialSurface EvaluateMaterial(Material aMaterial, MaterialEvaluationInputs aEvaluationInputs)
{
    MaterialSurface surface = (MaterialSurface)0;
    
    const float4 albedo         = EvaluateAlbedo(aMaterial, aEvaluationInputs.UV);
    surface.AlbedoColor         = albedo.xyz;
    surface.Alpha               = EvaluateOpacity(aMaterial, albedo.w, aEvaluationInputs.UV);
    
    #ifdef ALPHA_MASK
        clip(surface.Alpha < aMaterial.AlphaCutOff ? -1 : 1);
    #endif
    
    surface.Roughness           = EvaluateRoughness(aMaterial, aEvaluationInputs.UV);
    surface.Metalness           = EvaluateMetalness(aMaterial, aEvaluationInputs.UV);
    surface.AmbientOcclusion    = EvaluateAmbientOcclusion(aMaterial, aEvaluationInputs.UV);
    surface.EmissiveColor       = EvaluateEmission(aMaterial, aEvaluationInputs.UV);
    surface.Normal              = EvaluateNormal(aMaterial, aEvaluationInputs);
    surface.Roughness           = AdjustRoughnessForSpecularAA(surface.Roughness, surface.Normal);
    surface.IOR                 = aMaterial.IOR;
    
    return surface;
}

#endif // COMMON_BINDINGS_HLSLI