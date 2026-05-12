#pragma once
#include "Assets/AssetMeta.h"

#include "Callback/Callback.h"
#include "Callback/Broadcaster.h"
#include "Core/DLLExport.h"

#include "Graphics/RHI/RHI.h"
#include "Graphics/Scene/RenderScene.h"

#include "RenderTypes.h"

namespace ShaderInterop
{
	struct ViewUniforms;
}

namespace Relentless
{
	class AutoExposure;
	class DepthPrePass;
	class EditorGrid;
	class ForwardRenderer;
	class ForwardOpaqueAlphaMask;
	class ForwardAlphaBlend;
	class HBAOPlus;
	class Outlines;
	class Picking;
	class PostProcessing;
	class ResolveDepthPass;
	class Scene;
	class SkyBoxRenderer;

	enum class ERenderJobType : uint8 { None = 0u, Raster, Compute };

	class RLS_API Renderer
	{
	public:
		Renderer(GraphicsDevice* pDevice) noexcept;
		~Renderer() noexcept;

		static void BindViewData(CommandContext& commandContext, const RenderView& pRenderView) noexcept;
		NO_DISCARD RenderView& BuildRenderView(const ViewRenderDesc& aViewRenderDesc) noexcept;

		void CreateRenderScene(const UUID& aUUID) noexcept;
		void CreateView(const UUID& aUUID) noexcept;

		void DestroyRenderScene(const UUID& aUUID) noexcept;
		void DestroyView(const UUID& aUUID) noexcept;
		static void Dispatch(Callback<void(Renderer*)>&& aCallback) noexcept;
		static void DrawScene(CommandContext& context, const RenderView& view, Batch::Blending blendMode) noexcept;
		static void DrawScene(CommandContext& context, Span<const Batch> batches, Batch::Blending blendMode) noexcept;
		
		NO_DISCARD bool ExistRenderScene(const UUID& aUUID) const noexcept;
		NO_DISCARD bool ExistView(const UUID& aUUID) const noexcept;

		NO_DISCARD GraphicsDevice* GetDevice() const noexcept;
		NO_DISCARD uint32 GetFrameIndex() const noexcept;
		NO_DISCARD RenderScene* GetRenderScene(const UUID& aUUID) const noexcept;

		template<typename InstanceType>
		void OnRequestBRDFLut(InstanceType* instance, AssetHandle(InstanceType::*method)()) noexcept
		{
			m_OnRequestBRDFLut = [instance, method]() { return (instance->*method)(); };
		}
		
		void Render() noexcept;
		CallbackID RegisterOnFrameRenderBeginCallback(Callback<void()> aFrameRenderBeginCallback) noexcept;
		CallbackID RegisterOnUploadCallback(Callback<void(CommandContext&)> aUploadCallback) noexcept;
		void RenderViews(const std::vector<ViewRenderDesc>& someRenderDescs) noexcept;

		static void SubmitBatch(CommandContext& aCommandContext, const Batch& aBatch) noexcept;
		static RenderJobHandle SubmitComputeJob(Callback<void(CommandContext&)>&& aCallback) noexcept;
		static RenderJobHandle SubmitRenderJob(Callback<void(CommandContext&)>&& aCallback) noexcept;

		void UnregisterOnFrameRenderBeginCallback(CallbackID aCallbackID) noexcept;
		void UnregisterOnUploadCallback(CallbackID aCallbackID) noexcept;
		
		NO_DISCARD SceneTextures& ValidateAndGetViewTextures(const ViewRenderDesc& aViewRenderDesc) noexcept;
		NO_DISCARD SceneBuffers& ValidateAndGetViewBuffers(const ViewRenderDesc& aViewRenderDesc) noexcept;

		Broadcaster<void(uint32 aReadbackResult, const UUID& aSceneUUID)> OnEntityIDReadbackDone;
	private:
		void GetViewUniforms(const RenderView& renderView, ShaderInterop::ViewUniforms& outViewUniform) noexcept;

		void InvokeDispatchRequests() noexcept;
		void InvokeEntityReadbackCallbacks() noexcept;
		void InvokeFrameBeginCallbacks() noexcept;
		void InvokeRenderJobs() noexcept;
		void InvokeUploadCallbacks() noexcept;

		void NotifySubsystemsOfBRDFLut_Temp() noexcept;

		void Render(RenderView& aRenderView, SceneTextures& aSceneTextures, SceneBuffers& aSceneBuffers, const Ref<Texture>& aTarget) noexcept;

		void SyncRingBuffer() noexcept;

		void UploadViewUniforms(CommandContext& commandContext, RenderView& view) noexcept;
	private:
		struct RenderJob
		{
			Callback<void(CommandContext&)> Callback;
			ERenderJobType Type = ERenderJobType::None;
			Ref<RenderJobState> State = nullptr;
		};

		std::unordered_map<CallbackID, Callback<void()>> m_OnFrameBeginCallbacks;
		std::unordered_map<CallbackID, Callback<void(CommandContext&)>> m_OnUploadCallbacks;

		std::unordered_map<UUID, UniquePtr<RenderScene>> m_RenderScenes;
		std::unordered_map<UUID, SceneTextures> m_ViewTextures;
		std::unordered_map<UUID, SceneTextureResources> m_ViewTextureResources;

		std::unordered_map<UUID, SceneBuffers> m_ViewBuffers;
		std::unordered_map<UUID, RenderView> m_RenderViews;

		std::vector<ViewRenderDesc> m_PendingViews;

		GraphicsDevice* m_pDevice = nullptr;

		inline static std::vector<Callback<void(Renderer*)>> s_EnqueuedRequests;
		inline static std::vector<RenderJob> s_EnqueuedRenderJobs;
		inline static std::vector<RenderJob> s_InProgressRenderJobs;
		
		uint32 m_Frame = 0u;

		/*
		* Techniques
		*/
		UniquePtr<ForwardOpaqueAlphaMask> m_pForwardOpaqueAlphaMask;
		UniquePtr<ForwardAlphaBlend> m_pForwardAlphaBlend;
		UniquePtr<EditorGrid> m_pEditorGrid;
		UniquePtr<PostProcessing> m_pPostProcessing;
		UniquePtr<DepthPrePass> m_pDepthPrePass;
		UniquePtr<HBAOPlus> m_pHBAOPlus;
		UniquePtr<Outlines> m_pOutlines;
		UniquePtr<AutoExposure> m_pAutoExposure;
		UniquePtr<SkyBoxRenderer> m_pSkyBoxRenderer;
		UniquePtr<Picking> m_pPicking;
		UniquePtr<ResolveDepthPass> m_pResolveDepthPass;

		AssetHandle m_BRDFLutTextureHandle = AssetHandle::INVALID;
		
		struct EntityIDReadbackData
		{
			SyncPoint Sync;
			UUID RenderSceneUUID;
			UUID ViewUUID;
		};
		
		std::queue<EntityIDReadbackData> m_EntityIDSyncDatas;

		Callback<AssetHandle()> m_OnRequestBRDFLut;

		inline static std::mutex s_RenderJobMutex;
		inline static std::mutex s_DispatchMutex;
		std::mutex m_OnFrameBeginMutex;
		std::mutex m_OnUploadMutex;
	};
}