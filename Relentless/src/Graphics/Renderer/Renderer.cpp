#include "Renderer.h"

#include "Assets/AssetManager.h"
#include "Assets/CoreTypes/Mesh.h"
#include "Assets/CoreTypes/Texture2D.h"

#include "Core/Time.h"

#include "Graphics/Renderer/Techniques/AutoExposure.h"
#include "Graphics/Renderer/Techniques/BlitPass.h"
#include "Graphics/Renderer/Techniques/Bloom.h"
#include "Graphics/Renderer/Techniques/DepthPrePass.h"
#include "Graphics/Renderer/Techniques/EditorGrid.h"
#include "Graphics/Renderer/Techniques/ExponentialHeightFog.h"
#include "Graphics/Renderer/Techniques/ForwardAlphaBlend.h"
#include "Graphics/Renderer/Techniques/ForwardOpaqueAlphaMask.h"
#include "Graphics/Renderer/Techniques/HBAOPlus.h"
#include "Graphics/Renderer/Techniques/Outlines.h"
#include "Graphics/Renderer/Techniques/Picking.h"
#include "Graphics/Renderer/Techniques/PostProcessing.h"
#include "Graphics/Renderer/Techniques/ResolveDepthPass.h"
#include "Graphics/Renderer/Techniques/SelectionOutlinesCompositePass.h"
#include "Graphics/Renderer/Techniques/ShadowMapping.h"
#include "Graphics/Renderer/Techniques/SkyBoxRenderer.h"

#include "Graphics/RHI/CommandContext.h"
#include "Graphics/RHI/Device.h"
#include "Graphics/RHI/ResourceViews.h"
#include "Graphics/RHI/RingBufferAllocator.h"

#include "Subsystem/CoreTypes/FogRenderSubsystem.h"
#include "Subsystem/CoreTypes/LightRenderSubsystem.h"
#include "Subsystem/CoreTypes/MaterialRenderSubsystem.h"
#include "Subsystem/CoreTypes/MeshRenderSubsystem.h"
#include "Subsystem/CoreTypes/PrimitiveRenderSubsystem.h"
#include "Subsystem/CoreTypes/SkyBoxRenderSubsystem.h"
#include "Subsystem/CoreTypes/SkyLightRenderSubsystem.h"

namespace Relentless
{
	Renderer::Renderer(GraphicsDevice* pDevice) noexcept
		: m_pDevice{pDevice}
	{
		GraphicsCommon::Create(m_pDevice);

		m_pForwardOpaqueAlphaMask			= MakeUnique<ForwardOpaqueAlphaMask>(pDevice);
		m_pForwardAlphaBlend				= MakeUnique<ForwardAlphaBlend>(pDevice);
		m_pEditorGrid						= MakeUnique<EditorGrid>(pDevice);
		m_pPostProcessing					= MakeUnique<PostProcessing>(pDevice);
		m_pDepthPrePass						= MakeUnique<DepthPrePass>(pDevice);
		m_pHBAOPlus							= MakeUnique<HBAOPlus>(pDevice);
		m_pOutlines							= MakeUnique<Outlines>(pDevice);
		m_pAutoExposure						= MakeUnique<AutoExposure>(pDevice);
		m_pSkyBoxRenderer					= MakeUnique<SkyBoxRenderer>(pDevice);
		m_pPicking							= MakeUnique<Picking>(pDevice);
		m_pResolveDepthPass					= MakeUnique<ResolveDepthPass>(pDevice);
		m_pSelectionOutlinesCompositePass	= MakeUnique<SelectionOutlinesCompositePass>(pDevice);
		m_pBlitPass							= MakeUnique<BlitPass>(pDevice);
		m_pBloom							= MakeUnique<Bloom>(pDevice);
		m_pShadowMapping					= MakeUnique<ShadowMapping>(pDevice);
		m_pExponentialHeightFog				= MakeUnique<ExponentialHeightFog>(pDevice);
	}

	Renderer::~Renderer() noexcept
	{
		//Explicit so subsystems can clean up properly:
		m_RenderScenes.clear();
		GraphicsCommon::Destroy();
	}

	void Renderer::Render() noexcept
	{
		PROFILE_FUNC;

		InvokeDispatchRequests();
		NotifySubsystemsOfBRDFLut_Temp();
		InvokeFrameBeginCallbacks();
		InvokeRenderJobs();

		std::vector<PreparedView> preparedViews;
		preparedViews.reserve(m_PendingViews.size());

		for (const ViewRenderDesc& desc : m_PendingViews)
		{
			if (!desc.RenderTarget)            
				continue;

			if (!ExistRenderScene(desc.SceneID)) 
				continue;

			if (!ExistView(desc.ViewID))       
				continue;

			RenderScene* pScene = GetRenderScene(desc.SceneID);
			pScene->OnRenderBegin(desc);

			RenderView& view = BuildRenderView(desc);
			pScene->OnViewPrepare(view);

			preparedViews.push_back({ &view, desc, pScene });
		}

		m_PendingViews.clear();

		InvokeEntityReadbackCallbacks();
		InvokeUploadCallbacks();
		SyncRingBuffer();

		for (PreparedView& aPreparedView : preparedViews)
		{
			SceneTextures& viewTextures = ValidateAndGetViewTextures(aPreparedView.Desc);
			SceneBuffers& viewBuffers = ValidateAndGetViewBuffers(aPreparedView.Desc);

			CommandContext* pCtx = m_pDevice->AllocateCommandContext();
			UploadViewUniforms(*pCtx, *aPreparedView.pView);
			pCtx->Execute();

			Render(*aPreparedView.pView, viewTextures, viewBuffers, aPreparedView.Desc.RenderTarget);
		}

		m_Frame++;
	}

	void Renderer::SyncRingBuffer() noexcept
	{
		PROFILE_FUNC;
		m_pDevice->GetRingBuffer()->Sync();
	}

