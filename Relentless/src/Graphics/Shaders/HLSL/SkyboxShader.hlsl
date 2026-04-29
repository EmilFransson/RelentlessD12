#include "CommonBindings.hlsli"

struct VS_OUT
{
    float4 PositionCS    : SV_POSITION;
    float3 TexCoord      : TEXCOORD0;
};

static const float3 cubeVertices[8] =
{
    float3(-1.0f, 1.0f, -1.0f),     // 0
    float3(1.0f, 1.0f, -1.0f),      // 1
    float3(-1.0f, -1.0f, -1.0f),    // 2
    float3(1.0f, -1.0f, -1.0f),     // 3
    float3(-1.0f, 1.0f, 1.0f),      // 4
    float3(1.0f, 1.0f, 1.0f),       // 5
    float3(-1.0f, -1.0f, 1.0f),     // 6
    float3(1.0f, -1.0f, 1.0f)       // 7
};

static const uint cubeIndices[36] =
{
    0, 1, 2, // side 1
    2, 1, 3,
    4, 0, 6, // side 2
    6, 0, 2,
    7, 5, 6, // side 3
    6, 5, 4,
    3, 1, 7, // side 4
    7, 1, 5,
    4, 5, 0, // side 5
    0, 5, 1,
    3, 7, 2, // side 6
    2, 7, 6,
};

VS_OUT vs_main(uint vertexID : SV_VertexID)
{
    VS_OUT vsOut = (VS_OUT)0;
    
    const float3 position = cubeVertices[cubeIndices[vertexID]];
    
    vsOut.PositionCS = mul(cView.ViewToClip, float4(position, 0.0f));
    vsOut.PositionCS.z = vsOut.PositionCS.w;
    
    const SkyboxData skyBox = GetSkyBox();
    
    const float3 localDir = normalize(position);
    const float3 worldDir = mul((float3x3)cView.ViewToWorld, localDir);
    vsOut.TexCoord = mul(worldDir, (float3x3)skyBox.WorldRotation);
    
    return vsOut;
}

float4 ps_main(VS_OUT psIn) : SV_Target
{
    const SkyboxData skyBoxData = GetSkyBox();
    
    TextureCube skyboxTextureA = ResourceDescriptorHeap[skyBoxData.EnvironmentMapAIndex];
    float4 color = skyboxTextureA.SampleLevel(sLinearWrap, psIn.TexCoord, skyBoxData.LODBias);
    
    #ifdef BLEND_ENVIRONMENTS
    TextureCube skyboxTextureB = ResourceDescriptorHeap[skyBoxData.EnvironmentMapBIndex];
    const float4 colorB = skyboxTextureB.SampleLevel(sLinearWrap, psIn.TexCoord, skyBoxData.LODBias);
    color = lerp(color, colorB, skyBoxData.BlendFactor);
    #endif
    
    return color * float4(skyBoxData.BackgroundColor, 1.0f) * skyBoxData.Intensity;
}