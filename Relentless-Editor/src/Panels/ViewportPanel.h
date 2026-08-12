#pragma once
#include <Relentless.h>

#include "Panel.h"

#include "UI/Viewport/ViewportClient.h"
#include "UI/Viewport/ViewportInputEvent.h"
#include "UI/Viewport/IViewportExtension.h"
#include "UI/Viewport/ViewportSurface.h"
#include "UI/Viewport/ViewportSidePanelDesc.h"
#include "UI/Viewport/ViewportToolbarDesc.h"

namespace Relentless
{
	class Canvas;
	class HorizontalBox;
	class VerticalBox;

	struct ViewportToolbarSlots
	{
		HorizontalBox* Left = nullptr;
		HorizontalBox* Right = nullptr;
	};

	class ViewportPanel : public PanelBase
	{
	public:
		ViewportPanel(const char* aTitle) noexcept;
		virtual ~ViewportPanel() noexcept override;

		NO_DISCARD virtual bool AcceptsMouseInput() const noexcept override;

		NO_DISCARD Ref<IBaseWidget> BuildDefaultCanvasWidget() noexcept;
		NO_DISCARD virtual ViewRenderDesc BuildRenderDescriptor() const noexcept;

		NO_DISCARD Canvas* GetCanvas() const noexcept;
		NO_DISCARD ViewportClient& GetClient() noexcept;
		NO_DISCARD const ViewportClient& GetClient() const noexcept;
		NO_DISCARD ViewportSurface& GetSurface() noexcept;
		NO_DISCARD const ViewportSurface& GetSurface() const noexcept;
		NO_DISCARD Vector2u GetViewportSize() const noexcept;
		NO_DISCARD Vector2i GetClientHoverCoordinates() const noexcept;
		NO_DISCARD const Vector2u& GetClientScreenPosition() const noexcept;
		NO_DISCARD const UUID& GetUUID() const noexcept;

		void Initialize();

		NO_DISCARD bool IsClientAreaHovered() const noexcept;
		NO_DISCARD virtual bool IsViewportPanel() const noexcept override { return true; }

		template<typename ExtensionType, typename ...Args>
		ExtensionType& RegisterExtension(Args&&... args);
		
		virtual void RegisterExtensions() {};

		Broadcaster<void(ViewportPanel* pPanel, RLS_Key pressedKey)> OnHotkeyPressed;
		Broadcaster<void(ViewportPanel* pPanel, Vector2u relativeMouseCoords)> OnClickedOnViewport;
	protected:
		NO_DISCARD virtual Ref<VerticalBox> BuildWindowLayout() noexcept;

		NO_DISCARD virtual UniquePtr<ViewportClient> CreateClient();
		NO_DISCARD virtual ViewportClient::Desc CreateClientDesc();
		NO_DISCARD virtual UniquePtr<ViewportSurface> CreateSurface();
		NO_DISCARD virtual ViewportSurface::Desc CreateSurfaceDesc();
		NO_DISCARD virtual ViewportToolbarDesc CreateToolbarDesc();
		NO_DISCARD virtual ViewportSidePanelDesc CreateSidePanelDesc();

		virtual void ExtendToolbar(ViewportToolbarSlots&) {};
		virtual void ExtendSidePanel(Ref<VerticalBox>&) {};

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

		virtual void OnInitialized() {}

		virtual void PreRender() noexcept override;
		void OnRender() noexcept override{};
		virtual void PostRender() noexcept override;

		virtual void Update() noexcept override;
	private:
		NO_DISCARD Ref<IBaseWidget> BuildToolbarWidget();
		NO_DISCARD Ref<VerticalBox> BuildSidePanelWidget();

		void BroadcastEvent(const ViewportInputEvent& aInputEvent);

		void ConfineAndHideMouseAtCursorPosition() noexcept;

		NO_DISCARD bool IsCameraValidClientAreaHovered() const noexcept;

		NO_DISCARD ViewportInputEvent MakeInputEvent(EViewportInputType aInputType) const noexcept;

		void OnCanvasHoverStateChanged(bool newState) noexcept;
		void OnCanvasResize(const Vector2i& newSize) noexcept;
		NO_DISCARD Texture* OnCanvasTargetRequest() const noexcept;
		void OnCanvasRenderEnd() noexcept;
		void OnFocusLost(MAYBE_UNUSED PanelBase* aPanelBase) noexcept;

		void RecomputeCameraValidScreenRect() noexcept;
		NO_DISCARD bool RouteInput(const ViewportInputEvent& aInputEvent);

		void UpdateCursorCapture();
	private:
		std::vector<UniquePtr<IViewportExtension>> m_Extensions;

		UUID m_UUID;

		IntRect m_CameraValidScreenRect;
		Vector2u m_ScreenPosition = Vector2u::Zero();

		ViewportToolbarSlots m_ToolbarSlots;

		UniquePtr<ViewportSurface> m_pSurface = nullptr;
		UniquePtr<ViewportClient> m_pClient = nullptr;
		Canvas* m_pCanvas = nullptr;

		bool m_ClientAreaHovered = false;
		bool m_CursorCaptured = false;
		bool m_OwnsCurrentPress = false;
	};

	template<typename ExtensionType, typename ...Args>
	ExtensionType& ViewportPanel::RegisterExtension(Args&&... args)
	{
		static_assert(std::is_base_of_v<IViewportExtension, ExtensionType>, "[ViewportPanel::RegisterExtension] Extension must inherit from IViewportExtension");
		
		UniquePtr<ExtensionType> pExtension = MakeUnique<ExtensionType>(std::forward<Args>(args)...);
		ExtensionType& reference = *pExtension;

		m_Extensions.push_back(std::move(pExtension));
		reference.OnRegistered(*this);

		return reference;
	}
}