	CallbackID Renderer::RegisterOnFrameRenderBeginCallback(Callback<void()> aFrameRenderBeginCallback) noexcept
	{
		std::lock_guard<std::mutex> lock(m_OnFrameBeginMutex);
		static CallbackID nextCallbackID = 0;
		CallbackID toReturn = nextCallbackID++;

		m_OnFrameBeginCallbacks.emplace(toReturn, std::move(aFrameRenderBeginCallback));

		return toReturn;
	}

	CallbackID Renderer::RegisterOnUploadCallback(Callback<void(CommandContext&)> aUploadCallback) noexcept
	{
		std::lock_guard<std::mutex> lock(m_OnUploadMutex);
		static CallbackID nextCallbackID = 0;
		CallbackID toReturn = nextCallbackID++;

		m_OnUploadCallbacks.emplace(toReturn, std::move(aUploadCallback));

		return toReturn;
	}

	void Renderer::RenderViews(const std::vector<ViewRenderDesc>& someRenderDescs) noexcept
	{
		m_PendingViews = someRenderDescs;
	}

	void Renderer::SubmitBatch(CommandContext& aCommandContext, const Batch& aBatch) noexcept
	{
		struct
		{
			uint32 BaseInstanceOffset = 0xFFFFFFFF;
		} params;

		params.BaseInstanceOffset = aBatch.BaseInstanceOffset;
		aCommandContext.BindRootCBV(BindingSlot::PerInstance, (const void*)&params, sizeof(params));
		aCommandContext.Draw(0u, aBatch.NumIndices, 0u, aBatch.InstanceCount);
	}

	RenderJobHandle Renderer::SubmitComputeJob(Callback<void(CommandContext&)>&& aCallback) noexcept
	{
		Ref<RenderJobState> pJobState = RLS_NEW RenderJobState();

		std::lock_guard<std::mutex> guard(s_RenderJobMutex);
		RenderJob& renderJob = s_EnqueuedRenderJobs.emplace_back();
		renderJob.Callback = std::move(aCallback);
		renderJob.Type = ERenderJobType::Compute;
		renderJob.State = pJobState;

		return RenderJobHandle(pJobState);
	}

	RenderJobHandle Renderer::SubmitRenderJob(Callback<void(CommandContext&)>&& aCallback) noexcept
	{
		Ref<RenderJobState> pJobState = RLS_NEW RenderJobState();
		
		std::lock_guard<std::mutex> guard(s_RenderJobMutex);
		RenderJob& renderJob = s_EnqueuedRenderJobs.emplace_back();
		renderJob.Callback = std::move(aCallback);
		renderJob.Type = ERenderJobType::Raster;
		renderJob.State = pJobState;

		return RenderJobHandle(pJobState);
	}

	void Renderer::UnregisterOnFrameRenderBeginCallback(CallbackID aCallbackID) noexcept
	{
		std::lock_guard<std::mutex> lock(m_OnFrameBeginMutex);
		m_OnFrameBeginCallbacks.erase(aCallbackID);
	}

	void Renderer::UnregisterOnUploadCallback(CallbackID aCallbackID) noexcept
	{
		std::lock_guard<std::mutex> lock(m_OnUploadMutex);
		m_OnUploadCallbacks.erase(aCallbackID);
	}

