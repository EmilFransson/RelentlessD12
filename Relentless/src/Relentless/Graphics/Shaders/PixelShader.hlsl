struct PS_IN
{
	float4 inPositionSS	: SV_Position;
};

float4 ps_main(in PS_IN psIn) : SV_TARGET
{
	return float4(1.0f, 0.0f, 0.0f, 1.0f);
}