#ifndef ENVIRONMENT_MAPPING_HLSLI
#define ENVIRONMENT_MAPPING_HLSLI
#include "Common.hlsli"

static const uint CUBEMAP_FACE_POS_X = 0;
static const uint CUBEMAP_FACE_NEG_X = 1;
static const uint CUBEMAP_FACE_POS_Y = 2;
static const uint CUBEMAP_FACE_NEG_Y = 3;
static const uint CUBEMAP_FACE_POS_Z = 4;
static const uint CUBEMAP_FACE_NEG_Z = 5;

float3 GetCubeMapTexCoord(uint3 aDispatchThreadID, uint2 aFaceSize)
{
    const float2 st = (float2(aDispatchThreadID.xy) + 0.5f) / float2(aFaceSize);
    const float2 uv = 2.0f * float2(st.x, 1.0f - st.y) - 1.0f;

    float3 dir;

    switch (aDispatchThreadID.z)
    {
        case CUBEMAP_FACE_POS_X:
            dir = float3(1.0f, uv.y, -uv.x);
            break; // +X
        case CUBEMAP_FACE_NEG_X:
            dir = float3(-1.0f, uv.y, uv.x);
            break; // -X
        case CUBEMAP_FACE_POS_Y:
            dir = float3(uv.x, 1.0f, -uv.y);
            break; // +Y
        case CUBEMAP_FACE_NEG_Y:
            dir = float3(uv.x, -1.0f, uv.y);
            break; // -Y
        case CUBEMAP_FACE_POS_Z:
            dir = float3(uv.x, uv.y, 1.0f);
            break; // +Z
        default:
            dir = float3(-uv.x, uv.y, -1.0f);
            break; // -Z
    }

    return normalize(dir);
}

// Compute orthonormal basis for converting from tanget/shading space to world space.
void ComputeBasisVectors(const float3 N, out float3 S, out float3 T)
{
	// Branchless select non-degenerate T.
    T = cross(N, float3(0.0, 1.0, 0.0));
    T = lerp(cross(N, float3(1.0, 0.0, 0.0)), T, step(EPSILON, dot(T, T)));

    T = normalize(T);
    S = normalize(cross(N, T));
}

// ---------------------------------------------------------------------------------------------------
// The following code (from Unreal Engine 4's paper) shows how to filter the environment map
// for different roughnesses. This is meant to be computed offline and stored in cube map mips,
// so turning this on online will cause poor performance
// Compute Van der Corput radical inverse
// See: http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
float RadicalInverse_VdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

// Sample i-th point from Hammersley point set of NumSamples points total.
float2 SampleHammersley(uint i, uint samples)
{
    const float invSamples = 1.0 / float(samples);
    return float2(i * invSamples, RadicalInverse_VdC(i));
}

// Convert point from tangent/shading space to world space.
float3 TangentToWorld(const float3 v, const float3 N, const float3 S, const float3 T)
{
    return S * v.x + T * v.y + N * v.z;
}

// Uniformly sample point on a hemisphere.
// Cosine-weighted sampling would be a better fit for Lambertian BRDF but since this
// compute shader runs only once as a pre-processing step performance is not *that* important.
// See: "Physically Based Rendering" 2nd ed., section 13.6.1.
float3 SampleHemisphere(float u1, float u2)
{
    const float u1p = sqrt(max(0.0f, 1.0f - u1 * u1));
    return float3(cos(TwoPI * u2) * u1p, sin(TwoPI * u2) * u1p, u1);
}

// Importance sample GGX normal distribution function for a fixed roughness value.
// This returns normalized half-vector between Li & Lo.
// For derivation see: http://blog.tobias-franke.eu/2014/03/30/notes_on_importance_sampling.html
float3 SampleGGX(float u1, float u2, float roughness)
{
    const float alpha = roughness * roughness;

    const float cosTheta = sqrt((1.0f - u2) / (1.0f + (alpha * alpha - 1.0f) * u2));
    const float sinTheta = sqrt(1.0f - cosTheta * cosTheta); // Trig. identity
    const float phi = TwoPI * u1;

	// Convert to Cartesian upon return.
    return float3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);
}

// GGX/Towbridge-Reitz normal distribution function.
// Uses Disney's reparametrization of alpha = roughness^2
float NdfGGX(float cosLh, float roughness)
{
    const float alpha = roughness * roughness;
    const float alphaSq = alpha * alpha;

    const float denom = (cosLh * cosLh) * (alphaSq - 1.0f) + 1.0f;
    return alphaSq / (PI * denom * denom);
}

#endif //ENVIRONMENT_MAPPING_HLSLI