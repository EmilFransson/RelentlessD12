#pragma once

#ifdef __cplusplus
namespace ShaderInterop
{
using float2 = Vector2;
using float3 = Vector3;
using float4 = Vector4;
using uint = uint32;
using uint2 = Vector2u;
using uint3 = Vector3u;
using uint4 = Vector4u;
using int2 = Vector2i;
using int3 = Vector3i;
using int4 = Vector4i;
using float4x4 = Matrix;
#endif

#define CONCAT_IMPL( x, y ) x##y
#define MACRO_CONCAT( x, y ) CONCAT_IMPL( x, y )
#define PAD uint MACRO_CONCAT(padding, __COUNTER__)
	struct Material
	{
		uint AlbedoIndex;
		uint NormalIndex;
		uint RoughnessIndex;
		uint MetalnessIndex;

		uint EmissiveIndex;
		uint HeightMapIndex;
		uint AOIndex;
		uint RoughnessMetalnessIndex;

		float4 BaseColorFactor;
		float4 EmissiveFactor;

		float MetalnessFactor;
		float RoughnessFactor;
		float AOFactor;
		float HeightFactor;

		float EmissionIntensity;
		float3 padding;

		float2 TilingFactor;
		float2 Offset;
	};

	struct MeshData
	{
		uint VertexBufferIndex;
		uint IndexBufferIndex;
		uint Padding1;
		uint Padding2;
	}; 

	struct InstanceData
	{
		float4x4 LocalToWorld;

		uint ID;
		uint MaterialIndex;
		uint MeshDataIndex;
		uint EntityID;
	};

	struct SkyboxData
	{
		float3 BackgroundColor;
		float Intensity;

		float4x4 WorldRotation;

		uint EnvironmentMapAIndex;
		uint EnvironmentMapBIndex;
		float LODBias;
		float BlendFactor;
	};

	struct SkyLightData
	{
		float3 Tint;
		float Intensity;

		float4x4 WorldRotation;

		uint IrradianceMapIndex;
		uint RadianceMapIndex;
		uint BRDFLutTextureIndex;
		float BlendFactor;

		float4 LowerHemisphereColor;

		uint BlendIrradianceMapIndex;
		uint BlendRadianceMapIndex;
		float2 Padding;
	};

	struct Environment
	{
		SkyboxData Skybox;
		SkyLightData SkyLight;
	};

	struct Light
	{
		float3 Position;
		float Intensity;
		float3 Direction;
		float Range;
		float3 Color;
		uint Padding;

		float2 SpotlightAngles;

		// flags
		uint IsEnabled : 1;
		uint IsSpot : 1;
		uint IsPoint : 1;
		uint IsDirectional : 1;
	};

	struct ViewUniforms
	{
		float4x4 WorldToView;
		float4x4 ViewToWorld;
		float4x4 ViewToClip;
		float4x4 ClipToView;
		float4x4 WorldToClip;
		float4x4 ClipToWorld;

		float3 ViewLocation;
		float Padding0;

		float2 ViewportDimensions;
		float2 ViewportDimensionsInv;

		uint FrameIndex;
		float DeltaTime;
		float ElapsedTime;
		uint SkyLightIndex;

		uint NumInstances;
		uint LightCount;
		uint EnvironmentIndex;
		uint SkyBoxIndex;

		uint InstancesIndex;
		uint MeshesIndex;
		uint MaterialsIndex;
		uint LightsIndex;
	};

	static const uint INVALID_DESCRIPTOR_INDEX = 0xFFFFFFFF;

#ifdef __cplusplus
}
#endif
