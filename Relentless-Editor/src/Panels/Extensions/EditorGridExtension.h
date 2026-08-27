#pragma once
#include "UI/Viewport/IViewportExtension.h"

#include "Graphics/Techniques/EditorGrid.h"

namespace Relentless
{
	class CommandContext;
	class IDetailLayoutBuilder;
	struct RenderView;
	struct SceneTextures;
	class ViewportClient;

	class EditorGridExtension final : public IViewportExtension
	{
	public:
		void CustomizeGridDetails(IDetailLayoutBuilder& aBuilder);
		
		void OnRegistered(ViewportPanel& aViewportPanel) override;
		void OnUnregister() override;
	protected:
		void OnRender(CommandContext& aCommandContext, const RenderView& aRenderView, SceneTextures& aSceneTextures);
	private:
		UniquePtr<EditorGrid> m_pEditorGridPass;
		ViewportClient* m_pViewportClient = nullptr;
	};
}