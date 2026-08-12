#pragma once
#include "Controller/TransformGizmoController.h"

#include "UI/Viewport/IViewportExtension.h"

namespace Relentless
{
	class Button;

	class TransformGizmoViewportExtension final : public IViewportExtension
	{
	public:
		void OnRegistered(const ViewportPanel& aViewportPanel) override;
	protected:
		void ExtendToolbar(ViewportToolbarSlots& aSlots) override;

		NO_DISCARD bool HandleInput(const ViewportInputEvent& aInputEvent) override;

		void OnCanvasRenderEnd() override;

		NO_DISCARD bool WantsExclusiveInput() const noexcept override;
	private:
		void BuildSnapMenu(Span<const float> someValues, Callback<float()>&& aGetter, Callback<void(float)>&& aSetter, StringView aSectionName, StringView aBaseTooltip, StringView aOptionalSuffix, Button* aLabelButton) noexcept;

		bool HasSelection() const noexcept;

		void OnToggleGizmoTranslationMovementModeButtonClicked();
		void OnTransformGizmoModeToggle();
		void OnTransformGizmoSelectModeSelected();
		void OnTransformGizmoTranslateModeSelected();
		void OnTransformGizmoRotateModeSelected();
		void OnTransformGizmoScaleModeSelected();

		void OnTranslationSnapValuesButtonClicked();
		void OnRotationSnapValuesButtonClicked();
		void OnScaleSnapValuesButtonClicked();
	private:
		TransformGizmoController m_TransformGizmoController;
		const ViewportPanel* m_pViewportPanel = nullptr;

		Button* m_pTransformGizmoSelectButton = nullptr;
		Button* m_pTransformGizmoTranslateButton = nullptr;
		Button* m_pTransformGizmoRotateButton = nullptr;
		Button* m_pTransformGizmoScaleButton = nullptr;
		Button* m_pTransformGizmoModeButton = nullptr;
		Button* m_pTransformGizmoTranslationMovementModeButton = nullptr;
		Button* m_pTransformGizmoTranslationSnapValuesButton = nullptr;
		Button* m_pTransformGizmoRotationSnapValuesButton = nullptr;
		Button* m_pTransformGizmoScaleSnapValuesButton = nullptr;
		Button* m_pTransformGizmoRotationMovementModeButton = nullptr;
		Button* m_pTransformGizmoScaleMovementModeButton = nullptr;
	};
}