#pragma once

#include "Graphics/Renderer/Techniques/EditorGrid.h"
#include "Graphics/Renderer/Techniques/ForwardRenderer.h"
#include "Graphics/Renderer/Techniques/ToyRenderer.h"
#include "Graphics/RHI/RHI.h"
#include "Graphics/RHI/DescriptorHeap.h"
#include "RenderTypes.h"

namespace ShaderInterop
{
	struct ViewUniforms;
}

namespace Relentless
{
	class Scene;

	class Renderer
	{
	public:
		Renderer(GraphicsDevice* pDevice) noexcept;
		~Renderer() noexcept = default;

		static void BindViewData(CommandContext& commandContext, const RenderView& pRenderView) noexcept;
		static void DrawScene(CommandContext& context, const RenderView& view, Batch::Blending blendMode) noexcept;
		static void DrawScene(CommandContext& context, Span<const Batch> batches, Batch::Blending blendMode) noexcept;
		
		[[nodiscard]] Span<const Batch> GetBatches() const noexcept;
		void Render(Scene* pScene, ViewTransform* pViewTransform, const GraphicsOptions& graphicsOptions, Ref<TextureEx> pTarget) noexcept;
	private:
		void GetViewUniforms(const RenderView& renderView, ShaderInterop::ViewUniforms& outViewUniform) noexcept;
		void UploadSceneData(CommandContext& commandContext) noexcept;
		void UploadViewUniforms(CommandContext& commandContext, RenderView& view) noexcept;
	private:
		GraphicsDevice* m_pDevice = nullptr;
		RenderView m_MainView{};

		Scene* m_pCurrentScene = nullptr;

		SceneBuffer m_LightsBuffer;
		SceneBuffer m_MaterialsBuffer;
		SceneBuffer m_MeshesBuffer;
		SceneBuffer m_InstancesBuffer;

		SceneTextures m_SceneTextures;

		uint32 m_Frame = 0u;

		std::vector<Batch> m_Batches;

		/*
		* Techniques
		*/
		UniquePtr<ForwardRenderer> m_pForwardRenderer = nullptr;
		UniquePtr<EditorGrid> m_pEditorGrid = nullptr;
		UniquePtr<ToyRenderer> m_pToyRenderer = nullptr;

	};
}