	SceneTextures& Renderer::ValidateAndGetViewTextures(const ViewRenderDesc& aViewRenderDesc) noexcept
	{
		RLS_ASSERT(ExistView(aViewRenderDesc.ViewID), "[Renderer::ValidateAndGetSceneTextures]: Render view with UUID does not exist.");
		
		const uint32 width = static_cast<uint32>(aViewRenderDesc.ViewTransform.Viewport.GetWidth());
		const uint32 height = static_cast<uint32>(aViewRenderDesc.ViewTransform.Viewport.GetHeight());
		const bool isMSAA = aViewRenderDesc.RenderQualitySettings.MSAASampleCount != EMSAASampleCount::None;

		SceneTextureResources& sceneTextureResources = m_ViewTextureResources.at(aViewRenderDesc.ViewID);
		SceneTextures& sceneTextures = m_ViewTextures.at(aViewRenderDesc.ViewID);

		if (isMSAA)
		{
			if (!sceneTextureResources.pMSAAColorTarget
				|| sceneTextureResources.pMSAAColorTarget->GetSampleCount() != static_cast<uint32>(aViewRenderDesc.RenderQualitySettings.MSAASampleCount)
				|| sceneTextureResources.pMSAAColorTarget->GetWidth() != width
				|| sceneTextureResources.pMSAAColorTarget->GetHeight() != height)
			{
				sceneTextureResources.pMSAAColorTarget = m_pDevice->CreateTexture(TextureDesc::Create2D(width, height, ResourceFormat::RGBA32_FLOAT, 1u, TextureFlag::RenderTarget | TextureFlag::ShaderResource, ClearBinding(Colors::Black), static_cast<uint32>(aViewRenderDesc.RenderQualitySettings.MSAASampleCount)), "Color HDR MSAA Target");
			}

			if (!sceneTextureResources.pMSAADepthTarget
				|| sceneTextureResources.pMSAADepthTarget->GetSampleCount() != static_cast<uint32>(aViewRenderDesc.RenderQualitySettings.MSAASampleCount)
				|| sceneTextureResources.pMSAADepthTarget->GetWidth() != width
				|| sceneTextureResources.pMSAADepthTarget->GetHeight() != height)
			{
				sceneTextureResources.pMSAADepthTarget = m_pDevice->CreateTexture(TextureDesc::Create2D(width, height, ResourceFormat::R32_TYPELESS, 1u, TextureFlag::DepthStencil | TextureFlag::ShaderResource, ClearBinding(0.0f, 1u), static_cast<uint32>(aViewRenderDesc.RenderQualitySettings.MSAASampleCount)), "Depth MSAA Target");
			}

			if (!sceneTextures.pDepthIntermediateResolveTarget || sceneTextures.pDepthIntermediateResolveTarget->GetWidth() != width || sceneTextures.pDepthIntermediateResolveTarget->GetHeight() != height)
				sceneTextures.pDepthIntermediateResolveTarget = m_pDevice->CreateTexture(TextureDesc::Create2D(width, height, ResourceFormat::R32_FLOAT, 1u, TextureFlag::UnorderedAccess | TextureFlag::ShaderResource), "Depth Intermediate Resolve Target");
		
			if (!sceneTextures.pDepthResolveTarget || sceneTextures.pDepthResolveTarget->GetWidth() != width || sceneTextures.pDepthResolveTarget->GetHeight() != height)
				sceneTextures.pDepthResolveTarget = m_pDevice->CreateTexture(TextureDesc::Create2D(width, height, ResourceFormat::R32_TYPELESS, 1u, TextureFlag::DepthStencil | TextureFlag::ShaderResource, ClearBinding(0.0f, 1u)), "Depth Resolve Target");

			if (!sceneTextures.pColorResolveTarget || sceneTextures.pColorResolveTarget->GetWidth() != width || sceneTextures.pColorResolveTarget->GetHeight() != height)
				sceneTextures.pColorResolveTarget = m_pDevice->CreateTexture(TextureDesc::Create2D(width, height, ResourceFormat::RGBA32_FLOAT, 1u, TextureFlag::RenderTarget | TextureFlag::ShaderResource | TextureFlag::UnorderedAccess), "Color HDR Target");
		}

		if (!sceneTextureResources.pColorTarget || sceneTextureResources.pColorTarget->GetWidth() != width || sceneTextureResources.pColorTarget->GetHeight() != height)
			sceneTextureResources.pColorTarget = m_pDevice->CreateTexture(TextureDesc::Create2D(width, height, ResourceFormat::RGBA32_FLOAT, 1u, TextureFlag::RenderTarget | TextureFlag::ShaderResource | TextureFlag::UnorderedAccess), "Color Target");

		if (!sceneTextureResources.pDepthTarget || sceneTextureResources.pDepthTarget->GetWidth() != width || sceneTextureResources.pDepthTarget->GetHeight() != height)
			sceneTextureResources.pDepthTarget = m_pDevice->CreateTexture(TextureDesc::Create2D(width, height, ResourceFormat::R32_TYPELESS, 1u, TextureFlag::DepthStencil | TextureFlag::ShaderResource, ClearBinding(0.0f, 1u)), "Depth Target");

		if (!sceneTextures.pLDRColorTarget || sceneTextures.pLDRColorTarget->GetWidth() != width || sceneTextures.pLDRColorTarget->GetHeight() != height)
			sceneTextures.pLDRColorTarget = m_pDevice->CreateTexture(TextureDesc::Create2D(width, height, ResourceFormat::RGB10A2_UNORM, 1u, TextureFlag::RenderTarget | TextureFlag::ShaderResource | TextureFlag::UnorderedAccess), "LDR Color Target");

		sceneTextures.pHDRColorTarget = isMSAA ? sceneTextureResources.pMSAAColorTarget : sceneTextureResources.pColorTarget;
		sceneTextures.pDepthTarget = isMSAA ? sceneTextureResources.pMSAADepthTarget : sceneTextureResources.pDepthTarget;
		
		//Entity ID:
		if (!sceneTextures.pEntityIDTarget || sceneTextures.pEntityIDTarget->GetWidth() != width || sceneTextures.pEntityIDTarget->GetHeight() != height)
			sceneTextures.pEntityIDTarget = m_pDevice->CreateTexture(TextureDesc::Create2D(width, height, ResourceFormat::R32_FLOAT, 1u, TextureFlag::RenderTarget | TextureFlag::ShaderResource), "Entity ID Target");

		if (!sceneTextures.pEntityDepthTarget || sceneTextures.pEntityDepthTarget->GetWidth() != width || sceneTextures.pEntityDepthTarget->GetHeight() != height)
			sceneTextures.pEntityDepthTarget = m_pDevice->CreateTexture(TextureDesc::Create2D(width, height, ResourceFormat::R32_TYPELESS, 1u, TextureFlag::DepthStencil, ClearBinding(0.0f, 1u)), "Entity Depth Target");

		//Outlines:
		if (aViewRenderDesc.RenderFeatures.IsEnabled(ERenderFeature::Outlines))
		{
			if (!sceneTextures.pOutlinesSolidTarget || sceneTextures.pOutlinesSolidTarget->GetWidth() != width || sceneTextures.pOutlinesSolidTarget->GetHeight() != height)
				sceneTextures.pOutlinesSolidTarget = m_pDevice->CreateTexture(TextureDesc::Create2D(width, height, ResourceFormat::R32_FLOAT, 1u, TextureFlag::RenderTarget | TextureFlag::ShaderResource), "Outlines Solid Target");

			if (!sceneTextures.pOutlinesDepthTarget || sceneTextures.pOutlinesDepthTarget->GetWidth() != width || sceneTextures.pOutlinesDepthTarget->GetHeight() != height)
				sceneTextures.pOutlinesDepthTarget = m_pDevice->CreateTexture(TextureDesc::Create2D(width, height, ResourceFormat::R32_TYPELESS, 1u, TextureFlag::DepthStencil, ClearBinding(0.0f, 1u)), "Outlines Depth Target");

			if (!sceneTextures.pOutlinesIntermediateBlurTarget || sceneTextures.pOutlinesIntermediateBlurTarget->GetWidth() != width || sceneTextures.pOutlinesIntermediateBlurTarget->GetHeight() != height)
				sceneTextures.pOutlinesIntermediateBlurTarget = m_pDevice->CreateTexture(TextureDesc::Create2D(width, height, ResourceFormat::R32_FLOAT, 1u, TextureFlag::UnorderedAccess | TextureFlag::ShaderResource), "Outlines Intermediate Blur Target");

			if (!sceneTextures.pOutlinesBlurTarget || sceneTextures.pOutlinesBlurTarget->GetWidth() != width || sceneTextures.pOutlinesBlurTarget->GetHeight() != height)
				sceneTextures.pOutlinesBlurTarget = m_pDevice->CreateTexture(TextureDesc::Create2D(width, height, ResourceFormat::R32_FLOAT, 1u, TextureFlag::UnorderedAccess | TextureFlag::ShaderResource), "Outlines Blur Target");
		}

		//Auto exposure:
		const uint32 downscaleWidth = Math::DivideAndRoundUp(width, 4u);
		const uint32 downscaleHeight = Math::DivideAndRoundUp(height, 4u);
		if (!sceneTextures.pAutoExposureDownscaleTarget || sceneTextures.pAutoExposureDownscaleTarget->GetWidth() != downscaleWidth || sceneTextures.pAutoExposureDownscaleTarget->GetHeight() != downscaleHeight)
			sceneTextures.pAutoExposureDownscaleTarget = m_pDevice->CreateTexture(TextureDesc::Create2D(downscaleWidth, downscaleHeight, ResourceFormat::RGBA32_FLOAT, 1u, TextureFlag::UnorderedAccess | TextureFlag::ShaderResource), "Autoexposure Downscale Target");

		//Color + Alpha Masked Color Target Copy
		if (!sceneTextures.pOpaqueAlphaMaskedColorTargetCopy || sceneTextures.pOpaqueAlphaMaskedColorTargetCopy->GetWidth() != width || sceneTextures.pOpaqueAlphaMaskedColorTargetCopy->GetHeight() != height)
			sceneTextures.pOpaqueAlphaMaskedColorTargetCopy = m_pDevice->CreateTexture(TextureDesc::Create2D(width, height, ResourceFormat::RGBA32_FLOAT, 1u, TextureFlag::RenderTarget | TextureFlag::ShaderResource), "Opaque & Alpha Masked Color Target Copy");

		//Bloom:
		auto ComputeNumMips = [](uint32 width, uint32 height) -> uint32 { return (uint32)Math::Floor(std::log2f((float)Math::Max(width, height))) + 1u; };
		const Vector2u bloomDimensions = Vector2u(width >> 1, height >> 1);
		constexpr uint32 mipBias = 3;
		const uint32 numMips = ComputeNumMips(bloomDimensions.x, bloomDimensions.y) - mipBias;
		if (!sceneTextures.pBloomDownsampleTarget || sceneTextures.pBloomDownsampleTarget->GetWidth() != bloomDimensions.x || sceneTextures.pBloomDownsampleTarget->GetHeight() != bloomDimensions.y || sceneTextures.pBloomDownsampleTarget->GetMipLevels() != numMips)
			sceneTextures.pBloomDownsampleTarget = m_pDevice->CreateTexture(TextureDesc::Create2D(bloomDimensions.x, bloomDimensions.y, ResourceFormat::RGBA16_FLOAT, numMips, TextureFlag::UnorderedAccess | TextureFlag::ShaderResource), "Bloom Downscale Target");

		if (!sceneTextures.pBloomUpscaleTarget || sceneTextures.pBloomUpscaleTarget->GetWidth() != bloomDimensions.x || sceneTextures.pBloomUpscaleTarget->GetHeight() != bloomDimensions.y || sceneTextures.pBloomUpscaleTarget->GetMipLevels() != (Math::Max(2u, numMips) - 1u))
			sceneTextures.pBloomUpscaleTarget = m_pDevice->CreateTexture(TextureDesc::Create2D(bloomDimensions.x, bloomDimensions.y, ResourceFormat::RGBA16_FLOAT, Math::Max(2u, numMips) - 1, TextureFlag::UnorderedAccess | TextureFlag::ShaderResource), "Bloom Upscale Target");

		//Environment:
		sceneTextures.pEnvironmentTarget = GraphicsCommon::GetDefaultTexture(DefaultTextureType::BlackCube);

		return sceneTextures;
	}

