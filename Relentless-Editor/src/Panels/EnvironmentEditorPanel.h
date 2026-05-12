#pragma once
#include "UI/Views/Details/EnvironmentDetailsView.h"

#include "Panels/ViewportPanel.h"

namespace Relentless
{
	class Scene;

	class EnvironmentEditorPanel : public ViewportPanel
	{
	public:
		EnvironmentEditorPanel(const std::vector<AssetHandle>& someEnvironments) noexcept;
		virtual ~EnvironmentEditorPanel() override;

		NO_DISCARD ViewRenderDesc BuildRenderDescriptor() const noexcept override;
		NO_DISCARD Ref<VerticalBox> BuildWindowLayout() noexcept;
		
		NO_DISCARD virtual String GetDisplayName() const noexcept override;
		NO_DISCARD virtual String GetPersistKey() const noexcept override;
	protected:
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
		EnvironmentDetailsView* m_pEnvironmentDetailsView = nullptr;
	};
}