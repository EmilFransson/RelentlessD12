SamplerState sampler_ANISOTROPIC    : register(s0, space0);
SamplerState sampler_LINEAR         : register(s1, space0);

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
    float3 color;
    float metallic;
    float roughness;
    uint albedoIndex;
    uint metallicIndex;
    uint roughnessIndex;
    uint normalIndex;
};

struct PerFrameData
{
    uint cameraMetaDataIndex;
    uint pointLightStructuredBufferIndex;
    uint directionalLightStructuredBufferIndex;
    uint nrOfDirectionalLights;
    uint nrOfPointLights;
};

struct Camera
{
    float3 positionWS;
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

ConstantBuffer<PerDrawData> perDrawData     : register(b3, space0);
ConstantBuffer<PerFrameData> perFrameData   : register(b4, space0);

static const float3 ambientLight = float3(0.065f, 0.065f, 0.065f);

static const uint NO_USE = 0xFFFFFFFF;

static const float PI = 3.14159265359;
static const float EPSILON = 0.0000001f;

float DistributionGGX(float nDotH, float roughness)
{
    float r2 = roughness * roughness;
    float r22 = r2 * r2;
    float denominator = nDotH * nDotH * (r22 - 1.0f) + 1.0f;
    denominator = PI * denominator * denominator;
    return r22 / max(denominator, EPSILON);
}

float GeometrySmith(float nDotV, float nDotL, float roughness)
{
    float r = roughness + 1.0f;
    float k = (r * r) / 8.0f;
    float ggx1 = nDotV / (nDotV * (1.0 - k) + k);
    float ggx2 = nDotL / (nDotL * (1.0 - k) + k);
    return ggx1 * ggx2;
}

float3 FresnelSchlick(float hDotV, float3 baseReflectivity)
{
    return baseReflectivity + (1.0f - baseReflectivity) * pow(1.0 - hDotV, 5.0f);
}


float4 ps_main(PS_IN psIn) : SV_TARGET
{
    ConstantBuffer<Material> material = ResourceDescriptorHeap[perDrawData.materialIndex];
    ConstantBuffer<Camera> camera = ResourceDescriptorHeap[perFrameData.cameraMetaDataIndex];

    float4 albedoColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
    if (material.albedoIndex != NO_USE)
    {
        Texture2D albedoTexture = ResourceDescriptorHeap[material.albedoIndex];
        albedoColor = albedoTexture.Sample(sampler_ANISOTROPIC, float2(psIn.inTexCoords.x, psIn.inTexCoords.y));
    }
    else
    {
        albedoColor = float4(material.color, 1.0f);
    }

    float3 normal = normalize(psIn.inNormalWS);
    if (material.normalIndex != NO_USE)
    {
        Texture2D normalMap = ResourceDescriptorHeap[material.normalIndex];
        float3 sampledNormal = normalMap.Sample(sampler_LINEAR, float2(psIn.inTexCoords.x, psIn.inTexCoords.y)).rgb;
        sampledNormal = normalize(sampledNormal * 2.0f - 1.0f);
        sampledNormal.y = -sampledNormal.y;

        const float3 tangent = normalize(psIn.inTangentWS);
        const float3 biTangent = normalize(psIn.inBiTangentWS);
        float3x3 tbn = float3x3(tangent, biTangent, normal);
        normal = normalize(mul(sampledNormal, tbn).xyz);
    }

    const float3 viewDirection = normalize(camera.positionWS - psIn.inPositionWS);

    const float3 baseReflectivity = lerp(float3(0.04f, 0.04f, 0.04f), float3(albedoColor.xyz), material.metallic);

    float3 lightOut = float3(0.0f, 0.0f, 0.0f);

    StructuredBuffer<DirectionalLight> directionalLights = ResourceDescriptorHeap[perFrameData.directionalLightStructuredBufferIndex];
    for (uint i = 0u; i < perFrameData.nrOfDirectionalLights; i++)
    {
       DirectionalLight directionalLight = directionalLights[i];
       
       const float3 toLightDirection = normalize(-directionalLight.direction);
       const float3 halfwayVector = normalize(viewDirection + toLightDirection);
       const float3 radiance = directionalLight.color * directionalLight.intensity;

       const float nDotV = max(dot(normal, viewDirection), EPSILON);
       const float nDotL = max(dot(normal, toLightDirection), EPSILON);
       const float hDotV = max(dot(halfwayVector, viewDirection), 0.0f);
       const float nDotH = max(dot(normal, halfwayVector), 0.0f);

       float D = DistributionGGX(nDotH, material.roughness);
       float G = GeometrySmith(nDotV, nDotL, material.roughness);
       float3 F = FresnelSchlick(hDotV, baseReflectivity);

       float3 specular = D * G * F;
       specular /= 4.0f * nDotV * nDotL;

       float3 kD = float3(1.0f, 1.0f, 1.0f) - F;
       kD *= (1.0f - material.metallic);

       lightOut += (kD * float3(albedoColor.xyz) / PI + specular) * radiance * nDotL;
    }

    StructuredBuffer<PointLight> pointLights = ResourceDescriptorHeap[perFrameData.pointLightStructuredBufferIndex];
    for (uint j = 0u; j < perFrameData.nrOfPointLights; j++)
    {
        PointLight pointLight = pointLights[j];

        const float3 toLightDirection = normalize(pointLight.position - psIn.inPositionWS);
        const float3 halfwayVector = normalize(viewDirection + toLightDirection);
        const float distanceToLight = length(pointLight.position - psIn.inPositionWS);
        const float attenuation = 1.0f / (distanceToLight * distanceToLight);
        const float3 radiance = pointLight.color * pointLight.intensity * attenuation;

        const float nDotV = max(dot(normal, viewDirection), EPSILON);
        const float nDotL = max(dot(normal, toLightDirection), EPSILON);
        const float hDotV = max(dot(halfwayVector, viewDirection), 0.0f);
        const float nDotH = max(dot(normal, halfwayVector), 0.0f);

        float D = DistributionGGX(nDotH, material.roughness);
        float G = GeometrySmith(nDotV, nDotL, material.roughness);
        float3 F = FresnelSchlick(hDotV, baseReflectivity);

        float3 specular = D * G * F;
        specular /= 4.0f * nDotV * nDotL;

        float3 kD = float3(1.0f, 1.0f, 1.0f) - F;
        kD *= (1.0f - material.metallic);

        lightOut += (kD * float3(albedoColor.xyz) / PI + specular) * radiance * nDotL;
    }

    const float3 finalAmbientColor = float3(albedoColor.xyz) * ambientLight;
    const float3 outColor = finalAmbientColor + lightOut;

    return float4(outColor, 1.0f);
}