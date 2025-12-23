#pragma once
#include "Common.hlsli"

//Distance between rays is proportional to distance squared
//Extra windowing function to make light radius finite
//https://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
//float RadialAttenuation(float3 surfaceToLight, float range)
//{
//    const float distSq = dot(surfaceToLight, surfaceToLight);
//    const float distanceAttenuation = 1 / (distSq + 1);
//    const float windowing = Square(saturate(1 - Square(distSq * Square(rcp(range)))));
//    return distanceAttenuation * windowing;
//}

//float RadialAttenuation(float3 surfaceToLight, float range)
//{
//    const float distSq = dot(surfaceToLight, surfaceToLight);
//
//    // Prevent blow-up at extremely close distances
//    const float invDistSq = rcp(max(distSq, 1e-4f));
//
//    // Karis/Epic "smooth cutoff" windowing to make the radius finite
//    const float rangeSq = range * range;
//    const float x = distSq * rcp(rangeSq); // (d^2 / r^2)
//    const float windowing = Square(saturate(1.0f - Square(x)));
//
//    return invDistSq * windowing;
//}

float GetSquareFalloffAttenuation(float aDistanceSquare, float aFalloff)
{
    const float factor = aDistanceSquare * aFalloff;
    const float smoothFactor = saturate(1.0 - factor * factor);
    // We would normally divide by the square distance here
    // but we do it at the call site
    return smoothFactor * smoothFactor;
}

float RadialAttenuation(float3 aSurfaceToLight, float aFalloff)
{
    const float distanceSquare = dot(aSurfaceToLight, aSurfaceToLight);
    float attenuation = GetSquareFalloffAttenuation(distanceSquare, aFalloff);
    // light far attenuation
    //const float3 = getWorldPosition() - getWorldCameraPosition();
    //attenuation *= saturate(frameUniforms.lightFarAttenuationParams.x - dot(v, v) * frameUniforms.lightFarAttenuationParams.y);
    // Assume a punctual light occupies a volume of 1cm to avoid a division by 0
    return attenuation / max(distanceSquare, 1e-4);
}

// Angle >= Umbra -> 0
// Angle < Penumbra -> 1
//Gradient between Umbra and Penumbra
float DirectionalAttenuation(float3 L, float3 direction, float cosUmbra, float cosPenumbra)
{
    const float cosAngle = dot(-normalize(L), direction);
    const float falloff = saturate((cosAngle - cosUmbra) / (cosPenumbra - cosUmbra));
    return falloff * falloff;
}