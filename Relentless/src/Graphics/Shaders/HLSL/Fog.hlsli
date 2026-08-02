#pragma once
#include "Common.hlsli"
#include "CommonBindings.hlsli"

struct FogResult
{
    float3 Inscatter;
    float Transmittance;
};

// (1 - exp(-x)) / x with a Taylor fallback around x == 0.
// This is UE's CalculateLineIntegralShared, converted from exp2 to exp:
// the log(2) factors in their Taylor terms disappear in base e.
float FogLineIntegral(float aHeightFalloff, float aRayDirectionY, float aRayOriginTerms)
{
    // Asymmetric clamp, mirroring UE's max(-127, ...) for exp2:
    // only the NEGATIVE side is dangerous (exp(+large) -> inf).
    // Large positive just underflows exp() to 0, which is harmless.
    // exp() overflows near +88, so clamp the argument at -87.
    const float falloff = max(-87.0f, aHeightFalloff * aRayDirectionY);

    const float lineIntegral = (1.0f - exp(-falloff)) / falloff;
    const float lineIntegralTaylor = 1.0f - 0.5f * falloff; // 1st-order expansion at 0

    return aRayOriginTerms * (abs(falloff) > 0.01f ? lineIntegral : lineIntegralTaylor);
}

// Optical depth through one exponential layer, UE-style:
// density at (possibly shifted) ray origin * slope-corrected ray length.
float ComputeLayerOpticalDepth(
    float3 aCameraPosition,
    float3 aCameraToReceiver, // UNnormalized, camera -> surface
    float aCameraToReceiverLength,
    float aDensity,
    float aHeightFalloff,
    float aHeightOffset,
    float aStartDistance,
    float aEndDistance)
{
    if (aDensity <= 0.0f)
        return 0.0f;
    
    if (aEndDistance > 0.0f && aCameraToReceiverLength > aEndDistance)
        return 0.0f;

    float rayLength = aCameraToReceiverLength;
    float rayDirectionY = aCameraToReceiver.y; // NOTE: full y-extent of ray, not normalized
    float rayOriginY = aCameraPosition.y;

    // StartDistance: restart the ray at the exclusion boundary (UE's exclude block).
    if (aStartDistance > 0.0f)
    {
        const float excludeTime = aStartDistance / aCameraToReceiverLength;
        if (excludeTime >= 1.0f)
            return 0.0f; // surface entirely inside fog-free zone

        const float cameraToIntersectionY = excludeTime * aCameraToReceiver.y;
        rayOriginY = aCameraPosition.y + cameraToIntersectionY;
        rayDirectionY = aCameraToReceiver.y - cameraToIntersectionY;
        rayLength = (1.0f - excludeTime) * aCameraToReceiverLength;
    }

    // Density at the ray origin. Same asymmetric clamp rationale as above.
    const float exponent = max(-87.0f, aHeightFalloff * (rayOriginY - aHeightOffset));
    const float rayOriginTerms = aDensity * exp(-exponent);

    return FogLineIntegral(aHeightFalloff, rayDirectionY, rayOriginTerms) * rayLength;
}

FogResult EvaluateExponentialHeightFog(float3 aWorldPos, float3 aCameraPosition, FogData aFogData)
{
    const float3 cameraToReceiver = aWorldPos - aCameraPosition;
    const float dist = max(length(cameraToReceiver), 0.0001f);

    float opticalDepth = 0.0f;

    opticalDepth += ComputeLayerOpticalDepth(
        aCameraPosition, cameraToReceiver, dist,
        aFogData.DensityLayer0, aFogData.HeightFallOffLayer0,
        aFogData.HeightOffsetLayer0, aFogData.StartDistanceLayer0, aFogData.EndDistanceLayer0);

    opticalDepth += ComputeLayerOpticalDepth(
        aCameraPosition, cameraToReceiver, dist,
        aFogData.DensityLayer1, aFogData.HeightFallOffLayer1,
        aFogData.HeightOffsetLayer1, aFogData.StartDistanceLayer1, aFogData.EndDistanceLayer1);

    FogResult result;
    result.Transmittance = saturate(exp(-opticalDepth));
    result.Transmittance = max(result.Transmittance, 1.0f - aFogData.MaxOpacity);
    
    #ifdef USE_INSCATTERING_TEXTURE
        TextureCube inscatterCube = ResourceDescriptorHeap[aFogData.InScatteringTextureIndex];
        const float directionality = saturate((dist - aFogData.NonDirectionalDistance) / max(aFogData.FullyDirectionalDistance - aFogData.NonDirectionalDistance, 0.0001f));
        const float mip = lerp(aFogData.InScatteringTextureMaxMip, 0.0f, directionality);
        const float3 viewDir = cameraToReceiver / dist;
        result.Inscatter = inscatterCube.SampleLevel(sLinearClamp, viewDir, mip).rgb * aFogData.InScatteringTextureColorTint * aFogData.InscatteringColorIntensity;
    #else
        result.Inscatter = aFogData.InScatteringColor * aFogData.InscatteringColorIntensity;
    #endif

    return result;
}