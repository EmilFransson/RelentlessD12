#pragma once
#include "Common.hlsli"

float DistributionGGX(float NoH, float roughness)
{
    const float alpha = roughness * roughness;
    const float alpha2 = alpha * alpha;
    const float NoH2 = NoH * NoH;
    const float b = (NoH2 * (alpha2 - 1.0f) + 1.0f);
    return alpha2 * RECIPROCAL_PI / max(b * b, EPSILON);
}

float GeometrySmith(float nDotV, float nDotL, float roughness)
{
    const float r = roughness + 1.0f;
    const float k = (r * r) / 8.0f;
    const float ggx1 = nDotV / (nDotV * (1.0 - k) + k);
    const float ggx2 = nDotL / (nDotL * (1.0 - k) + k);
    return ggx1 * ggx2;
}

float3 FresnelSchlick(float hDotV, float3 F0)
{
    return F0 + (1.0f - F0) * pow(1.0 - hDotV, 5.0f);
}

float FresnelSchlick90(float cosTheta, float F0, float F90)
{
    return F0 + (F90 - F0) * pow(1.0 - cosTheta, 5.0);
}

float3 FresnelSchlickRoughness(float HdotV, float3 F0, float roughness)
{
    return F0 + (max(float3(1.0f - roughness, 1.0f - roughness, 1.0f - roughness), F0) - F0) * pow(clamp(1.0f - HdotV, 0.0f, 1.0f), 5.0f);
}

float DisneyDiffuseFactor(float NoV, float NoL, float VoH, float roughness)
{
    const float alpha = roughness * roughness;
    const float F90 = 0.5 + 2.0 * alpha * VoH * VoH;
    const float F_in = FresnelSchlick90(NoL, 1.0, F90);
    const float F_out = FresnelSchlick90(NoV, 1.0, F90);
    return F_in * F_out;
}

float3 CalculateBRDF(const float3 toLightDirection, const float3 viewDirection, const float3 normal, const float metallic, const float roughness, const float3 albedo, const float3 baseReflectivity)
{
    const float3 halfwayVector = normalize(viewDirection + toLightDirection);

    const float NoV = clamp(dot(normal, viewDirection), 0.0f, 1.0f);
    const float NoL = clamp(dot(normal, toLightDirection), 0.0f, 1.0f);
    const float HoV = clamp(dot(halfwayVector, viewDirection), 0.0f, 1.0f);
    const float NoH = clamp(dot(normal, halfwayVector), 0.0f, 1.0f);

    const float3 F = FresnelSchlick(HoV, baseReflectivity);
    const float D = DistributionGGX(NoH, roughness);
    const float G = GeometrySmith(NoV, NoL, roughness);

    const float3 specular = (F * D * G) / (4.0f * max(NoV, EPSILON) * max(NoL, EPSILON));

    const float3 kS = F;
    const float3 kD = (1.0f - kS) * (1.0f - metallic);
    const float3 diffuse = albedo * kD * DisneyDiffuseFactor(NoV, NoL, HoV, roughness) * RECIPROCAL_PI;
    
    return diffuse + specular;
}