#include "CommonBindings.hlsli"

struct PassParameters
{
	uint TargetIndex;
	uint w;
	uint h;
	uint pad0;
};

ConstantBuffer<PassParameters> passData : register(b1, space0);

float2 TexelToUV(uint2 texel, float2 texelSize)
{
	return ((float2)texel + 0.5f) * texelSize;
}

[numthreads(16, 16, 1)]
void cs_main(uint3 threadId : SV_DispatchThreadID)
{
    if (any(threadId.xy >= uint2(cView.ViewportDimensions)))
		return;

    RWTexture2D<float4> targetTexture = ResourceDescriptorHeap[passData.TargetIndex];
	float2 uv = TexelToUV(threadId.xy, cView.ViewportDimensionsInv);

	// Access elapsed time from the view's constant buffer (assuming cView provides it)
	float t = cView.ElapsedTime;
	
	// Create dynamic color channels using sine functions.
	float r = sin(uv.x * 10.0f + t) * 0.5f + 0.5f;
	float g = sin(uv.y * 10.0f + t) * 0.5f + 0.5f;
	float b = sin((uv.x + uv.y) * 5.0f + t) * 0.5f + 0.5f;

	float4 outCol = float4(r, g, b, 1.0f);
	targetTexture[threadId.xy] = outCol;
}