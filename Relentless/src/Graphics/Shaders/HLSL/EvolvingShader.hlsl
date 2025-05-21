#include "BRDF.hlsli"
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

VS_OUT vs_main(uint vertexID : SV_VertexID)
{
    VS_OUT vsOut = (VS_OUT)0;
    
    InstanceData instanceData = GetInstance(perDrawData.InstanceIndex);
    MeshData meshData = GetMesh(instanceData.MeshDataIndex);
    Material material = GetMaterial(instanceData.MaterialIndex);
    Vertex vertex = LoadVertex(meshData, vertexID);

    float2 adjustedTexCoords = (vertex.inTexCoords * material.TilingFactor) + material.Offset;

    float3 positionLS = vertex.inPositionLS;
    if (material.HeightMapIndex != INVALID_DESCRIPTOR_INDEX)
    {
        Texture2D heightMap = ResourceDescriptorHeap[material.HeightMapIndex];
        const float height = heightMap.SampleLevel(sLinearWrap, adjustedTexCoords, 0).r;
        positionLS += vertex.inNormalLS * height * material.HeightFactor;
    }
    
    float4 worldPos = mul(instanceData.LocalToWorld, float4(positionLS, 1.0f));
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
    InstanceData instanceData = GetInstance(perDrawData.InstanceIndex);
    Material material = GetMaterial(instanceData.MaterialIndex);

    float2 adjustedUV = psIn.TexCoords;

    float4 albedoColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
    if (material.AlbedoIndex != INVALID_DESCRIPTOR_INDEX)
    {
        Texture2D albedoTexture = ResourceDescriptorHeap[material.AlbedoIndex];
        albedoColor = albedoTexture.Sample(sAnisoWrap, adjustedUV);
    }
    albedoColor *= material.BaseColorFactor;
    #ifdef ALPHA_MASK
        clip(albedoColor.a < 0.1f ? -1 : 1);
    #endif
    
    #ifdef ALPHA_BLEND
        albedoColor.a *= material.BaseColorFactor.a;
    #endif

    float metallic = material.MetalnessFactor;
    float roughness = material.RoughnessFactor;
    if (material.RoughnessMetalnessIndex != INVALID_DESCRIPTOR_INDEX)
    {
        Texture2D metallicTexture = ResourceDescriptorHeap[material.RoughnessMetalnessIndex];
        const float3 sampledTexel = metallicTexture.Sample(sAnisoWrap, adjustedUV).rgb;
        roughness = sampledTexel.g;
        metallic = sampledTexel.b;
    }
    else
    {
        if (material.MetalnessIndex != INVALID_DESCRIPTOR_INDEX)
        {
            Texture2D metallicTexture = ResourceDescriptorHeap[material.MetalnessIndex];
            metallic = metallicTexture.Sample(sAnisoWrap, adjustedUV).r;
        }
        if (material.RoughnessIndex != INVALID_DESCRIPTOR_INDEX)
        {
            Texture2D roughnessTexture = ResourceDescriptorHeap[material.RoughnessIndex];
            roughness = roughnessTexture.Sample(sAnisoWrap, adjustedUV).r;
        }
    }
    metallic = clamp(metallic, 0.0f, 1.0f);
    roughness = clamp(roughness, 0.0f, 1.0f);

    float3 normal = normalize(psIn.NormalWS);
    if (material.NormalIndex != INVALID_DESCRIPTOR_INDEX)
    {
        const float3 tangent = normalize(psIn.TangentWS);
        const float3 biTangent = normalize(psIn.BiTangentWS);
        float3x3 tbn = float3x3(tangent, biTangent, normal);
        
        Texture2D normalMap = ResourceDescriptorHeap[material.NormalIndex];
        float3 sampledNormal = normalMap.Sample(sAnisoWrap, adjustedUV).rgb;
        
        //sampledNormal.x = sampledNormal.x * 2.0f - 1.0f;
        //sampledNormal.y = -sampledNormal.y * 2.0f + 1.0f;
        //sampledNormal.z = sampledNormal.z;
        sampledNormal = sampledNormal * 2.0f - 1.0f;
        
        normal = normalize(mul(sampledNormal, tbn).xyz);
    }

    float ambientOcclusion = 1.0f;
    if (material.AOIndex != INVALID_DESCRIPTOR_INDEX)
    {
        Texture2D aoMap = ResourceDescriptorHeap[material.AOIndex];
        ambientOcclusion = aoMap.Sample(sAnisoWrap, adjustedUV).r;
        ambientOcclusion = lerp(1.0f, ambientOcclusion, material.AOFactor);
    }

    float4 emissionColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    if (material.EmissiveIndex != INVALID_DESCRIPTOR_INDEX)
    {
        Texture2D emissionTexture = ResourceDescriptorHeap[material.EmissiveIndex];
        emissionColor = emissionTexture.Sample(sAnisoWrap, adjustedUV) * material.EmissiveFactor;
    }
    else
    {
        emissionColor = material.EmissiveFactor;
    }
    emissionColor.rgb *= material.EmissionIntensity;

    const float3 viewDirection = normalize(cView.ViewLocation - psIn.PositionWS);
    const float3 baseReflectivity = lerp(float3(0.04f, 0.04f, 0.04f), float3(albedoColor.xyz), metallic);

    float3 lightOut = float3(0.0f, 0.0f, 0.0f);

    StructuredBuffer<Light> lights = ResourceDescriptorHeap[cView.LightsIndex];
    for (uint i = 0u; i < cView.LightCount; ++i)
    {
        const Light light = lights[i];
        
        if (!light.IsEnabled)
        {
            continue;
        }
        
        float3 toLightDirection = float3(0.0f, 0.0f, 0.0f);
        float attenuation = 1.0f;
        
        if (light.IsDirectional)
        {
            toLightDirection = normalize(-light.Direction);
        }
        else if (light.IsPoint || light.IsSpot)
        {
            const float3 surfaceToLight = light.Position - psIn.PositionWS;
            toLightDirection = normalize(surfaceToLight);
            attenuation = RadialAttenuation(surfaceToLight, light.Range);
            
            if (light.IsSpot)
            {
                attenuation *= DirectionalAttenuation(surfaceToLight, light.Direction, light.SpotlightAngles.y, light.SpotlightAngles.x);
            }
        }
        
        const float NoL = dot(normal, toLightDirection);
        if (NoL > 0.0f && attenuation > 0.0f)
        {
            const float3 lightRadiance = light.Color * light.Intensity * attenuation;
            const float3 brdf = CalculateBRDF(toLightDirection, viewDirection, normal, metallic, roughness, albedoColor.xyz, baseReflectivity);
            lightOut += brdf * lightRadiance * NoL;
        }
    }

    const float NoV = clamp(dot(normal, viewDirection), 0.0f, 1.0f);
    const float3 F = FresnelSchlickRoughness(NoV, baseReflectivity, roughness);
    const float3 kD = (1.0 - F) * (1.0 - metallic);
    
    float3 diffuse = float3(0.0f, 0.0f, 0.0f);
    float3 specular = float3(0.0f, 0.0f, 0.0f);
    float3 backgroundColor = float3(0.0f, 0.0f, 0.0f);
    
    if (cView.IrradianceMapIndex != INVALID_DESCRIPTOR_INDEX)
    {
        TextureCube irradianceMap = ResourceDescriptorHeap[cView.IrradianceMapIndex];
        diffuse = irradianceMap.Sample(sLinearWrap, normal).rgb * albedoColor.xyz * kD;
    }
    else if (cView.EnvironmentIndex != INVALID_DESCRIPTOR_INDEX)
    {
        StructuredBuffer<Environment> environment = ResourceDescriptorHeap[cView.EnvironmentIndex];
        backgroundColor = environment[0].BackgroundColor;
        diffuse = backgroundColor * albedoColor.xyz * kD;
    }
    
    float3 prefilteredColor = float3(0.0f, 0.0f, 0.0f);
    if (cView.RadianceMapIndex != INVALID_DESCRIPTOR_INDEX)
    {
        TextureCube radianceMap = ResourceDescriptorHeap[cView.RadianceMapIndex];
        static const float MAX_REFLECTION_LOD = 4.0f;
        prefilteredColor = radianceMap.SampleLevel(sLinearWrap, reflect(-viewDirection, normal), roughness * MAX_REFLECTION_LOD).rgb;
    }
    else
    {
        prefilteredColor = backgroundColor;
    }
    
    if (cView.BRDFfLutTextureIndex != INVALID_DESCRIPTOR_INDEX)
    {
        Texture2D brdfLutTexture = ResourceDescriptorHeap[cView.BRDFfLutTextureIndex];
        const float2 brdf = brdfLutTexture.Sample(sLinearClamp, float2(NoV, 1.0f - roughness)).rg;
        specular = prefilteredColor * (F * brdf.r + brdf.g);
    }

    const float3 finalAmbientColor = ((diffuse + specular) * ambientOcclusion) + emissionColor.rgb;
   
    const float3 outColor = finalAmbientColor + lightOut;
    return float4(outColor, albedoColor.a);
}