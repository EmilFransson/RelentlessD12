struct PS_IN
{
    float4 inPositionSS : SV_Position;
    float3 inPositionWS : POSITIONWS;
    float3 inColor      : COLOR;
};

float4 ps_main(in PS_IN psIn) : SV_TARGET
{
    return float4(psIn.inColor, 1.0f);
}