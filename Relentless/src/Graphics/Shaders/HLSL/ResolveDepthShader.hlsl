//ResolveDepthShader
struct PassData
{
    uint SourceIndex;
    uint TargetIndex;
    uint2 TextureSize;
};

ConstantBuffer<PassData> passData : register(b1, space0);

[numthreads(8, 8, 1)]
void cs_main(uint3 aDispatchID : SV_DispatchThreadID)
{
    const uint2 texel = aDispatchID.xy;
    if (any(texel >= passData.TextureSize))
        return;
    
    Texture2DMS<float> msaaDepth = ResourceDescriptorHeap[passData.SourceIndex];
    RWTexture2D<float> resolvedDepth = ResourceDescriptorHeap[passData.TargetIndex];
    
    float minDepth = 1.0f;
    
    [unroll]
    for (int sample = 0; sample < 8; ++sample)
    {
        minDepth = min(minDepth, msaaDepth.Load(texel, sample));
    }
    
    resolvedDepth[texel] = minDepth;
}