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
    uint cameraIndex;
    uint directionalLightIndex;
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

ConstantBuffer<PerDrawData> perDrawData : register(b3, space0);
ConstantBuffer<PerFrameData> perFrameData : register(b4, space0);

static const float3 ambientLight = float3(0.065f, 0.065f, 0.065f);

float4 ps_main(in PS_IN psIn) : SV_TARGET
{
    ConstantBuffer<Material> material                   = ResourceDescriptorHeap[perDrawData.materialIndex];
    ConstantBuffer<Camera> camera                       = ResourceDescriptorHeap[perFrameData.cameraIndex];
    ConstantBuffer<DirectionalLight> directionalLight   = ResourceDescriptorHeap[perFrameData.directionalLightIndex];

    psIn.inNormalWS = normalize(psIn.inNormalWS);
    
    float3 ambientColor = material.color * ambientLight;
    
    float3 lightD = -directionalLight.direction;
    
    float3 diffuseColor = material.color * directionalLight.color * directionalLight.intensity;
    float3 finalDiffuseColor = saturate(dot(psIn.inNormalWS, lightD)) * diffuseColor;
    
    float3 viewDir = normalize(camera.positionWS - psIn.inPositionWS);
    float3 halfWayDir = normalize(lightD + viewDir);
    
    float specularFactor = pow(max(dot(psIn.inNormalWS, halfWayDir), 0.0f), 16.0f);
    float3 specularColor = directionalLight.color * specularFactor * directionalLight.intensity * material.color;
    
    return float4(saturate(finalDiffuseColor + specularColor + ambientColor), 1.0f);
}