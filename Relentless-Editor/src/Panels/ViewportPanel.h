#pragma once
#include <Relentless.h>

#include <Controller/PerspectiveCameraController.h>
#include <Controller/TransformGizmoController.h>

#include <UI/Widgets/Canvas.h>
#include <UI/Widgets/HorizontalBox.h>
#include <UI/Widgets/VerticalBox.h>
#include <UI/Widgets/Panel.h>

namespace Relentless
{
	class Editor;

	enum class EViewportState : uint8 { None = 0u, Default, NavigatingScene, TransformingGizmo };

	class ViewportPanel : public PanelBase
	{
	public:
		ViewportPanel(uint32 aRenderViewIndex) noexcept;
		virtual ~ViewportPanel() noexcept override = default;

		NO_DISCARD std::shared_ptr<PerspectiveCamera> GetCamera() const noexcept;
		NO_DISCARD uint32 GetRenderViewIndex() const noexcept;

		NO_DISCARD const Vector2i& GetViewportSize() const noexcept;
		NO_DISCARD Vector2i GetClientHoverCoordinates() const noexcept;
		NO_DISCARD const Vector2u& GetClientScreenPosition() const noexcept;
		NO_DISCARD bool IsClientAreaHovered() const noexcept;

		Broadcaster<void(ViewportPanel* pPanel, RLS_Key pressedKey)> OnHotkeyPressed;
		Broadcaster<void(ViewportPanel* pPanel, Vector2u relativeMouseCoords)> OnClickedOnViewport;

		Broadcaster<void(ViewportPanel* pPanel)> OnMouseEnterViewport;
		Broadcaster<void(ViewportPanel* pPanel)> OnMouseExitViewport;

	protected:
		virtual void PreRender() noexcept override;
		void OnRender() noexcept override {};
		virtual void PostRender() noexcept override;

		virtual void Update() noexcept override;
	private:
		NO_DISCARD bool CanHandleHotkeys() const noexcept;
		NO_DISCARD bool CanHandleMouseInputs() const noexcept;
		void ConfineAndHideMouseAtCursorPosition() noexcept;

		void DetermineCameraAreaHoverState() noexcept;
		void DrawCameraValidClientAreaRect() noexcept;

		NO_DISCARD bool HandleKeyPressed(RLS_Key key) noexcept;
		void HandleTransformGizmoInteraction() noexcept;

		NO_DISCARD bool IsCameraValidClientAreaHovered() const noexcept;

		void OnCameraSpeedMultiplierChanged(float speed) noexcept;
		NO_DISCARD float OnCameraSpeedMultiplierRequested() const noexcept;

		void OnCanvasHoverStateChanged(bool newState) noexcept;
		void OnCanvasResize(const Vector2i& newSize) noexcept;
		NO_DISCARD Texture* OnCanvasTargetRequest() const noexcept;
		void OnCanvasRenderEnd() noexcept;

		NO_DISCARD float OnCameraFarViewPlaneRequested() const noexcept;
		NO_DISCARD float OnCameraNearViewPlaneRequested() const noexcept;
		void OnCameraFarViewPlaneChanged(float farPlane) noexcept;
		void OnCameraNearViewPlaneChanged(float nearPlane) noexcept;

		//Events:
		NO_DISCARD bool OnKeyPressedEvent(KeyPressedEvent& event) noexcept override;
		
		NO_DISCARD bool OnLeftMouseButtonPressedEvent(LeftMouseButtonPressedEvent& event) noexcept override;
		NO_DISCARD bool OnLeftMouseButtonReleasedEvent(LeftMouseButtonReleasedEvent& event) noexcept override;
		
		NO_DISCARD bool OnRightMouseButtonPressedEvent(RightMouseButtonPressedEvent& event) noexcept override;
		NO_DISCARD bool OnRightMouseButtonReleasedEvent(RightMouseButtonReleasedEvent& event) noexcept override;

		NO_DISCARD bool OnMiddleMouseButtonPressedEvent(MiddleMouseButtonPressedEvent& event) noexcept override;
		NO_DISCARD bool OnMiddleMouseButtonReleasedEvent(MiddleMouseButtonReleasedEvent& event) noexcept override;

		NO_DISCARD bool OnMouseWheelScrolledEvent(MouseWheelScrolledEvent& event) noexcept override;

		void OnBeginViewportHover() noexcept;
		void OnEndViewportHover() noexcept;

		void OnBeginCameraValidAreaHover() noexcept;
		void OnEndCameraValidAreaHover() noexcept;

		void OnCameraBeginMove() noexcept;
		void OnCameraEndMove() noexcept;
		void OnFocusGained(PanelBase* pSelf) noexcept;
		void OnFocusLost(PanelBase* pSelf) noexcept;

		void OnHorizontalFOVChanged(float value) noexcept;
		NO_DISCARD float OnHorizontalFOVRequested() const noexcept;

		void OnSettingsButtonClicked();

		void OnTransformGizmoInteractionStateChanged(ETransformGizmoInteractionState newState) noexcept;
		void OnViewportResize(const Vector2i& newSize);

		void SetState(EViewportState newState) noexcept;
	private:
		IntRect m_CameraValidScreenRect;

		UniquePtr<TransformGizmoController> m_pTransformController = nullptr;
		UniquePtr<PerspectiveCameraController> m_pCameraController = nullptr;

		Vector2u m_ScreenPosition = Vector2u::Zero();
		Vector2i m_ViewportSize = Vector2i::Zero();

		std::shared_ptr<PerspectiveCamera> m_pCamera = nullptr;

		uint32 m_RenderViewIndex = std::numeric_limits<uint32>::max();
		bool m_ClientAreaHovered = false;
		bool m_CameraValidAreaHovered = false;
		bool m_ShowSettingsPanel = false;

		EViewportState m_CurrentState = EViewportState::None;

		Ref<HorizontalBox> m_pToolbarBox = nullptr;
		HorizontalBox* m_pCanvasHBox = nullptr;
		VerticalBox* m_pSettingsBox = nullptr;
		Canvas* m_pCanvas = nullptr;
	};
}