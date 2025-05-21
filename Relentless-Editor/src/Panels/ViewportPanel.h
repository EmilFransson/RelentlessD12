#pragma once
#include "Panel.h"

#include "../Controller/PerspectiveCameraController.h"
#include "../Controller/TransformGizmoController.h"

namespace Relentless
{
	class Editor;

	enum class EViewportState : uint8 { None = 0u, Default, NavigatingScene, TransformingGizmo };

	class ViewportPanel : public PanelBase
	{
	public:
		ViewportPanel(const char* pName, ImGuiWindowFlags flags, Editor* pEditor, uint32 renderViewIndex) noexcept;
		virtual ~ViewportPanel() noexcept override = default;

		[[nodiscard]] std::shared_ptr<PerspectiveCamera> GetCamera() const noexcept;
		[[nodiscard]] uint32 GetRenderViewIndex() const noexcept;

		[[nodiscard]] const Vector2i& GetViewportSize() const noexcept;
		[[nodiscard]] Vector2i GetClientHoverCoordinates() const noexcept;
		[[nodiscard]] const Vector2u& GetClientScreenPosition() const noexcept;
		[[nodiscard]] bool IsClientAreaHovered() const noexcept;

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
		[[nodiscard]] bool CanHandleHotkeys() const noexcept;
		[[nodiscard]] bool CanHandleMouseInputs() const noexcept;
		void ConfineAndHideMouseAtCursorPosition() noexcept;

		void DetermineCameraAreaHoverState() noexcept;
		void DrawCameraValidClientAreaRect() noexcept;

		[[nodiscard]] bool HandleKeyPressed(RLS_Key key) noexcept;
		void HandleTransformGizmoInteraction() noexcept;

		[[nodiscard]] bool IsCameraValidClientAreaHovered() const noexcept;

		void OnCameraSpeedMultiplierChanged(float speed) noexcept;
		[[nodiscard]] float OnCameraSpeedMultiplierRequested() const noexcept;

		void OnCanvasHoverStateChanged(bool newState) noexcept;
		void OnCanvasResize(const Vector2i& newSize) noexcept;
		[[nodiscard]] Texture* OnCanvasTargetRequest() const noexcept;
		void OnCanvasRenderEnd() noexcept;

		[[nodiscard]] float OnCameraFarViewPlaneRequested() const noexcept;
		[[nodiscard]] float OnCameraNearViewPlaneRequested() const noexcept;
		void OnCameraFarViewPlaneChanged(float farPlane) noexcept;
		void OnCameraNearViewPlaneChanged(float nearPlane) noexcept;

		//Events:
		[[nodiscard]] bool OnKeyPressedEvent(KeyPressedEvent& event) noexcept override;
		
		[[nodiscard]] bool OnLeftMouseButtonPressedEvent(LeftMouseButtonPressedEvent& event) noexcept override;
		[[nodiscard]] bool OnLeftMouseButtonReleasedEvent(LeftMouseButtonReleasedEvent& event) noexcept override;
		
		[[nodiscard]] bool OnRightMouseButtonPressedEvent(RightMouseButtonPressedEvent& event) noexcept override;
		[[nodiscard]] bool OnRightMouseButtonReleasedEvent(RightMouseButtonReleasedEvent& event) noexcept override;

		[[nodiscard]] bool OnMiddleMouseButtonPressedEvent(MiddleMouseButtonPressedEvent& event) noexcept override;
		[[nodiscard]] bool OnMiddleMouseButtonReleasedEvent(MiddleMouseButtonReleasedEvent& event) noexcept override;

		[[nodiscard]] bool OnMouseWheelScrolledEvent(MouseWheelScrolledEvent& event) noexcept;

		void OnBeginViewportHover() noexcept;
		void OnEndViewportHover() noexcept;

		void OnBeginCameraValidAreaHover() noexcept;
		void OnEndCameraValidAreaHover() noexcept;

		void OnCameraBeginMove() noexcept;
		void OnCameraEndMove() noexcept;
		void OnFocusGained(PanelBase* pSelf) noexcept;
		void OnFocusLost(PanelBase* pSelf) noexcept;

		void OnHorizontalFOVChanged(float value) noexcept;
		[[nodiscard]] float OnHorizontalFOVRequested() const noexcept;

		void OnSettingsButtonClicked();

		void OnTransformGizmoInteractionStateChanged(ETransformGizmoInteractionState newState) noexcept;
		void OnViewportResize(const Vector2i& newSize);

		[[nodiscard]] float OnEV100Requested() const noexcept;
		void OnEV100Changed(float ev100) noexcept;

		void SetState(EViewportState newState) noexcept;
	private:
		String m_ToolbarID;

		IntRect m_CameraValidScreenRect;

		UniquePtr<TransformGizmoController> m_pTransformController = nullptr;
		UniquePtr<PerspectiveCameraController> m_pCameraController = nullptr;

		Vector2u m_ScreenPosition = Vector2u::Zero();
		Vector2i m_ViewportSize = Vector2i::Zero();

		Editor* m_pEditor = nullptr;
		std::shared_ptr<PerspectiveCamera> m_pCamera = nullptr;

		uint32 m_RenderViewIndex = std::numeric_limits<uint32>::max();
		bool m_ClientAreaHovered = false;
		bool m_CameraValidAreaHovered = false;
		bool m_ShowSettingsPanel = false;

		EViewportState m_CurrentState = EViewportState::None;

		Ref<HorizontalBox> m_pToolbarBox = nullptr;
		HorizontalBox* m_pCanvasHBox = nullptr;
		VerticalBox* m_pSettingsBox = nullptr;
		HorizontalBox* m_pCanvasAndSettingsBox = nullptr;
		Canvas* m_pCanvas = nullptr;
	};
}