	SceneBuffers& Renderer::ValidateAndGetViewBuffers(const ViewRenderDesc& aViewRenderDesc) noexcept
	{
		RLS_ASSERT(ExistView(aViewRenderDesc.ViewID), "[Renderer::ValidateAndGetSceneBuffers]: Render view with UUID does not exist.");

		SceneBuffers& sceneBuffers = m_ViewBuffers.at(aViewRenderDesc.ViewID);

		if (!sceneBuffers.EntityIDReadbackBuffer.pBuffer)
			sceneBuffers.EntityIDReadbackBuffer.pBuffer = m_pDevice->CreateBuffer(BufferDesc::CreateReadback(sizeof(float)), "Entity ID Readback Buffer");
		
		if (!sceneBuffers.AverageLuminanceBuffer.pBuffer)
			sceneBuffers.AverageLuminanceBuffer.pBuffer = m_pDevice->CreateBuffer(BufferDesc::CreateStructured(3u, sizeof(float), BufferFlag::UnorderedAccess), "Average Luminance Buffer");
		
		if (!sceneBuffers.LuminanceHistogramBuffer.pBuffer)
			sceneBuffers.LuminanceHistogramBuffer.pBuffer = m_pDevice->CreateBuffer(BufferDesc::CreateTyped(256u, ResourceFormat::R32_UINT, BufferFlag::UnorderedAccess), "Luminance Histogram Buffer");

		return sceneBuffers;
	}

	void Renderer::DrawScene(CommandContext& context, const RenderView& view, Batch::Blending blendMode) noexcept
	{
		DrawScene(context, view.pRenderScene->GetBatches(), blendMode);
	}

	void Renderer::DrawScene(CommandContext& context, Span<const Batch> batches, Batch::Blending blendMode) noexcept
	{
		PROFILE_SCOPE("Renderer::DrawScene");

		for (const Batch& batch : batches)
		{
			if (batch.BlendMode == blendMode)
				SubmitBatch(context, batch);
		}
	}

	bool Renderer::ExistRenderScene(const UUID& aUUID) const noexcept
	{
		return m_RenderScenes.contains(aUUID);
	}

	bool Renderer::ExistView(const UUID& aUUID) const noexcept
	{
		return m_ViewTextures.contains(aUUID) && m_ViewTextureResources.contains(aUUID) && m_RenderViews.contains(aUUID);
	}

