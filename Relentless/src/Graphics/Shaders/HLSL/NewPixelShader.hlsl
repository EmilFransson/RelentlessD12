#include "CommonBindings.hlsli"

struct PS_IN
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORDS;
};

float4 ps_main(PS_IN psIn) : SV_TARGET
{
    return float4(psIn.uv, 0.0f, 1.0f);
}