#pragma once

#include "ECS/ECSCommon.h"

#include "Graphics/RHI/Buffer.h"
#include "Graphics/RHI/DescriptorHeap.h"
#include "Graphics/RHI/Texture.h"
#include "Graphics/Shaders/Interop/ShaderInterop.h"

namespace Relentless
{
	class Mesh;
	class Renderer;
	class Scene;

	enum class RenderModeEx : uint8
	{
		Solid,
		Wireframe
	};

	struct ViewTransform
	{
		virtual ~ViewTransform() = default;

		Matrix ViewToClip		= Matrix::Identity;
		Matrix WorldToView		= Matrix::Identity;
		Matrix WorldToClip		= Matrix::Identity;
		Matrix WorldToClipPrev  = Matrix::Identity;
		Matrix ViewToWorld		= Matrix::Identity;
		Matrix ClipToView		= Matrix::Identity;

		Vector3 Location		= Vector3::Zero;
		Vector3 LocationPrev	= Vector3::Zero;

		FloatRect Viewport;
		float FoV				= 60.0f * Math::PI / 180.0f;
		float NearPlane			= 0.1f;
		float FarPlane			= 1'000.0f;

		bool IsPerspective		= true;

		BoundingFrustum PerspectiveFrustum;
		OrientedBoundingBox OrthographicFrustum;

		bool IsInFrustum(const BoundingBox& bb) const
		{
			return IsPerspective ? PerspectiveFrustum.Contains(bb) : OrthographicFrustum.Contains(bb);
		}
	};

	struct ViewportRenderView : public ViewTransform
	{
		Vector2i MouseHoverCoordinates	= Vector2i(-1, -1);
		Ref<Texture> pTarget			= nullptr;
		bool DrawGrid					= true;
		RenderModeEx RenderMode			= RenderModeEx::Solid;
		float MinLogLuminance			= -4.0f;
		float MinEV100					= -10.0f;
		float MaxEV100					= 20.0f;
		float ExposureCompensation		= 1.0f;
	};

	struct RenderView : public ViewTransform
	{
		Renderer* pRenderer = nullptr;
		Scene* pScene		= nullptr;
		Ref<Buffer> ViewCB	= nullptr;
	};

	struct Batch
	{
		enum class Blending : uint8 { Opaque, AlphaMask, AlphaBlend };

		Vector3 Location = Vector3::Zero;
		uint32 InstanceID = std::numeric_limits<uint32>::max();
		Blending BlendMode = Blending::Opaque;
		Mesh* pMesh = nullptr;
		uint32 MaterialIndex = DescriptorHeap::INVALID_DESCRIPTOR_INDEX;
		uint32 MeshIndex = DescriptorHeap::INVALID_DESCRIPTOR_INDEX;
		entity EntityID = NULL_ENTITY;
	};
	DECLARE_BITMASK_TYPE(Batch::Blending)

	struct SceneTextures
	{
		Ref<Texture> pColorTarget = nullptr;
		Ref<Texture> pDepthTarget = nullptr;
	};

	struct SceneBuffer
	{
		Ref<Buffer> pBuffer = nullptr;
		uint32 Count = 0u;
	};

	struct GraphicsOptions
	{
		uint32 SampleCount		= 1u;

		bool HBAOPlusEnabled	= true;
	};

	struct BindingSlot
	{
		static constexpr uint32 PerInstance = 0u;
		static constexpr uint32 PerPass		= 1u;
		static constexpr uint32 PerView		= 2u;
	};
}