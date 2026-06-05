#pragma once
#include "Panel.h"

#include "UI/DragDrop/DragDropOperation.h"

namespace Relentless
{
	class AssetView;
	class Button;
	class HorizontalBox;
	class Label;
	class PathView;
	class Separator;
	class VerticalBox;
	struct WidgetGeometry;

	struct ContentBrowserLayout
	{
		VerticalBox* LeftTopBox = nullptr;
		VerticalBox* LeftBottomBox = nullptr;
		VerticalBox* MiddleBox = nullptr;
		VerticalBox* RightTopBox = nullptr;
		VerticalBox* RightBottomBox = nullptr;
	};

	class ContentBrowserPanel : public PanelBase
	{
	public:
		ContentBrowserPanel() noexcept;
		virtual ~ContentBrowserPanel() noexcept;
		
		NO_DISCARD virtual String GetDisplayName() const noexcept override;
		NO_DISCARD virtual String GetPersistKey() const noexcept override;
	private:
		NO_DISCARD Ref<Button> BuildAddAssetButton() noexcept;
		NO_DISCARD Ref<Button> BuildImportAssetButton() noexcept;
		NO_DISCARD ContentBrowserLayout BuildLayout(HorizontalBox* aRoot) noexcept;
		NO_DISCARD Ref<HorizontalBox> BuildNavigation() noexcept;
		NO_DISCARD Ref<HorizontalBox> BuildPathBreadcrumb() noexcept;
		NO_DISCARD Ref<Button> BuildSettingsButton() noexcept;
		NO_DISCARD Ref<VerticalBox> BuildToolbar() noexcept;

		void OnAddAssetButtonClicked() noexcept;
		NO_DISCARD bool OnFilterRoots(EAssetSourceType aAssetSourceType, MAYBE_UNUSED const String& aVirtualPath) noexcept;
		void OnImportAssetButtonClicked() noexcept;
		void OnNavigateBackButtonClicked() noexcept;
		void OnNavigateForwardButtonClicked() noexcept;
		void OnPathViewSelectionChanged() noexcept;
		void OnSettingsButtonClicked() noexcept;

		void PopulateBreadcrumb() noexcept;
	
		void RecordHistory(const std::vector<String>& somePaths) noexcept;

		void UpdateAddAssetButton() noexcept;
		void UpdateImportAssetButton() noexcept;
		void UpdateItemsLabel() noexcept;
		void UpdateNavigationButtons() noexcept;
	protected:
		NO_DISCARD virtual bool AcceptsMouseInput() const noexcept override;

		NO_DISCARD bool OnLeftMouseButtonPressedEvent(MAYBE_UNUSED LeftMouseButtonPressedEvent& aLeftMouseButtonPressedEvent) noexcept override;
		NO_DISCARD bool OnLeftMouseButtonReleasedEvent(MAYBE_UNUSED LeftMouseButtonReleasedEvent& aLeftMouseButtonReleasedEvent) noexcept override;
		NO_DISCARD bool OnMouseDragEvent(MAYBE_UNUSED MouseDragEvent& aMouseDragEvent) noexcept override;
	private:
		std::vector<String> m_BreadcrumbForwardStack;
		std::vector<String> m_BreadcrumbBackwardStack;
		String m_CurrentPath;

		AssetView* m_pAssetsView = nullptr;
		PathView* m_pPathView = nullptr;
		Ref<HorizontalBox> m_pBreadcrumbBox = nullptr;
		VerticalBox* m_pPathViewBox = nullptr;
		Button* m_pAddAssetButton = nullptr;
		Button* m_pImportAssetButton = nullptr;
		Button* m_pNavigateBackButton = nullptr;
		Button* m_pNavigateForwardButton = nullptr;
		Label* m_pItemsLabel = nullptr;
		Separator* m_pViewSeparator = nullptr;

		float m_DragStartWidth = 0.0f;
		float m_DragStartMouseX = 0.0f;

		bool m_SuppressHistoryRecording = false;
		bool m_ShowEngineContent = true;
		bool m_DraggingViewSeparator = false;
	};
}