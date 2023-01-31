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
    uint4 directionalLightIndex; //Allows for 4 directional lights!
    uint4 pointLightIndex[4];
    uint nrOfDirectionalLights;
    uint nrOfPointLights;
    uint cameraIndex;
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

float4 ps_main(in PS_IN psIn) : SV_TARGET
{
    ConstantBuffer<Material> material                   = ResourceDescriptorHeap[perDrawData.materialIndex];
    ConstantBuffer<Camera> camera                       = ResourceDescriptorHeap[perFrameData.cameraIndex];

    psIn.inNormalWS = normalize(psIn.inNormalWS);
    float3 viewDir = normalize(camera.positionWS - psIn.inPositionWS);
    
    float3 ambientColor = material.color * ambientLight;
    
    float3 lightOut = float3(0.0f, 0.0f, 0.0f);
    for (uint i = 0u; i < perFrameData.nrOfDirectionalLights; i++)
    {
        ConstantBuffer<DirectionalLight> directionalLight = ResourceDescriptorHeap[perFrameData.directionalLightIndex[i]];
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
    
    for (uint j = 0u; j < perFrameData.nrOfPointLights; j++)
    {
        ConstantBuffer<PointLight> pointLight = ResourceDescriptorHeap[perFrameData.pointLightIndex[j / 4][j % 4]];
        
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