	GraphicsDevice* Renderer::GetDevice() const noexcept
	{
		return m_pDevice;
	}

	uint32 Renderer::GetFrameIndex() const noexcept
	{
		return m_Frame;
	}

	RenderScene* Renderer::GetRenderScene(const UUID& aUUID) const noexcept
	{
		RLS_ASSERT(ExistRenderScene(aUUID), "[Renderer::GetRenderScene]: Render scene with UUID does not exist.");
		return m_RenderScenes.at(aUUID).get();
	}

	void Renderer::BindViewData(CommandContext& commandContext, const RenderView& aRenderView) noexcept
	{
		ShaderInterop::ViewUniforms viewUniforms;
		aRenderView.pRenderer->GetViewUniforms(aRenderView, viewUniforms);
		commandContext.BindRootCBV(BindingSlot::PerView, &viewUniforms, sizeof(ShaderInterop::ViewUniforms));
	}

	RenderView& Renderer::BuildRenderView(const ViewRenderDesc& aViewRenderDesc) noexcept
	{
		RLS_ASSERT(ExistView(aViewRenderDesc.ViewID), "[Renderer::BuildRenderView]: Render view with UUID does not exist.");

		RenderView& renderView = m_RenderViews.at(aViewRenderDesc.ViewID);
		renderView.Viewport = aViewRenderDesc.ViewTransform.Viewport;
		renderView.ViewToWorld = aViewRenderDesc.ViewTransform.ViewToWorld;
		renderView.WorldToView = aViewRenderDesc.ViewTransform.WorldToView;
		renderView.ClipToView = aViewRenderDesc.ViewTransform.ClipToView;
		renderView.WorldToClipPrev = renderView.WorldToClip;
		renderView.WorldToClip = aViewRenderDesc.ViewTransform.WorldToClip;
		renderView.ViewToClip = aViewRenderDesc.ViewTransform.ViewToClip;

		renderView.LocationPrev = renderView.Location;
		renderView.Location = aViewRenderDesc.ViewTransform.Location;

		renderView.NearPlane = aViewRenderDesc.ViewTransform.NearPlane;
		renderView.FarPlane = aViewRenderDesc.ViewTransform.FarPlane;
		renderView.FoV = aViewRenderDesc.ViewTransform.FoV;
		renderView.IsPerspective = aViewRenderDesc.ViewTransform.IsPerspective;

		renderView.PerspectiveFrustum = aViewRenderDesc.ViewTransform.PerspectiveFrustum;
		renderView.OrthographicFrustum = aViewRenderDesc.ViewTransform.OrthographicFrustum;

		renderView.pRenderer = this;
		renderView.pRenderScene = GetRenderScene(aViewRenderDesc.SceneID);
		renderView.FrameIndex = GetFrameIndex();
		renderView.MouseHoverCoordinates = aViewRenderDesc.MouseHoverCoordinates;

		renderView.RenderFeatures = aViewRenderDesc.RenderFeatures;
		renderView.RenderQualitySettings = aViewRenderDesc.RenderQualitySettings;
		renderView.RenderViewMode = aViewRenderDesc.RenderViewMode;
		renderView.ViewID = aViewRenderDesc.ViewID;

		return renderView;
	}

	void Renderer::CreateRenderScene(const UUID& aUUID) noexcept
	{
		RLS_ASSERT(!ExistRenderScene(aUUID), "[Renderer::CreateRenderScene]: Render scene with UUID already exists.");
		m_RenderScenes.emplace(aUUID, MakeUnique<RenderScene>(aUUID, this));
	}

	void Renderer::CreateView(const UUID& aUUID) noexcept
	{
		RLS_ASSERT(!ExistView(aUUID), "[Renderer::CreateView]: View with UUID already exists.");
		m_ViewTextures.emplace(aUUID, SceneTextures{});
		m_ViewBuffers.emplace(aUUID, SceneBuffers{});
		m_RenderViews.emplace(aUUID, RenderView{});
		m_ViewTextureResources.emplace(aUUID, SceneTextureResources{});
	}

	void Renderer::DestroyRenderScene(const UUID& aUUID) noexcept
	{
		RLS_ASSERT(ExistRenderScene(aUUID), "[Renderer::DestroyRenderScene]: Render scene with UUID does not exist.");
		m_RenderScenes.erase(aUUID);
	}

	void Renderer::DestroyView(const UUID& aUUID) noexcept
	{
		RLS_ASSERT(ExistView(aUUID), "[Renderer::DestroyView]: View with UUID does not exist.");
		m_ViewTextures.erase(aUUID);
		m_ViewBuffers.erase(aUUID);
		m_RenderViews.erase(aUUID);
		m_ViewTextureResources.erase(aUUID);

	}

	void Renderer::Dispatch(Callback<void(Renderer*)>&& aCallback) noexcept
	{
		std::lock_guard<std::mutex> guard(s_DispatchMutex);
		s_EnqueuedRequests.emplace_back(std::move(aCallback));
	}

