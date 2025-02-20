#pragma once
#include "Graphics/RHI/RHI.h"
#include "RenderTypes.h"

#include "Graphics/Renderer/Techniques/ForwardRenderer.h"
#include "Graphics/Renderer/Techniques/ToyRenderer.h"


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

		void Render(Scene* pScene, const ViewTransform& cameraViewTransform, const GraphicsOptions& graphicsOptions, Ref<TextureEx> pTarget) noexcept;
	
		static void BindViewData(CommandContext& commandContext, const RenderView& pRenderView) noexcept;
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
		SceneBuffer m_InstancesBuffer;

		SceneTextures m_SceneTextures;

		uint32 m_Frame = 0u;

		/*
		* Techniques
		*/
		UniquePtr<ForwardRenderer> m_pForwardRenderer = nullptr;
		UniquePtr<ToyRenderer> m_pToyRenderer = nullptr;

	};
}