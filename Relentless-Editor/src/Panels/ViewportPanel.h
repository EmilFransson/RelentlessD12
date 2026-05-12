#pragma once
#include <Relentless.h>

#include "Controller/PerspectiveCameraController.h"
#include "Controller/TransformGizmoController.h"

#include "UI/Widgets/Canvas.h"
#include "UI/Widgets/Panel.h"

namespace Relentless
{
	class Editor;
	class HorizontalBox;
	class ViewportDetailsView;
	class VerticalBox;

	class ViewportPanel : public PanelBase
	{
	public:
		ViewportPanel(const char* aTitle) noexcept;
		virtual ~ViewportPanel() noexcept override;

		NO_DISCARD virtual bool AcceptsMouseInput() const noexcept override;

		NO_DISCARD Ref<IBaseWidget> BuildDefaultCanvasWidget() noexcept;
		NO_DISCARD Ref<VerticalBox> BuildDefaultWindowLayout() noexcept;
		NO_DISCARD virtual ViewRenderDesc BuildRenderDescriptor() const noexcept = 0;

		NO_DISCARD SharedPtr<PerspectiveCamera> GetCamera() const noexcept;
		NO_DISCARD const UniquePtr<PerspectiveCameraController>& GetCameraController() const noexcept;

		NO_DISCARD const Vector2i& GetViewportSize() const noexcept;
		NO_DISCARD Vector2i GetClientHoverCoordinates() const noexcept;
		NO_DISCARD const Vector2u& GetClientScreenPosition() const noexcept;
		NO_DISCARD const UUID& GetUUID() const noexcept;

		NO_DISCARD bool IsClientAreaHovered() const noexcept;
		NO_DISCARD virtual bool IsViewportPanel() const noexcept override { return true; };

		Broadcaster<void(ViewportPanel* pPanel, RLS_Key pressedKey)> OnHotkeyPressed;
		Broadcaster<void(ViewportPanel* pPanel, Vector2u relativeMouseCoords)> OnClickedOnViewport;
	protected:
		//Events:
		NO_DISCARD bool OnKeyPressedEvent(KeyPressedEvent& aEvent) noexcept override;
		NO_DISCARD bool OnKeyReleasedEvent(KeyReleasedEvent& aEvent) noexcept override;

		NO_DISCARD bool OnLeftMouseButtonPressedEvent(LeftMouseButtonPressedEvent& aEvent) noexcept override;
		NO_DISCARD bool OnLeftMouseButtonReleasedEvent(LeftMouseButtonReleasedEvent& aEvent) noexcept override;

		NO_DISCARD bool OnRightMouseButtonPressedEvent(RightMouseButtonPressedEvent& aEvent) noexcept override;
		NO_DISCARD bool OnRightMouseButtonReleasedEvent(RightMouseButtonReleasedEvent& aEvent) noexcept override;

		NO_DISCARD bool OnMiddleMouseButtonPressedEvent(MiddleMouseButtonPressedEvent& aEvent) noexcept override;
		NO_DISCARD bool OnMiddleMouseButtonReleasedEvent(MiddleMouseButtonReleasedEvent& aEvent) noexcept override;

		NO_DISCARD bool OnMouseBeginDragEvent(MAYBE_UNUSED MouseBeginDragEvent& aEvent) noexcept override;
		NO_DISCARD bool OnMouseDragEvent(MouseDragEvent& aEvent) noexcept override;
		NO_DISCARD bool OnMouseEndDragEvent(MAYBE_UNUSED MouseEndDragEvent& aEvent) noexcept override;

		NO_DISCARD bool OnMouseWheelScrolledEvent(MouseWheelScrolledEvent& aEvent) noexcept override;

		virtual void ResolveAndSetCameraMode() noexcept;

		virtual void PreRender() noexcept override;
		void OnRender() noexcept override{};
		virtual void PostRender() noexcept override;

		virtual void Update() noexcept override;

	private:
		void ConfineAndHideMouseAtCursorPosition() noexcept;

		void DrawCameraValidClientAreaRect() noexcept;

		void HandleTransformGizmoInteraction() noexcept;

		NO_DISCARD bool IsCameraValidClientAreaHovered() const noexcept;

		void OnCanvasHoverStateChanged(bool newState) noexcept;
		void OnCanvasResize(const Vector2i& newSize) noexcept;
		NO_DISCARD Texture* OnCanvasTargetRequest() const noexcept;
		void OnCanvasRenderEnd() noexcept;
		void OnSettingsButtonClicked();
		void OnViewportResize(const Vector2i& newSize);

		void RecomputeCameraValidScreenRect() noexcept;
	protected:
		Ref<Texture> m_pRenderTarget = nullptr;
		Canvas* m_pCanvas = nullptr;
	private:
		UUID m_UUID;
		IntRect m_CameraValidScreenRect;
		PerspectiveCameraController::Input m_CameraInput;

		UniquePtr<TransformGizmoController> m_pTransformController = nullptr;
		UniquePtr<PerspectiveCameraController> m_pCameraController = nullptr;

		Vector2u m_ScreenPosition = Vector2u::Zero();
		Vector2i m_ViewportSize = Vector2i::Zero();

		std::shared_ptr<PerspectiveCamera> m_pCamera = nullptr;

		bool m_ClientAreaHovered = false;
		bool m_CameraIsActive = false;
		bool m_CameraDeactivatedThisFrame = false;

		ViewportDetailsView* m_pViewportDetailsView = nullptr;

		Ref<HorizontalBox> m_pToolbarBox = nullptr;
		VerticalBox* m_pSettingsBox = nullptr;
	};
}