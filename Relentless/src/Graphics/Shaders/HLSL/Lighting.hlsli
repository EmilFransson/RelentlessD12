#pragma once
#include "Common.hlsli"
#include "CommonBindings.hlsli"
#include "BRDF.hlsli"

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

int GetCascadeIndex(Light aLight, float aViewDepth)
{
    const float4 mask = aViewDepth > aLight.CascadeSplits;
    return min(dot(mask, float4(1, 1, 1, 1)), aLight.ShadowViewCount - 1);
}

uint GetShadowMapIndex(Light aLight, float aViewDepth)
{
    if (aLight.IsDirectional)
    {
        const int cascade = GetCascadeIndex(aLight, aViewDepth);
        const ShadowViewData shadowView = GetShadowView(aLight.ShadowViewBaseIndex + cascade);
        return shadowView.ShadowMapIndex;
    }
    
    return 0u;
}

float GetSquareFalloffAttenuation(float aDistanceSquare, float aFalloff)
{
    const float factor = aDistanceSquare * aFalloff;
    const float smoothFactor = saturate(1.0 - factor * factor);
    // We would normally divide by the square distance here but we do it at the call site
    return smoothFactor * smoothFactor;
}

float RadialAttenuation(float3 aSurfaceToLight, float aFalloff)
{
    const float distanceSquare = dot(aSurfaceToLight, aSurfaceToLight);
    float attenuation = GetSquareFalloffAttenuation(distanceSquare, aFalloff);
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

float ShadowNoPCF(float3 aWorldPosition, uint aShadowViewIndex)
{
    const ShadowViewData shadowView = GetShadowView(aShadowViewIndex);
    
    float4 lightPos = mul(shadowView.WorldToClip, float4(aWorldPosition, 1));
    lightPos.xyz /= lightPos.w;
    const float2 uv = ClipToUV(lightPos.xy);

    Texture2D shadowTexture = ResourceDescriptorHeap[shadowView.ShadowMapIndex];
    return shadowTexture.SampleCmpLevelZero(sLinearClampComparisonGreater, uv, lightPos.z + 0.1f);
}

float Shadow3x3PCF(float3 aWorldPosition, float3 aGeometricNormal, float3 aToLight, uint aShadowViewIndex)
{
    const ShadowViewData shadowView = GetShadowView(aShadowViewIndex);
    
    const float NdotL = saturate(dot(aGeometricNormal, aToLight));
    float slope = sqrt(1.0f - NdotL * NdotL) / max(NdotL, 1e-4f);
    slope = min(slope, 4.0f);
    const float biasWS = shadowView.ConstantBiasWS + shadowView.SlopeBiasWS * slope;
    const float3 biasedPosition = aWorldPosition + aToLight * biasWS;
    
    float4 lightPos = mul(shadowView.WorldToClip, float4(biasedPosition, 1.0f));
    lightPos.xyz /= lightPos.w;
    const float2 uv = ClipToUV(lightPos.xy);
    Texture2D shadowTexture = ResourceDescriptorHeap[shadowView.ShadowMapIndex];

    const float dilation = 2.0f;
    const float d1 = dilation * shadowView.InverseShadowMapSize * 0.125f;
    const float d2 = dilation * shadowView.InverseShadowMapSize * 0.875f;
    const float d3 = dilation * shadowView.InverseShadowMapSize * 0.625f;
    const float d4 = dilation * shadowView.InverseShadowMapSize * 0.375f;
    const float result = (
		2.0f * shadowTexture.SampleCmpLevelZero(sLinearClampComparisonGreater, uv, lightPos.z) +
		shadowTexture.SampleCmpLevelZero(sLinearClampComparisonGreater, uv + float2(-d2, d1), lightPos.z) +
		shadowTexture.SampleCmpLevelZero(sLinearClampComparisonGreater, uv + float2(-d1, -d2), lightPos.z) +
		shadowTexture.SampleCmpLevelZero(sLinearClampComparisonGreater, uv + float2(d2, -d1), lightPos.z) +
		shadowTexture.SampleCmpLevelZero(sLinearClampComparisonGreater, uv + float2(d1, d2), lightPos.z) +
		shadowTexture.SampleCmpLevelZero(sLinearClampComparisonGreater, uv + float2(-d4, d3), lightPos.z) +
		shadowTexture.SampleCmpLevelZero(sLinearClampComparisonGreater, uv + float2(-d3, -d4), lightPos.z) +
		shadowTexture.SampleCmpLevelZero(sLinearClampComparisonGreater, uv + float2(d4, -d3), lightPos.z) +
		shadowTexture.SampleCmpLevelZero(sLinearClampComparisonGreater, uv + float2(d3, d4), lightPos.z)
		) / 10.0f;
    const float shadow = result * result;
    return lerp(1.0f, shadow, shadowView.ShadowAmount);
}

float3 EvaluateLights(MaterialSurface aSurface, float3 aWorldPosition, float3 aGeometricNormal, float aLinearDepth, uint aLightChannelMask)
{
    const float3 viewDirection = normalize(cView.ViewLocation - aWorldPosition);
    const float f0Scalar = pow((1.0f - aSurface.IOR) / (1.0f + aSurface.IOR), 2.0f); //IOR of 1.5 produces the typical base reflectivity (0.04)
    const float3 baseReflectivity = lerp(float3(f0Scalar, f0Scalar, f0Scalar), aSurface.AlbedoColor, aSurface.Metalness);
    float3 outgoingRadiance = float3(0.0f, 0.0f, 0.0f);
    
    StructuredBuffer<Light> lights = ResourceDescriptorHeap[cView.LightsIndex];
    
    for (uint i = 0u; i < cView.LightCount; ++i)
    {
        const Light light = lights[i];
        
        if (!light.IsEnabled)
        {
            continue;
        }

        if ((light.ChannelMask & aLightChannelMask) == 0u)
        {
            continue;
        }
        
        float3 toLightDirection = float3(0.0f, 0.0f, 0.0f);
        float attenuation = 1.0f;
        
        //if (light.IsDirectional)
        //{
        //    int cascade = GetCascadeIndex(light, aLinearDepth);
        //    ShadowViewData v = GetShadowView(light.ShadowViewBaseIndex + cascade);
        //    float4 lp = mul(float4(aWorldPosition, 1), v.WorldToClip);
        //    lp.xyz /= lp.w;
        //    float2 uv = float2(lp.x * 0.5f + 0.5f, lp.y * -0.5f + 0.5f);
        //    static const float3 COLORS[] = { float3(1, 0, 0), float3(0, 1, 0), float3(0, 0, 1), float3(0, 1, 1) };
        //    float3 color = COLORS[cascade];
        //    if (any(uv < -0.01) || any(uv > 1.01))
        //        color = float3(1, 0, 1);
        //    return color;
        //}
        
        if (light.IsDirectional)
        {
            toLightDirection = normalize(-light.Direction);
            
            if (light.CastShadows)
            {
                const int cascade = GetCascadeIndex(light, aLinearDepth);
                attenuation *= Shadow3x3PCF(aWorldPosition, aGeometricNormal, toLightDirection, light.ShadowViewBaseIndex + cascade);
            }
        }
        else if (light.IsPoint || light.IsSpot)
        {
            const float3 surfaceToLight = light.Position - aWorldPosition;
            toLightDirection = normalize(surfaceToLight);
            attenuation = RadialAttenuation(surfaceToLight, light.Range);
            
            if (light.IsSpot)
            {
                attenuation *= DirectionalAttenuation(surfaceToLight, light.Direction, light.SpotlightAngles.y, light.SpotlightAngles.x);
                
                if (light.CastShadows)
                {
                    attenuation *= Shadow3x3PCF(aWorldPosition, aGeometricNormal, toLightDirection,light.ShadowViewBaseIndex);
                }
            }
            else
            {
                if (light.CastShadows)
                {
                    const uint face = GetCubeFaceIndex(aWorldPosition - light.Position);
                    attenuation *= Shadow3x3PCF(aWorldPosition, aGeometricNormal, toLightDirection, light.ShadowViewBaseIndex + face);
                }
            }
        }
        
        if (attenuation <= 0.0f)
            continue;
        
        const float NoL = dot(aSurface.Normal, toLightDirection);
        if (NoL > 0.0f && attenuation > 0.0f)
        {
            const float3 lightRadiance = light.Color * light.Intensity * attenuation;
            const float3 brdf = CalculateBRDF(toLightDirection, viewDirection, aSurface.Normal, aSurface.Metalness, aSurface.Roughness, aSurface.AlbedoColor, baseReflectivity);
            outgoingRadiance += brdf * lightRadiance * NoL;
        }
    }
    
    return outgoingRadiance;
}

float3 EvaluateIBL(MaterialSurface aSurface, float3 aWorldPosition, uint aLightChannelMask)
{
    const SkyLightData skyLight = GetSkyLight();
    if ((skyLight.LightChannelMask & aLightChannelMask) == 0u)
    {
        return float3(0.0f, 0.0f, 0.0f);
    }

    const float3 viewDirection = normalize(cView.ViewLocation - aWorldPosition);
    const float3 F0 = lerp(float3(0.04f, 0.04f, 0.04f), aSurface.AlbedoColor, aSurface.Metalness);
    const float NoV = clamp(dot(aSurface.Normal, viewDirection), 0.0f, 1.0f);
    const float3 F = FresnelSchlickRoughness(NoV, F0, aSurface.Roughness);
    const float3 kD = (1.0f - F) * (1.0f - aSurface.Metalness);

    const float3 worldNormal = aSurface.Normal;
    const float3 worldReflect = reflect(-viewDirection, aSurface.Normal);

    const float3 rotatedNormal = mul(worldNormal, (float3x3) skyLight.WorldRotation);
    const float3 rotatedReflect = mul(worldReflect, (float3x3) skyLight.WorldRotation);

    const float tDiffuse = smoothstep(-0.08f, 0.08f, -worldNormal.y) * skyLight.LowerHemisphereColor.a;
    const float tSpecular = smoothstep(-0.08f, 0.08f, -worldReflect.y) * skyLight.LowerHemisphereColor.a;

    TextureCube irradianceMap = ResourceDescriptorHeap[skyLight.IrradianceMapIndex];
    TextureCube blendIrradianceMap = ResourceDescriptorHeap[skyLight.BlendIrradianceMapIndex];
    const float3 irradianceA = irradianceMap.Sample(sLinearWrap, rotatedNormal).rgb * skyLight.EnvironmentATintColor;
    const float3 irradianceB = blendIrradianceMap.Sample(sLinearWrap, rotatedNormal).rgb * skyLight.EnvironmentBTintColor;
    const float3 irradiance = lerp(irradianceA, irradianceB, skyLight.BlendFactor);
    const float3 blendedIrradiance = lerp(irradiance, skyLight.LowerHemisphereColor.rgb, tDiffuse);
    const float3 diffuseIBL = aSurface.AlbedoColor * blendedIrradiance;

    TextureCube radianceMap = ResourceDescriptorHeap[skyLight.RadianceMapIndex];
    TextureCube blendRadianceMap = ResourceDescriptorHeap[skyLight.BlendRadianceMapIndex];

    uint widthPrimary, heightPrimary, numMipsPrimary;
    radianceMap.GetDimensions(0, widthPrimary, heightPrimary, numMipsPrimary);

    uint widthBlend, heightBlend, numMipsBlend;
    blendRadianceMap.GetDimensions(0, widthBlend, heightBlend, numMipsBlend);

    const float maxMipPrimary = float(numMipsPrimary - 1u);
    const float mipLevelPrimary = aSurface.Roughness * maxMipPrimary;
    const float maxMipBlend = float(numMipsBlend - 1u);
    const float mipLevelBlend = aSurface.Roughness * maxMipBlend;

    const float3 radianceA = radianceMap.SampleLevel(sLinearWrap, rotatedReflect, mipLevelPrimary).rgb * skyLight.EnvironmentATintColor;
    const float3 radianceB = blendRadianceMap.SampleLevel(sLinearWrap, rotatedReflect, mipLevelBlend).rgb * skyLight.EnvironmentBTintColor;
    const float3 specularIrradiance = lerp(radianceA, radianceB, skyLight.BlendFactor);
    const float3 blendedSpecular = lerp(specularIrradiance, skyLight.LowerHemisphereColor.rgb, tSpecular);

    Texture2D brdfLutTexture = ResourceDescriptorHeap[skyLight.BRDFLutTextureIndex];
    const float2 specularBRDF = brdfLutTexture.Sample(sLinearClamp, float2(NoV, aSurface.Roughness)).rg;
    const float3 specularIBL = blendedSpecular * (F * specularBRDF.r + specularBRDF.g);

    return (kD * diffuseIBL + specularIBL) * skyLight.Tint * skyLight.Intensity;
}