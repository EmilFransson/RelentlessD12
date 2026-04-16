#pragma once

#include "Assets/AssetMeta.h"

#include "Core/DLLExport.h"

#include "Callback/Callback.h"
#include "Callback/Broadcaster.h"

#include "Graphics/Renderer/Techniques/AutoExposure.h"
#include "Graphics/Renderer/Techniques/DepthPrePass.h"
#include "Graphics/Renderer/Techniques/EditorGrid.h"
#include "Graphics/Renderer/Techniques/ForwardRenderer.h"
#include "Graphics/Renderer/Techniques/HBAOPlus.h"
#include "Graphics/Renderer/Techniques/Outlines.h"
#include "Graphics/Renderer/Techniques/PostProcessing.h"
#include "Graphics/Renderer/Techniques/SkyBoxRenderer.h"
#include "Graphics/RHI/RHI.h"
#include "Graphics/RHI/DescriptorHeap.h"

#include "RenderTypes.h"

#include "Subsystem/ISystemManager.h"

namespace ShaderInterop
{
	struct ViewUniforms;
}

namespace Relentless
{
	class Scene;

	enum class ERenderJobType : uint8 { None = 0u, Raster, Compute };

	class RLS_API Renderer : public ISystemManager
	{
	public:
		Renderer(GraphicsDevice* pDevice) noexcept;
		~Renderer() noexcept = default;

		static void BindViewData(CommandContext& commandContext, const RenderView& pRenderView) noexcept;
		
		static void Dispatch(Callback<void(Renderer*)>&& aCallback) noexcept;
		static void DrawScene(CommandContext& context, const RenderView& view, Batch::Blending blendMode) noexcept;
		static void DrawScene(CommandContext& context, Span<const Batch> batches, Batch::Blending blendMode) noexcept;
		
		NO_DISCARD Span<const Batch> GetBatches() const noexcept;
		NO_DISCARD GraphicsDevice* GetDevice() const noexcept;
		NO_DISCARD uint32 GetFrameIndex() const noexcept;
		void Render(Scene* pScene, ViewTransform* pViewTransform, MAYBE_UNUSED const GraphicsOptions& graphicsOptions, Ref<Texture> pTarget) noexcept;
		
		Broadcaster<void(uint32 readbackResult)> OnEntityIDReadbackDone;

		template<typename InstanceType>
		void OnRequestBRDFLut(InstanceType* instance, AssetHandle(InstanceType::*method)()) noexcept
		{
			m_OnRequestBRDFLut = [instance, method]() { return (instance->*method)(); };
		}
		
		CallbackID RegisterOnFrameRenderBeginCallback(Callback<void()> aFrameRenderBeginCallback) noexcept;
		CallbackID RegisterOnUploadCallback(Callback<void(CommandContext&)> aUploadCallback) noexcept;

		static RenderJobHandle SubmitComputeJob(Callback<void(CommandContext&)>&& aCallback) noexcept;
		static RenderJobHandle SubmitRenderJob(Callback<void(CommandContext&)>&& aCallback) noexcept;

		void UnregisterOnFrameRenderBeginCallback(CallbackID aCallbackID) noexcept;
		void UnregisterOnUploadCallback(CallbackID aCallbackID) noexcept;
	private:
		void GetViewUniforms(const RenderView& renderView, ShaderInterop::ViewUniforms& outViewUniform) noexcept;
		void InitializePipelines();

		void UploadSceneData(CommandContext& commandContext) noexcept;
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

		GraphicsDevice* m_pDevice = nullptr;
		RenderView m_MainView{};

		Scene* m_pCurrentScene = nullptr;

		SceneBuffer m_LightsBuffer;
		SceneBuffer m_MaterialsBuffer;
		SceneBuffer m_MeshesBuffer;
		SceneBuffer m_InstancesBuffer;
		SceneBuffer m_EnvironmentBuffer;

		SceneTextures m_SceneTextures;

		uint32 m_Frame = 0u;

		inline static std::vector<Callback<void(Renderer*)>> s_EnqueuedRequests;
		inline static std::vector<RenderJob> s_EnqueuedRenderJobs;
		inline static std::vector<RenderJob> s_InProgressRenderJobs;
		std::vector<Batch> m_Batches;

		Ref<Texture> m_pColortarget = nullptr;
		Ref<Texture> m_pDepthTarget = nullptr;
		Ref<Texture> m_pEntityIDTexture = nullptr;
		Ref<Texture> m_pEntityIDDepthTarget = nullptr;

		/*
		* Techniques
		*/
		UniquePtr<ForwardRenderer> m_pForwardRenderer = nullptr;
		UniquePtr<EditorGrid> m_pEditorGrid = nullptr;
		UniquePtr<PostProcessing> m_pPostProcessing = nullptr;
		UniquePtr<DepthPrePass> m_pDepthPrePass = nullptr;
		UniquePtr<HBAOPlus> m_pHBAOPlus = nullptr;
		UniquePtr<Outlines> m_pOutlines = nullptr;
		UniquePtr<AutoExposure> m_pAutoExposure = nullptr;
		UniquePtr<SkyBoxRenderer> m_pSkyBoxRenderer = nullptr;

		AssetHandle m_BRDFLutTextureHandle = AssetHandle::INVALID;
		
		Ref<Buffer> m_EntityIDReadbackBuffer = nullptr;
		
		Ref<PipelineState> m_pEntityIdPSO = nullptr;
		std::queue<SyncPoint> m_EntityIDSyncs;

		Callback<AssetHandle()> m_OnRequestBRDFLut;

		inline static std::mutex s_RenderJobMutex;
		inline static std::mutex s_DispatchMutex;
		std::mutex m_OnFrameBeginMutex;
		std::mutex m_OnUploadMutex;
	};
}