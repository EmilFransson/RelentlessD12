#pragma once

#include "ECS/ECSCommon.h"

#include "Graphics/Renderer/RenderFeatures.h"
#include "Graphics/Renderer/RenderQualitySettings.h"
#include "Graphics/RHI/Buffer.h"
#include "Graphics/RHI/DescriptorHeap.h"
#include "Graphics/RHI/Texture.h"
#include "Graphics/Shaders/Interop/ShaderInterop.h"

namespace Relentless
{
	class Mesh;
	class Renderer;
	class RenderScene;

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

		NO_DISCARD bool IsInFrustum(const BoundingBox& aBoundingBox) const
		{
			return IsPerspective ? PerspectiveFrustum.Contains(aBoundingBox) : OrthographicFrustum.Contains(aBoundingBox);
		}
	};

	struct RenderView : public ViewTransform
	{
		RenderFeatures RenderFeatures				= {};
		RenderQualitySettings RenderQualitySettings	= {};
		UUID ViewID									= {};
		Vector2i MouseHoverCoordinates				= Vector2i(-1, -1);
		Renderer* pRenderer							= nullptr;
		RenderScene* pRenderScene					= nullptr;
		Ref<Buffer> ViewCB							= nullptr;
		uint32 FrameIndex							= 0u;
	};

	struct ViewRenderDesc
	{
		ViewTransform ViewTransform					= {};
		UUID SceneID								= {};
		UUID ViewID									= {};
		RenderFeatures RenderFeatures				= {};
		RenderQualitySettings RenderQualitySettings = {};
		Vector2i MouseHoverCoordinates				= Vector2i(-1, -1);
		Ref<Texture> RenderTarget					= nullptr;
	};

	struct Batch
	{
		enum class Blending : uint8 { Opaque = 0u, AlphaMask, AlphaBlend };

		Vector3 Location = Vector3::Zero;
		uint32 InstanceID = std::numeric_limits<uint32>::max();
		uint32 BaseInstanceOffset = 0u;
		Blending BlendMode = Blending::Opaque;
		uint32 NumIndices = 0u;
		uint32 InstanceCount = 0u;
		entity EntityID = NULL_ENTITY;
		bool IsTwoSided = false;
	};
	DECLARE_BITMASK_TYPE(Batch::Blending)

	struct SceneTextureResources
	{
		Ref<Texture> pColorTarget			= nullptr;
		Ref<Texture> pDepthTarget			= nullptr;
		
		Ref<Texture> pMSAAColorTarget		= nullptr;
		Ref<Texture> pMSAADepthTarget		= nullptr;
	};

	struct SceneTextures
	{
		Ref<Texture> pColorTarget						= nullptr;
		Ref<Texture> pColorResolveTarget				= nullptr;
		Ref<Texture> pDepthTarget						= nullptr;
		Ref<Texture> pDepthResolveTarget				= nullptr;
		Ref<Texture> pDepthIntermediateResolveTarget	= nullptr;

		Ref<Texture> pEnvironmentTarget					= nullptr;

		Ref<Texture> pEntityIDTarget					= nullptr;
		Ref<Texture> pEntityDepthTarget					= nullptr;

		Ref<Texture> pOutlinesSolidTarget				= nullptr;
		Ref<Texture> pOutlinesDepthTarget				= nullptr;
		Ref<Texture> pOutlinesIntermediateBlurTarget	= nullptr;
		Ref<Texture> pOutlinesBlurTarget				= nullptr;

		Ref<Texture> pAutoExposureDownscaleTarget		= nullptr;

		Ref<Texture> pOpaqueAlphaMaskedColorTargetCopy	= nullptr;
	};

	struct SceneBuffer
	{
		Ref<Buffer> pBuffer = nullptr;
		uint32 Count = 0u;
	};

	struct SceneBuffers
	{
		SceneBuffer EntityIDReadbackBuffer;
		SceneBuffer AverageLuminanceBuffer;
		SceneBuffer LuminanceHistogramBuffer;
	};

	struct BindingSlot
	{
		static constexpr uint32 PerInstance = 0u;
		static constexpr uint32 PerPass		= 1u;
		static constexpr uint32 PerView		= 2u;
	};

	enum class DefaultTextureType
	{
		White2D,
		Black2D,
		Normal2D,
		BlackCube,
		WhiteCube,
		Max,
	};

	namespace GraphicsCommon
	{
		void Create(GraphicsDevice* aGraphicsDevice) noexcept;
		void Destroy() noexcept;
		NO_DISCARD Texture* GetDefaultTexture(DefaultTextureType aDefaultTextureType) noexcept;
	}
}