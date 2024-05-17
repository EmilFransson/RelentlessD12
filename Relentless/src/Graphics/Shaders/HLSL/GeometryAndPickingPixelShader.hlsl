SamplerState sampler_ANISOTROPIC : register(s0, space0);
SamplerState sampler_LINEAR         : register(s1, space0);
SamplerState sampler_LINEAR_CLAMP   : register(s2, space0);

struct PS_IN
{
    float4 inPositionSS	    : SV_Position;
    float3 inPositionWS     : POSITIONWS;
    float3 inNormalWS       : NORMALWS;
    float3 inTangentWS      : TANGENTWS;
    float3 inBiTangentWS    : BITANGENTWS;
    float2 inTexCoords      : TEXCOORDS;
};

struct PerDrawData
{
    uint materialIndex;
    uint worldMatrixIndex;
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

struct PerFrameData
{
    uint cameraMetaDataIndex;
    uint pointLightStructuredBufferIndex;
    uint directionalLightStructuredBufferIndex;
    uint nrOfDirectionalLights;
    uint nrOfPointLights;
    uint environmentIndex;
    uint brdfLutTextureIndex;
};

struct Camera
{
    float3 positionWS;
};

struct Environment
{
    float3 backgroundColor;
};

struct DirectionalLight
{
    float3 direction;
    float intensity;
    float3 color;
};

struct PointLight
{
    float3 position;
    float intensity;
    float3 color;
};

struct Identity
{
    uint ID;
};

ConstantBuffer<PerDrawData> perDrawData     : register(b3, space0);
ConstantBuffer<PerFrameData> perFrameData   : register(b4, space0);
ConstantBuffer<Identity> Identifier         : register(b5, space0);

static const uint NO_USE = 0xffffffff;

static const float PI = 3.14159265359f;
static const float RECIPROCAL_PI = 0.3183098861837907f;
static const float EPSILON = 0.0000001f;

float DistributionGGX(float NoH, float roughness)
{
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;
    float NoH2 = NoH * NoH;
    float b = (NoH2 * (alpha2 - 1.0f) + 1.0f);
    return alpha2 * RECIPROCAL_PI / max(b * b, EPSILON);
}

float GeometrySmith(float nDotV, float nDotL, float roughness)
{
    float r = roughness + 1.0f;
    float k = (r * r) / 8.0f;
    float ggx1 = nDotV / (nDotV * (1.0 - k) + k);
    float ggx2 = nDotL / (nDotL * (1.0 - k) + k);
    return ggx1 * ggx2;
}

float3 FresnelSchlick(float hDotV, float3 F0)
{
    return F0 + (1.0f - F0) * pow(1.0 - hDotV, 5.0f);
}

float FresnelSchlick90(float cosTheta, float F0, float F90) {
    return F0 + (F90 - F0) * pow(1.0 - cosTheta, 5.0);
}

float3 FresnelSchlickRoughness(float HdotV, float3 F0, float roughness)
{
    return F0 + (max(float3(1.0f - roughness, 1.0f - roughness, 1.0f - roughness), F0) - F0) * pow(clamp(1.0f - HdotV, 0.0f, 1.0f), 5.0f);
}

float DisneyDiffuseFactor(float NoV, float NoL, float VoH, float roughness) {
    float alpha = roughness * roughness;
    float F90 = 0.5 + 2.0 * alpha * VoH * VoH;
    float F_in = FresnelSchlick90(NoL, 1.0, F90);
    float F_out = FresnelSchlick90(NoV, 1.0, F90);
    return F_in * F_out;
}

float3 CalculateBRDF(const float3 toLightDirection, const float3 viewDirection, const float3 normal,
    const float metallic, const float roughness, const float3 albedo, const float3 baseReflectivity)
{
    const float3 halfwayVector = normalize(viewDirection + toLightDirection);

    const float NoV = clamp(dot(normal, viewDirection), 0.0f, 1.0f);
    const float NoL = clamp(dot(normal, toLightDirection), 0.0f, 1.0f);
    const float HoV = clamp(dot(halfwayVector, viewDirection), 0.0f, 1.0f);
    const float NoH = clamp(dot(normal, halfwayVector), 0.0f, 1.0f);

    const float3 F = FresnelSchlick(HoV, baseReflectivity);
    const float D = DistributionGGX(NoH, roughness);
    const float G = GeometrySmith(NoV, NoL, roughness);

    const float3 specular = (F * D * G) / (4.0f * max(NoV, EPSILON) * max(NoL, EPSILON));

    float3 kD = albedo;
    kD *= float3(1.0f, 1.0f, 1.0f) - F;
    kD *= DisneyDiffuseFactor(NoV, NoL, HoV, roughness);
    kD *= (1.0f - metallic);

    float3 diffuse = kD * RECIPROCAL_PI;

    return diffuse + specular;
}

struct PS_OUT
{
    float4 finalColor   : SV_TARGET0;
    uint ID             : SV_TARGET1;
};

PS_OUT ps_main(in PS_IN psIn)
{
    PS_OUT psOut = (PS_OUT) 0;
    
    ConstantBuffer<Material> material = ResourceDescriptorHeap[perDrawData.materialIndex];
    ConstantBuffer<Camera> camera = ResourceDescriptorHeap[perFrameData.cameraMetaDataIndex];

    float2 adjustedUV = psIn.inTexCoords;

    float4 albedoColor = material.color;
    if (material.albedoIndex != NO_USE)
    {
        Texture2D albedoTexture = ResourceDescriptorHeap[material.albedoIndex];
        albedoColor = albedoTexture.Sample(sampler_ANISOTROPIC, adjustedUV);
        albedoColor.rgb = pow(albedoColor.rgb, 2.2);
    }

    float metallic = material.metallic;
    float roughness = material.roughness;
    if (material.combinedRoughnessMetalnessMap == 1)
    {
        Texture2D metallicTexture = ResourceDescriptorHeap[material.metallicIndex];
        float3 sampledTexel = metallicTexture.Sample(sampler_ANISOTROPIC, adjustedUV).rgb;
        roughness = sampledTexel.g;
        metallic = sampledTexel.b;
    }
    else
    {
        if (material.metallicIndex != NO_USE)
        {
            Texture2D metallicTexture = ResourceDescriptorHeap[material.metallicIndex];
            metallic = metallicTexture.Sample(sampler_ANISOTROPIC, adjustedUV).r;
        }
        if (material.roughnessIndex != NO_USE)
        {
            Texture2D roughnessTexture = ResourceDescriptorHeap[material.roughnessIndex];
            roughness = roughnessTexture.Sample(sampler_ANISOTROPIC, adjustedUV).r;
        }
    }
    metallic = clamp(metallic, 0.0f, 1.0f);
    roughness = clamp(roughness, 0.0f, 1.0f);


    float3 normal = normalize(psIn.inNormalWS);
    if (material.normalIndex != NO_USE)
    {
        const float3 tangent = normalize(psIn.inTangentWS);
        const float3 biTangent = normalize(psIn.inBiTangentWS);
        float3x3 tbn = float3x3(tangent, biTangent, normal);
        
        Texture2D normalMap = ResourceDescriptorHeap[material.normalIndex];
        float3 sampledNormal = normalMap.Sample(sampler_ANISOTROPIC, adjustedUV).rgb;
        
        //sampledNormal = normalize(sampledNormal * 2.0f - 1.0f);
        //sampledNormal.y = -sampledNormal.y;
        sampledNormal.x = sampledNormal.x * 2.0f - 1.0f;
        sampledNormal.y = -sampledNormal.y * 2.0f + 1.0f;
        sampledNormal.z = sampledNormal.z;
        
       
        normal = normalize(mul(sampledNormal, tbn).xyz);
    }

    float ambientOcclusion = 1.0f;
    if (material.ambientOcclusionIndex != NO_USE)
    {
        Texture2D aoMap = ResourceDescriptorHeap[material.ambientOcclusionIndex];
        ambientOcclusion = aoMap.Sample(sampler_ANISOTROPIC, adjustedUV).r;
        ambientOcclusion = lerp(1.0f, ambientOcclusion, material.aoScale);
    }

    float4 emissionColor = material.emissionColor;
    if (material.emissionIndex != NO_USE)
    {
        Texture2D emissionTexture = ResourceDescriptorHeap[material.emissionIndex];
        emissionColor = emissionTexture.Sample(sampler_ANISOTROPIC, adjustedUV);
    }
    emissionColor.rgb *= material.emissionIntensity;

    const float3 viewDirection = normalize(camera.positionWS - psIn.inPositionWS);

    const float3 baseReflectivity = lerp(float3(0.04f, 0.04f, 0.04f), float3(albedoColor.xyz), metallic);

    float3 lightOut = float3(0.0f, 0.0f, 0.0f);

    StructuredBuffer<DirectionalLight> directionalLights = ResourceDescriptorHeap[perFrameData.directionalLightStructuredBufferIndex];
    for (uint i = 0u; i < perFrameData.nrOfDirectionalLights; i++)
    {
       DirectionalLight directionalLight = directionalLights[i];
       
       float3 toLightDirection = normalize(-directionalLight.direction);
       if (dot(toLightDirection, normal) > 0.0f)
       {
           float3 brdf = CalculateBRDF(toLightDirection, viewDirection, normal, metallic, roughness, albedoColor.xyz, baseReflectivity);
           const float3 lightRadiance = directionalLight.color * directionalLight.intensity;
           float3 NoL = max(dot(normal, toLightDirection), EPSILON);
           lightOut += brdf * lightRadiance * NoL;
       }
    }

    StructuredBuffer<PointLight> pointLights = ResourceDescriptorHeap[perFrameData.pointLightStructuredBufferIndex];
    for (uint j = 0u; j < perFrameData.nrOfPointLights; j++)
    {
        PointLight pointLight = pointLights[j];

        const float3 toLightDirection = normalize(pointLight.position - psIn.inPositionWS);
        if (dot(toLightDirection, normal) > 0.0f)
        {
            const float3 brdf = CalculateBRDF(toLightDirection, viewDirection, normal, metallic, roughness, albedoColor.xyz, baseReflectivity);
            
            const float distanceToLight = length(pointLight.position - psIn.inPositionWS);
            const float attenuation = 1.0f / (distanceToLight * distanceToLight);
            const float3 lightRadiance = pointLight.color * pointLight.intensity * attenuation;

            const float3 NoL = max(dot(normal, toLightDirection), EPSILON);
            lightOut += brdf * lightRadiance * NoL;
        }
    }

    float NoV = clamp(dot(normal, viewDirection), 0.0f, 1.0f);
    float3 F = FresnelSchlickRoughness(NoV, baseReflectivity, roughness);
    float3 kD = (1.0 - F) * (1.0 - metallic);
    
    // With a solid color, no need for a texture lookup, just use the color
    ConstantBuffer<Environment> environment = ResourceDescriptorHeap[perFrameData.environmentIndex];

    float3 diffuse = environment.backgroundColor * albedoColor.xyz * kD;
    
    float3 prefilteredColor = environment.backgroundColor;
    
    Texture2D brdfLutTexture = ResourceDescriptorHeap[perFrameData.brdfLutTextureIndex];
    float u = NoV;
    float v = 1.0f - roughness;
    
    float2 brdf2 = brdfLutTexture.Sample(sampler_LINEAR_CLAMP, float2(u,v)).xy;
    //brdf2 = pow(brdf2, 2.2);
    float2 brdf = brdf2.xy;
    float3 specular = prefilteredColor * (F * brdf.r + brdf.g);
   // return float4(specular, 1.0f);

    float3 finalAmbientColor = ((diffuse + specular) * ambientOcclusion) + emissionColor.rgb; // ao is from ambient occlusion map
   
    //const float3 finalAmbientColor = (float3(albedoColor.xyz) * ambientLight * ambientOcclusion) + emissionColor;
    
    float3 outColor = finalAmbientColor + lightOut;
    
    psOut.finalColor = float4(outColor, 1.0f);
    psOut.ID = Identifier.ID;
    return psOut;
}