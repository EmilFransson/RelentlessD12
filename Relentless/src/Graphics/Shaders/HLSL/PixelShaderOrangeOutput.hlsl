struct PS_IN
{
	float4 inPositionSS	    : SV_Position;
    float3 inPositionWS     : POSITIONWS;
    float3 inNormalWS       : NORMALWS;
    float3 inTangentWS      : TANGENTWS;
    float3 inBiTangentWS    : BITANGENTWS;
    float2 inTexCoords      : TEXCOORDS;
};

float4 ps_main(in PS_IN psIn) : SV_TARGET
{
    return float4(210.0f / 255.0f, 40.0 / 255.0f, 0.0f, 1.0f);
}