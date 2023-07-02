SamplerState sampler_ANISOTROPIC : register(s0, space0);

struct PS_IN
{
	float4 inPositionSS	: SV_Position;
    float3 inPositionWS : POSITIONWS;
    float3 inNormalWS   : NORMALWS;
    float2 inTexCoords  : TEXCOORDS;
};

struct PerDrawData
{
    uint materialIndex;
    uint worldMatrixIndex;
};

struct Material
{
    float3 color;
    uint usesAlbedoTexture;
    uint albedoIndex;
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

ConstantBuffer<PerDrawData> perDrawData : register(b3, space0);
ConstantBuffer<PerFrameData> perFrameData : register(b4, space0);

static const float3 ambientLight = float3(0.065f, 0.065f, 0.065f);

static const uint USES_TEXTURE = 0xffffffff;

float4 ps_main(in PS_IN psIn) : SV_TARGET
{
    ConstantBuffer<Material> material                   = ResourceDescriptorHeap[perDrawData.materialIndex];
    ConstantBuffer<Camera> camera                       = ResourceDescriptorHeap[perFrameData.cameraMetaDataIndex];

    float4 albedoTextureColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
    if (material.usesAlbedoTexture == USES_TEXTURE)
    {
        Texture2D albedoTexture = ResourceDescriptorHeap[material.albedoIndex];
        albedoTextureColor = albedoTexture.Sample(sampler_ANISOTROPIC, float2(psIn.inTexCoords.x, psIn.inTexCoords.y));
    }

    psIn.inNormalWS = normalize(psIn.inNormalWS);
    const float3 viewDir = normalize(camera.positionWS - psIn.inPositionWS);
    
    const float3 ambientColor = material.color * ambientLight;
    
    float3 lightOut = float3(0.0f, 0.0f, 0.0f);
    StructuredBuffer<DirectionalLight> directionalLights = ResourceDescriptorHeap[perFrameData.directionalLightStructuredBufferIndex];
    for (uint i = 0u; i < perFrameData.nrOfDirectionalLights; i++)
    {
        DirectionalLight directionalLight = directionalLights[i];
        const float3 lightD = -normalize(directionalLight.direction);
        const float normalDotLightDir = dot(psIn.inNormalWS, lightD);
        
        const float3 diffuseColor = material.color * directionalLight.color * directionalLight.intensity;
        const float3 finalDiffuseColor = saturate(normalDotLightDir) * diffuseColor;
        
        if (normalDotLightDir > 0.0f)
        {
            const float3 halfWayDir = normalize(lightD + viewDir);
        
            const float specularFactor = pow(max(dot(psIn.inNormalWS, halfWayDir), 0.0f), 16.0f);
            const float3 specularColor = directionalLight.color * specularFactor * directionalLight.intensity * material.color;
        
            lightOut += (finalDiffuseColor + specularColor);
        }
        else
        {
            lightOut += finalDiffuseColor;
        }
    }
    
    StructuredBuffer<PointLight> pointLights = ResourceDescriptorHeap[perFrameData.pointLightStructuredBufferIndex];
    for (uint j = 0u; j < perFrameData.nrOfPointLights; j++)
    {
        PointLight pointLight = pointLights[j];
        
        const float3 surfaceToLightDirection = normalize(pointLight.position - psIn.inPositionWS);
        const float normalDotLightDir = dot(psIn.inNormalWS, surfaceToLightDirection);
        
        const float3 diffuseColor = material.color * pointLight.color * pointLight.intensity;
        const float3 finalDiffuseColor = saturate(normalDotLightDir) * diffuseColor;
    
        if (normalDotLightDir > 0.0f)
        {
            const float3 halfWayDir = normalize(surfaceToLightDirection + viewDir);
    
            const float specularFactor = pow(max(dot(psIn.inNormalWS, halfWayDir), 0.0f), 16.0f);
            const float3 specularColor = pointLight.color * specularFactor * pointLight.intensity * material.color;
        
            lightOut += (finalDiffuseColor + specularColor);
        }
        else
        {
            lightOut += finalDiffuseColor;
        }
    }

    return float4((lightOut + ambientColor) * albedoTextureColor.xyz, 1.0f);
}