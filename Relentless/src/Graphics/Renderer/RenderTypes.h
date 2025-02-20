#pragma once

#include "Graphics/RHI/TextureEx.h"
#include "Graphics/RHI/Buffer.h"

#include "Graphics/Shaders/Interop/ShaderInterop.h"

namespace Relentless
{
	class Renderer;
	class Scene;

	struct ViewTransform
	{
		Matrix ViewToClip		= Matrix::Identity;
		Matrix WorldToView		= Matrix::Identity;
		Matrix WorldToClip		= Matrix::Identity;
		Matrix WorldToClipPrev  = Matrix::Identity;
		Matrix ViewToWorld		= Matrix::Identity;
		Matrix ClipToView		= Matrix::Identity;

		Vector3 Location		= Vector3::Zero;
		Vector3 LocationPrev	= Vector3::Zero;

		FloatRect Viewport;
		float FoV				= 60.0f * 3.14159265358979323846f / 180.0f;
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

	struct ViewportRenderView : ViewTransform
	{
		Ref<TextureEx> pTarget = nullptr;
	};


	struct RenderView : ViewTransform
	{
		Renderer* pRenderer = nullptr;
		Scene* pScene = nullptr;

		Ref<BufferEx> ViewCB = nullptr;
	};

	struct SceneTextures
	{
		Ref<TextureEx> pColorTarget = nullptr;
		Ref<TextureEx> pDepthTarget = nullptr;
	};

	struct SceneBuffer
	{
		Ref<BufferEx> pTarget = nullptr;
		uint64 Count = 0u;
	};

	enum class RenderModeEx : uint8
	{
		Default,
		Wireframe 
	};

	struct GraphicsOptions
	{
		RenderModeEx Mode		= RenderModeEx::Default;
		uint32 SampleCount		= 1u;

		bool GridEnabled		= true;
		bool HBAOPlusEnabled	= true;
	};

	struct BindingSlot
	{
		static constexpr uint32 PerInstance = 0u;
		static constexpr uint32 PerPass		= 1u;
		static constexpr uint32 PerView		= 2u;
	};
}