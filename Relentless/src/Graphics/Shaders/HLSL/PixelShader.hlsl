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
};

struct Material
{
    float3 color;
};

struct PerFrameData
{
    uint cameraMetaDataIndex;
    uint lightMetaDataIndex;
};

struct Camera
{
    float3 positionWS;
};

//Should probably make it so I have the lights in a structured buffer
//and just have the "nrOf..." in this meta data struct, to know the amount of content of
//the structured buffer(s). In this way I could keep the number of lights dynamic. =)
struct LightMetaData
{
    uint4 directionalLightIndex[16];
    uint4 pointLightIndex[128];
    uint nrOfDirectionalLights;
    uint nrOfPointLights;
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

float4 ps_main(in PS_IN psIn) : SV_TARGET
{
    ConstantBuffer<Material> material                   = ResourceDescriptorHeap[perDrawData.materialIndex];
    ConstantBuffer<Camera> camera                       = ResourceDescriptorHeap[perFrameData.cameraMetaDataIndex];
    ConstantBuffer<LightMetaData> lightData             = ResourceDescriptorHeap[perFrameData.lightMetaDataIndex];

    psIn.inNormalWS = normalize(psIn.inNormalWS);
    float3 viewDir = normalize(camera.positionWS - psIn.inPositionWS);
    
    float3 ambientColor = material.color * ambientLight;
    
    float3 lightOut = float3(0.0f, 0.0f, 0.0f);
    for (uint i = 0u; i < lightData.nrOfDirectionalLights; i++)
    {
        ConstantBuffer<DirectionalLight> directionalLight = ResourceDescriptorHeap[lightData.directionalLightIndex[i / 4][i % 4]];
        float3 lightD = -normalize(directionalLight.direction);
    
        float3 diffuseColor = material.color * directionalLight.color * directionalLight.intensity;
        float3 finalDiffuseColor = saturate(dot(psIn.inNormalWS, lightD)) * diffuseColor;
    
        if (dot(psIn.inNormalWS, lightD) > 0.0f)
        {
            float3 halfWayDir = normalize(lightD + viewDir);
    
            float specularFactor = pow(max(dot(psIn.inNormalWS, halfWayDir), 0.0f), 16.0f);
            float3 specularColor = directionalLight.color * specularFactor * directionalLight.intensity * material.color;
        
            lightOut += (finalDiffuseColor + specularColor);
        }
        else
        {
            lightOut += finalDiffuseColor;
        }
    }
    
    for (uint j = 0u; j < lightData.nrOfPointLights; j++)
    {
        ConstantBuffer<PointLight> pointLight = ResourceDescriptorHeap[lightData.pointLightIndex[j / 4][j % 4]];
        
        float3 surfaceToLightDirection = normalize(pointLight.position - psIn.inPositionWS);
        
        float3 diffuseColor = material.color * pointLight.color * pointLight.intensity;
        float3 finalDiffuseColor = saturate(dot(psIn.inNormalWS, surfaceToLightDirection)) * diffuseColor;
    
        if (dot(psIn.inNormalWS, surfaceToLightDirection) > 0.0f)
        {
            float3 halfWayDir = normalize(surfaceToLightDirection + viewDir);
    
            float specularFactor = pow(max(dot(psIn.inNormalWS, halfWayDir), 0.0f), 16.0f);
            float3 specularColor = pointLight.color * specularFactor * pointLight.intensity * material.color;
        
            lightOut += (finalDiffuseColor + specularColor);
        }
        else
        {
            lightOut += finalDiffuseColor;

        }
    }
        
    return float4(lightOut + ambientColor, 1.0f);
}