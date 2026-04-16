
struct PassData
{
    uint InputTextureIndex;
    uint OutputTextureIndex;
    uint SrcDimension;
    uint DstDimension;
};

ConstantBuffer<PassData> passData : register(b0, space0);

[numthreads(32, 32, 1)]
void cs_main(uint3 threadID : SV_DispatchThreadID)
{
    if (any(threadID >= uint3(passData.DstDimension, passData.DstDimension, 6)))
        return;
    
    Texture2DArray<float4> srcTexture = ResourceDescriptorHeap[passData.InputTextureIndex];
    RWTexture2DArray<float4> dstTexture = ResourceDescriptorHeap[passData.OutputTextureIndex];

    const uint2 dstCoord = threadID.xy;
    const uint face = threadID.z;
    const uint2 srcBase = dstCoord * 2;

    const uint2 size = uint2(passData.SrcDimension - 1, passData.SrcDimension - 1);
    const uint2 p0 = min(srcBase + uint2(0, 0), size);
    const uint2 p1 = min(srcBase + uint2(1, 0), size);
    const uint2 p2 = min(srcBase + uint2(0, 1), size);
    const uint2 p3 = min(srcBase + uint2(1, 1), size);

    const float4 a = srcTexture.Load(int4(p0, face, 0));
    const float4 b = srcTexture.Load(int4(p1, face, 0));
    const float4 c = srcTexture.Load(int4(p2, face, 0));
    const float4 d = srcTexture.Load(int4(p3, face, 0));

    dstTexture[uint3(dstCoord, face)] = 0.25f * (a + b + c + d);
}