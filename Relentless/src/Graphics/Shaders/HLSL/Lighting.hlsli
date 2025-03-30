#pragma once
#include "Common.hlsli"

//Distance between rays is proportional to distance squared
//Extra windowing function to make light radius finite
//https://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
float RadialAttenuation(float3 surfaceToLight, float range)
{
    const float distSq = dot(surfaceToLight, surfaceToLight);
    const float distanceAttenuation = 1 / (distSq + 1);
    const float windowing = Square(saturate(1 - Square(distSq * Square(rcp(range)))));
    return distanceAttenuation * windowing;
}