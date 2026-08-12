#pragma once
#include "UI/Views/Details/EnvironmentDetailsView.h"

#include "Panels/SceneViewportPanel.h"

namespace Relentless
{
	class Scene;

	class EnvironmentEditorPanel : public SceneViewportPanel
	{
	public:
		EnvironmentEditorPanel(const std::vector<AssetHandle>& someEnvironments) noexcept;
		virtual ~EnvironmentEditorPanel() override;

		NO_DISCARD virtual String GetDisplayName() const noexcept override;
		NO_DISCARD virtual String GetPersistKey() const noexcept override;
		NO_DISCARD Scene* GetViewportScene() const noexcept override;
	protected:
		NO_DISCARD ViewportSidePanelDesc CreateSidePanelDesc() override;
		
		void ExtendSidePanel(Ref<VerticalBox>& aSidePanelBox) override;
		
		void OnInitialized() override;
		NO_DISCARD bool OnKeyPressedEvent(KeyPressedEvent& aEvent) noexcept override;

		void Update() noexcept override;
	private:
		void CreatePreviewScene() noexcept;

		NO_DISCARD Reply OnCanvasDragOver(MAYBE_UNUSED const WidgetGeometry& aWidgetGeometry, const Ref<DragDropOperationBase>& aDragDropOperation) noexcept;
		NO_DISCARD Reply OnDropOnCanvas(MAYBE_UNUSED const WidgetGeometry& aWidgetGeometry, const Ref<DragDropOperationBase>& aDragDropOperation) noexcept;
		void OnEnvironmentEdited(MAYBE_UNUSED IAsset* aAsset, MAYBE_UNUSED uint64 aProperty) noexcept;
		void OnEnvironmentSaved(MAYBE_UNUSED IAsset* aAsset) noexcept;
	private:
		Ref<Scene> m_pPreviewScene;
		entity m_EnvironmentPreviewEntity = NULL_ENTITY;
		Ref<EnvironmentDetailsView> m_pEnvironmentDetailsView = nullptr;
	};
}