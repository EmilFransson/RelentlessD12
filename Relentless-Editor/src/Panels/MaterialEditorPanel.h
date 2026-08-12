#pragma once
#include "Panels/SceneViewportPanel.h"

namespace Relentless
{
	class MaterialDetailsView;
	class Scene;

	class MaterialEditorPanel : public SceneViewportPanel
	{
	public:
		MaterialEditorPanel(const std::vector<AssetHandle>& someEnvironments) noexcept;
		virtual ~MaterialEditorPanel() override;

		NO_DISCARD virtual String GetDisplayName() const noexcept override;
		NO_DISCARD virtual String GetPersistKey() const noexcept override;
		NO_DISCARD Scene* GetViewportScene() const noexcept override;
	protected:
		NO_DISCARD ViewportClient::Desc CreateClientDesc() override;
		NO_DISCARD ViewportSidePanelDesc CreateSidePanelDesc() override;
		
		void ExtendSidePanel(Ref<VerticalBox>& aSidePanelBox) override;
		
		void OnInitialized() override;
		NO_DISCARD bool OnKeyPressedEvent(KeyPressedEvent& aEvent) noexcept override;
		NO_DISCARD bool OnMouseDragEvent(MouseDragEvent& aEvent) noexcept override;

		void Update() noexcept override;
	private:
		void CreatePreviewScene() noexcept;

		void OnMaterialEdited(MAYBE_UNUSED IAsset* aAsset, MAYBE_UNUSED uint64 aProperty) noexcept;
		void OnMaterialSaved(MAYBE_UNUSED IAsset* aAsset) noexcept;
	private:
		Ref<Scene> m_pPreviewScene;
		Ref<MaterialDetailsView> m_pMaterialDetailsView = nullptr;
		entity m_MaterialPreviewEntity = NULL_ENTITY;
	};
}