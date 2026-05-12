#pragma once

#include "Panels/ViewportPanel.h"

namespace Relentless
{
	class MaterialDetailsView;
	class Scene;

	class MaterialEditorPanel : public ViewportPanel
	{
	public:
		MaterialEditorPanel(const std::vector<AssetHandle>& someEnvironments) noexcept;
		virtual ~MaterialEditorPanel() override;

		NO_DISCARD ViewRenderDesc BuildRenderDescriptor() const noexcept override;
		NO_DISCARD Ref<VerticalBox> BuildWindowLayout() noexcept;

		NO_DISCARD virtual String GetDisplayName() const noexcept override;
		NO_DISCARD virtual String GetPersistKey() const noexcept override;
	protected:
		NO_DISCARD bool OnKeyPressedEvent(KeyPressedEvent& aEvent) noexcept override;
		NO_DISCARD bool OnMouseDragEvent(MouseDragEvent& aEvent) noexcept override;

		virtual void ResolveAndSetCameraMode() noexcept override;

		void Update() noexcept override;
	private:
		void CreatePreviewScene() noexcept;

		void OnMaterialEdited(MAYBE_UNUSED IAsset* aAsset, MAYBE_UNUSED uint64 aProperty) noexcept;
		void OnMaterialSaved(MAYBE_UNUSED IAsset* aAsset) noexcept;
	private:
		Ref<Scene> m_pPreviewScene;
		entity m_MaterialPreviewEntity = NULL_ENTITY;
		MaterialDetailsView* m_pMaterialDetailsView = nullptr;
	};
}