	void Renderer::GetViewUniforms(const RenderView& aRenderView, ShaderInterop::ViewUniforms& outViewUniform) noexcept
	{
		RenderScene* pRenderScene = aRenderView.pRenderScene;

		LightRenderSubsystem* pLightRenderSubsystem = pRenderScene->GetSubsystem<LightRenderSubsystem>();
		MaterialRenderSubsystem* pMaterialRenderSubsystem = pRenderScene->GetSubsystem<MaterialRenderSubsystem>();
		MeshRenderSubsystem* pMeshRenderSubsystem = pRenderScene->GetSubsystem<MeshRenderSubsystem>();
		PrimitiveRenderSubsystem* pPrimitiveRenderSubsystem = pRenderScene->GetSubsystem<PrimitiveRenderSubsystem>();
		SkyLightRenderSubsystem* pSkyLightRenderSubsystem = pRenderScene->GetSubsystem<SkyLightRenderSubsystem>();
		SkyBoxRenderSubsystem* pSkyBoxRenderSubsystem = pRenderScene->GetSubsystem<SkyBoxRenderSubsystem>();
		FogRenderSubsystem* pFogRenderSubsystem = pRenderScene->GetSubsystem<FogRenderSubsystem>();

		outViewUniform.WorldToView				= aRenderView.WorldToView;
		outViewUniform.ViewToWorld				= aRenderView.ViewToWorld;
		outViewUniform.ViewToClip				= aRenderView.ViewToClip;
		outViewUniform.ClipToView				= aRenderView.ClipToView;
		outViewUniform.WorldToClip				= aRenderView.WorldToClip;
		outViewUniform.WorldToClip.Invert(outViewUniform.ClipToWorld);

		outViewUniform.ViewLocation				= aRenderView.Location;

		outViewUniform.ViewportDimensions		= Vector2(aRenderView.Viewport.GetWidth(), aRenderView.Viewport.GetHeight());
		outViewUniform.ViewportDimensionsInv	= Vector2(1.0f / aRenderView.Viewport.GetWidth(), 1.0f / aRenderView.Viewport.GetHeight());

		outViewUniform.FrameIndex				= aRenderView.FrameIndex;
		outViewUniform.DeltaTime				= Time::GetDeltaTime();
		outViewUniform.ElapsedTime				= Time::GetElapsedTime();

		outViewUniform.NumInstances				= pPrimitiveRenderSubsystem->GetNumInstances();
		outViewUniform.LightCount				= pLightRenderSubsystem->GetNumLights();
		outViewUniform.EnvironmentIndex			= ShaderInterop::INVALID_DESCRIPTOR_INDEX;
		
		outViewUniform.InstancesIndex			= pPrimitiveRenderSubsystem->GetRenderData()->GetSRVIndex();
		outViewUniform.MeshesIndex				= pMeshRenderSubsystem->GetRenderData()->GetSRVIndex();
		outViewUniform.MaterialsIndex			= pMaterialRenderSubsystem->GetRenderData()->GetSRVIndex();
		outViewUniform.LightsIndex				= pLightRenderSubsystem->GetLightRenderData()->GetSRVIndex();

		outViewUniform.SkyLightIndex			= pSkyLightRenderSubsystem->GetRenderData()->GetSRVIndex();
		outViewUniform.SkyBoxIndex				= pSkyBoxRenderSubsystem->GetRenderData()->GetSRVIndex();
		outViewUniform.ShadowViewsIndex			= pLightRenderSubsystem->GetShadowViewsRenderData()->GetSRVIndex();
		outViewUniform.FogIndex					= pFogRenderSubsystem->GetRenderData()->GetSRVIndex();

		outViewUniform.RenderFeatures			= aRenderView.RenderFeatures.ToShaderMask();
		outViewUniform.RenderViewMode			= static_cast<uint32>(aRenderView.RenderViewMode);
	}

	void Renderer::InvokeDispatchRequests() noexcept
	{
		PROFILE_FUNC;

		std::vector<Callback<void(Renderer*)>> dispatchRequests;
		{
			std::lock_guard<std::mutex> guard(s_DispatchMutex);
			std::swap(s_EnqueuedRequests, dispatchRequests);
		}

		for (auto& request : dispatchRequests)
			request(this);
	}

	void Renderer::InvokeEntityReadbackCallbacks() noexcept
	{
		PROFILE_FUNC;

		while (!m_EntityIDSyncDatas.empty())
		{
			EntityIDReadbackData& readbackData = m_EntityIDSyncDatas.front();

			if (!readbackData.Sync.IsComplete())
				break;

			if (!ExistRenderScene(readbackData.RenderSceneUUID) || !ExistView(readbackData.ViewUUID))
			{
				m_EntityIDSyncDatas.pop();
				continue;
			}

			SceneBuffer& entityIDReadbackBuffer = m_ViewBuffers.at(readbackData.ViewUUID).EntityIDReadbackBuffer;

			entityIDReadbackBuffer.pBuffer->Map(0u, nullptr);

			void* pMappedData = entityIDReadbackBuffer.pBuffer->GetMappedData();
			const float encodedID = *reinterpret_cast<float*>(pMappedData);

			entityIDReadbackBuffer.pBuffer->Unmap(0u, nullptr);

			const uint32 id = static_cast<uint32>(encodedID);
			OnEntityIDReadbackDone(id, readbackData.RenderSceneUUID);

			m_EntityIDSyncDatas.pop();
		}
	}

	void Renderer::InvokeFrameBeginCallbacks() noexcept
	{
		PROFILE_FUNC;

		std::lock_guard<std::mutex> guard(m_OnFrameBeginMutex);
		for (auto& [id, frameBeginCallback] : m_OnFrameBeginCallbacks)
			frameBeginCallback();
	}

	void Renderer::InvokeRenderJobs() noexcept
	{
		PROFILE_FUNC;

		for (auto it = s_InProgressRenderJobs.begin(); it != s_InProgressRenderJobs.end();)
		{
			RenderJob& job = *it;
			if (!job.State->Sync.IsComplete())
				break;

			job.State->ConditionVariable.notify_all();
			it = s_InProgressRenderJobs.erase(it);
		}

		std::lock_guard<std::mutex> guard(s_RenderJobMutex);
		for (auto& job : s_EnqueuedRenderJobs)
		{
			CommandContext* pCommandContext = m_pDevice->AllocateCommandContext(job.Type == ERenderJobType::Raster ? D3D12_COMMAND_LIST_TYPE_DIRECT : D3D12_COMMAND_LIST_TYPE_COMPUTE);
			job.Callback(*pCommandContext);
			job.State->Sync = pCommandContext->Execute();
			job.State->Submitted = true;
		}

		std::ranges::move(s_EnqueuedRenderJobs, std::back_inserter(s_InProgressRenderJobs));
		s_EnqueuedRenderJobs.clear();
	}

	void Renderer::InvokeUploadCallbacks() noexcept
	{
		PROFILE_FUNC;

		CommandContext* pCommandContext = m_pDevice->AllocateCommandContext();

		for (auto& [id, callback] : m_OnUploadCallbacks)
			callback(*pCommandContext);

		pCommandContext->Execute();
	}

