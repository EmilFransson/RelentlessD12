struct PS_IN
{
    float4 outPositionCS : SV_Position;
    float3 outPositionWS : POSITIONWS;
    float3 outNormalWS : NORMALWS;
    float3 outTangentWS : TANGENTWS;
    float3 outBiTangentWS : BITANGENTWS;
    float2 outTexCoords : TEXCOORDS;
};

float4 ps_main(PS_IN psIn) : SV_TARGET
{
    return float4(1.0f, 1.0f, 1.0f, 1.0f);
}