float Lerp(float a, float b, float t)
{
    return (1.0f - t) * a + b * t;
}

float InverseLerp(float a, float b, float v)
{
    return (v - a) / (b - a);
}

float Remap(float iMin, float iMax, float oMin, float oMax, float v)
{
    float t = InverseLerp(iMin, iMax, v);
    return Lerp(oMin, oMax, t);
}

struct PS_IN
{
    float4 inPositionSS : SV_Position;
    float3 inPositionWS : POSITIONWS;
    float3 inColor      : COLOR;
};

struct PerFrameData
{
    uint cameraMetaDataIndex;
};

struct Camera
{
    float3 positionWS;
};

ConstantBuffer<PerFrameData> perFrameData : register(b1, space0);

static const float BOUNDARY_EPSILON = 0.01f;
static const float3 RED_GRID_COLOR = float3(0.63f, 0.0f, 0.0f);
static const float3 BLUE_GRID_COLOR = float3(0.0f, 0.0f, 0.63f);
static const float MAX_DISTANCE_TO_CAMERA = 10.0f;

float4 ps_main(in PS_IN psIn) : SV_TARGET
{
    ConstantBuffer<Camera> camera = ResourceDescriptorHeap[perFrameData.cameraMetaDataIndex];

    float distanceToCamera = length(psIn.inPositionWS - camera.positionWS);
    float t = distanceToCamera / MAX_DISTANCE_TO_CAMERA;
    float finalAlpha = saturate(Remap(MAX_DISTANCE_TO_CAMERA, 0.0f, 0.0f, 0.6f, t));

    /*This if-else clause checks whether the grid line to be rendered is at x = 0 or z = 0. These lines
      are colored red or blue to highlight the x-axis and z-axis in the world.*/
    float3 finalColor = 0.0f;
    if (psIn.inPositionWS.x > -BOUNDARY_EPSILON && psIn.inPositionWS.x < BOUNDARY_EPSILON)
    {
        finalColor = BLUE_GRID_COLOR;
    }
    else if (psIn.inPositionWS.z > -BOUNDARY_EPSILON && psIn.inPositionWS.z < BOUNDARY_EPSILON)
    {
        finalColor = RED_GRID_COLOR;
    }
    else
    {
        finalColor = float3(psIn.inColor);
    }
    return float4(finalColor, finalAlpha);
}