	void Renderer::NotifySubsystemsOfBRDFLut_Temp() noexcept
	{
		if (!m_BRDFLutTextureHandle.IsValid())
		{
			m_BRDFLutTextureHandle = m_OnRequestBRDFLut();
			Ref<Texture2D> pBRDFLutTexture = AssetManager::Get<Texture2D>(m_BRDFLutTextureHandle);
			pBRDFLutTexture->CreateResource();
		}
		for (auto& [id, pRenderScene] : m_RenderScenes)
			pRenderScene->GetSubsystem<SkyLightRenderSubsystem>()->SetBRDFLutTexture(AssetManager::Get<Texture2D>(m_BRDFLutTextureHandle)->GetResource());
	}

	void Renderer::Render(RenderView& aRenderView, SceneTextures& aSceneTextures, SceneBuffers& aSceneBuffers, const Ref<Texture>& aTarget) noexcept
	{
		std::vector<CommandContext*> commandContexts;

		//Color target clear:
		{
			CommandContext* pCommandContext = m_pDevice->AllocateCommandContext();
			
			pCommandContext->InsertResourceBarrier(aSceneTextures.pHDRColorTarget, D3D12_RESOURCE_STATE_RENDER_TARGET);
			pCommandContext->ClearRenderTarget(aSceneTextures.pHDRColorTarget);
			
			commandContexts.push_back(pCommandContext);
		}
		
		//Pre-Z
		{
			PROFILE_SCOPE("Renderer::Render::Pre-Z");

			CommandContext* pCommandContext = m_pDevice->AllocateCommandContext();
			m_pDepthPrePass->Render(*pCommandContext, aRenderView, aSceneTextures);

			commandContexts.push_back(pCommandContext);
		}

		//Skybox
		if (aRenderView.RenderFeatures.IsEnabled(ERenderFeature::Skybox))
		{
			PROFILE_SCOPE("Renderer::Render::Skybox");
			
			CommandContext* pCommandContext = m_pDevice->AllocateCommandContext();
			m_pSkyBoxRenderer->Render(*pCommandContext, aRenderView, aSceneTextures);
			
			commandContexts.push_back(pCommandContext);
		}

		m_pDevice->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT)->InsertWait(m_pDevice->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE));
		
		//Shadow maps:
		{
			PROFILE_SCOPE("Renderer::Render::ShadowMap");
		
			CommandContext* pCommandContext = m_pDevice->AllocateCommandContext();
			m_pShadowMapping->Render(*pCommandContext, aRenderView);
		
			commandContexts.push_back(pCommandContext);
		}

		//Forward Opaque + Alpha Masked
		{
			PROFILE_SCOPE("Renderer::Render::ForwardOpaqueAlphaMask");
		
			CommandContext* pCommandContext = m_pDevice->AllocateCommandContext();
			m_pForwardOpaqueAlphaMask->Render(*pCommandContext, aRenderView, aSceneTextures);
			
			commandContexts.push_back(pCommandContext);
		}

		//Copy/Resolve scene color (referenced in alpha blend pass)
		{
			CommandContext* pCommandContext = m_pDevice->AllocateCommandContext();
			if (aRenderView.RenderQualitySettings.MSAASampleCount != EMSAASampleCount::None)
			{
				PROFILE_SCOPE("Renderer::Render::PreAlphaBlendColorResolve");
				
				pCommandContext->InsertResourceBarrier(aSceneTextures.pHDRColorTarget, D3D12_RESOURCE_STATE_RESOLVE_SOURCE);
				pCommandContext->InsertResourceBarrier(aSceneTextures.pOpaqueAlphaMaskedColorTargetCopy, D3D12_RESOURCE_STATE_RESOLVE_DEST);
				pCommandContext->ResolveResource(aSceneTextures.pHDRColorTarget, 0u, aSceneTextures.pOpaqueAlphaMaskedColorTargetCopy, 0u, aSceneTextures.pOpaqueAlphaMaskedColorTargetCopy->GetFormat());
			}
			else
			{
				PROFILE_SCOPE("Renderer::Render::PreAlphaBlendColorCopy");
				
				pCommandContext->InsertResourceBarrier(aSceneTextures.pHDRColorTarget, D3D12_RESOURCE_STATE_COPY_SOURCE);
				pCommandContext->InsertResourceBarrier(aSceneTextures.pOpaqueAlphaMaskedColorTargetCopy, D3D12_RESOURCE_STATE_COPY_DEST);
				pCommandContext->CopyResource(aSceneTextures.pHDRColorTarget, aSceneTextures.pOpaqueAlphaMaskedColorTargetCopy);
			}
			commandContexts.push_back(pCommandContext);
		}

		//Forward Alpha Blend
		{
			PROFILE_SCOPE("Renderer::Render::ForwardAlphaBlend");
		
			CommandContext* pCommandContext = m_pDevice->AllocateCommandContext();
			m_pForwardAlphaBlend->Render(*pCommandContext, aRenderView, aSceneTextures);
		
			commandContexts.push_back(pCommandContext);
		}

		//Depth resolve:
		{
			if (aRenderView.RenderQualitySettings.MSAASampleCount != EMSAASampleCount::None)
			{
				PROFILE_SCOPE("Renderer::Render::DepthResolve");
				CommandContext* pCommandContext = m_pDevice->AllocateCommandContext();
				m_pResolveDepthPass->Render(*pCommandContext, aRenderView, aSceneTextures);
				commandContexts.push_back(pCommandContext);
			}
		}

		//Exponential Height Fog
		if (aRenderView.RenderFeatures.IsEnabled(ERenderFeature::ExponentialHeightFog))
		{
			PROFILE_SCOPE("Renderer::Render::ExponentialHeightFog");

			CommandContext* pCommandContext = m_pDevice->AllocateCommandContext();
			m_pExponentialHeightFog->Render(*pCommandContext, aRenderView, aSceneTextures);
			commandContexts.push_back(pCommandContext);
		}

		//HBAO+
		if (aRenderView.RenderFeatures.IsEnabled(ERenderFeature::HBAOPlus))
		{
			PROFILE_SCOPE("Renderer::Render::HBAO+");
		
			CommandContext* pCommandContext = m_pDevice->AllocateCommandContext();
			m_pHBAOPlus->Render(*pCommandContext, aRenderView, aSceneTextures);
			commandContexts.push_back(pCommandContext);
		}

		CommandContext::Execute(commandContexts);
		commandContexts.clear();

		//Entity ID Picking
		if (aRenderView.RenderFeatures.IsEnabled(ERenderFeature::EntityPicking))
		{
			PROFILE_SCOPE("Renderer::Render::Entity IDs");
			CommandContext* pCommandContext = m_pDevice->AllocateCommandContext();
			m_pPicking->Render(*pCommandContext, aRenderView, aSceneTextures, aSceneBuffers);
			SyncPoint s = pCommandContext->Execute();
			m_EntityIDSyncDatas.push({ .Sync = s, .RenderSceneUUID = aRenderView.pRenderScene->GetUUID(), .ViewUUID = aRenderView.ViewID });
		}

		//Outlines
		if (aRenderView.RenderFeatures.IsEnabled(ERenderFeature::Outlines))
		{
			PROFILE_SCOPE("Renderer::Render::Outlines");
			CommandContext* pCommandContext = m_pDevice->AllocateCommandContext();
			m_pOutlines->Render(*pCommandContext, aRenderView, aSceneTextures);
			commandContexts.push_back(pCommandContext);
		}

		//Histogram Auto Exposure:
		if (aRenderView.RenderFeatures.IsEnabled(ERenderFeature::AutoExposure))
		{
			PROFILE_SCOPE("Renderer::Render::AutoExposure");
			CommandContext* pCommandContext = m_pDevice->AllocateCommandContext();
			m_pAutoExposure->Render(*pCommandContext, aRenderView, aSceneTextures, aSceneBuffers);
			commandContexts.push_back(pCommandContext);
		}

		//Bloom:
		if (aRenderView.RenderFeatures.IsEnabled(ERenderFeature::Bloom))
		{
			PROFILE_SCOPE("Renderer::Render::Bloom");
			CommandContext* pCommandContext = m_pDevice->AllocateCommandContext();
			m_pBloom->Render(*pCommandContext, aSceneTextures);
			commandContexts.push_back(pCommandContext);
		}

		if (!commandContexts.empty())
			CommandContext::Execute(commandContexts);

		//Transition LDR & final texture to UAV:
		{
			CommandContext* pCommandContext = m_pDevice->AllocateCommandContext();
			pCommandContext->InsertResourceBarrier(aTarget, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			pCommandContext->InsertResourceBarrier(aSceneTextures.pLDRColorTarget, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			pCommandContext->Execute();
		}

		m_pDevice->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE)->InsertWait(m_pDevice->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT));
		
		//Post process; HDR -> LDR:
		{
			PROFILE_SCOPE("Renderer::Render::Post Process");

			CommandContext* pCommandContext = m_pDevice->AllocateCommandContext();
			m_pPostProcessing->Render(*pCommandContext, aRenderView, aSceneTextures, aSceneBuffers.AverageLuminanceBuffer.pBuffer);
			pCommandContext->Execute();
		}

		/*BEGIN LDR*/
		
		//Editor Grid:
		if (aRenderView.RenderFeatures.IsEnabled(ERenderFeature::Grid))
		{
			PROFILE_SCOPE("Renderer::Render::Editor Grid");

			CommandContext* pCommandContext = m_pDevice->AllocateCommandContext();
			m_pEditorGrid->Render(*pCommandContext, aRenderView, aSceneTextures);
			pCommandContext->Execute();
		}

		m_pDevice->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE)->InsertWait(m_pDevice->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT));
		
		//Selection Outlines Composite:
		if (aRenderView.RenderFeatures.IsEnabled(ERenderFeature::Outlines))
		{
			PROFILE_SCOPE("Renderer::Render::Selection Outlines Compose");
			CommandContext* pCommandContext = m_pDevice->AllocateCommandContext(D3D12_COMMAND_LIST_TYPE_COMPUTE);
			m_pSelectionOutlinesCompositePass->Render(*pCommandContext, aRenderView, aSceneTextures);
			pCommandContext->Execute();
		}

		//Gamma Correction & Blit to final target:
		{
			PROFILE_SCOPE("Renderer::Render::Blit");

			CommandContext* pCommandContext = m_pDevice->AllocateCommandContext(D3D12_COMMAND_LIST_TYPE_COMPUTE);
			m_pBlitPass->Render(*pCommandContext, aRenderView, aSceneTextures, aTarget);
			pCommandContext->Execute();

			m_pDevice->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT)->InsertWait(m_pDevice->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE));
		}

		//Transition final texture to Pixel SRV:
		{
			CommandContext* pCommandContext = m_pDevice->AllocateCommandContext();
			pCommandContext->InsertResourceBarrier(aTarget, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			pCommandContext->Execute();
		}
	}

	void Renderer::UploadViewUniforms(CommandContext& commandContext, RenderView& aRenderView) noexcept
	{
		ScratchAllocation alloc = commandContext.AllocateScratch(sizeof(ShaderInterop::ViewUniforms));
		ShaderInterop::ViewUniforms& parameters = alloc.As<ShaderInterop::ViewUniforms>();
		GetViewUniforms(aRenderView, parameters);
		
		if (!aRenderView.ViewCB)
			aRenderView.ViewCB = commandContext.GetParent()->CreateBuffer(BufferDesc{ .Size = sizeof(ShaderInterop::ViewUniforms), .ElementSize = sizeof(ShaderInterop::ViewUniforms) }, "ViewUniforms");
		
		commandContext.CopyBuffer(alloc.pBackingResource, aRenderView.ViewCB, alloc.Size, alloc.Offset, 0);
	}
}