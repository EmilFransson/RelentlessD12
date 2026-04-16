#ifndef COMMON_HLSLI
#define COMMON_HLSLI

template<typename T>
T Lerp(T a, T b, T t)
{
    return lerp(a, b, t);
}

template<typename T>
T InverseLerp(T a, T b, T v)
{
    return (v - a) / (b - a);
}

template<typename T>
T Remap(T iMin, T iMax, T oMin, T oMax, T v)
{
    T t = InverseLerp(iMin, iMax, v);
    return Lerp(oMin, oMax, t);
}

template<typename T>
T Square(T x)
{
    return x * x;
}

float3 LinearToSRGB(float3 color)
{
    return pow(color, 1.0f / 2.2f);
}

float2 TexelToUV(uint2 texel, float2 texelSize)
{
    return ((float2) texel + 0.5f) * texelSize;
}

float GetLuminance(float3 color)
{
    return dot(color, float3(0.2126729, 0.7151522, 0.0721750));
}

static const float PI = 3.14159265359f;
static const float TwoPI = 2.0f * PI;
static const float RECIPROCAL_PI = 0.3183098861837907f;
static const float EPSILON = 0.0000001f;

#endif // COMMON_HLSLI