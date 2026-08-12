#include "ViewportPanel.h"
#include "Core/Editor.h"

#include "UI/Widgets/Button.h"
#include "UI/Widgets/Canvas.h"
#include "UI/Widgets/HorizontalBox.h"
#include "UI/Widgets/Spacer.h"
#include "UI/Widgets/VerticalBox.h"

namespace Relentless
{
	ViewportPanel::ViewportPanel(const char* aTitle) noexcept
		:PanelBase(aTitle, ImGuiWindowFlags_None),
		 m_UUID{ CreateUUID() }
	{
		SetPadding(Vector2(0.0f, 0.0f));
		OnLostFocus.Connect(this, &ViewportPanel::OnFocusLost);
	}

	ViewportPanel::~ViewportPanel() noexcept = default;

	bool ViewportPanel::AcceptsMouseInput() const noexcept
	{
		return IsCameraValidClientAreaHovered();
	}

	Ref<IBaseWidget> ViewportPanel::BuildDefaultCanvasWidget() noexcept
	{
		HorizontalBox* pCanvasBox = RLS_NEW HorizontalBox();
		pCanvasBox->SetHorizontalSizePolicy(ESizePolicy::Stretch);
		pCanvasBox->SetVerticalSizePolicy(ESizePolicy::Stretch);

		m_pCanvas = pCanvasBox->AddWidget(RLS_NEW Canvas());
		m_pCanvas
			->Target(this, &ViewportPanel::OnCanvasTargetRequest)
			->OnHoverStateChanged(this, &ViewportPanel::OnCanvasHoverStateChanged)
			->OnResize(this, &ViewportPanel::OnCanvasResize)
			->OnRenderEnd.Connect(this, &ViewportPanel::OnCanvasRenderEnd);

		m_pCanvas->SetHorizontalSizePolicy(ESizePolicy::Stretch);
		m_pCanvas->SetVerticalSizePolicy(ESizePolicy::Stretch);

		return pCanvasBox;
	}

	ViewRenderDesc ViewportPanel::BuildRenderDescriptor() const noexcept
	{
		const PerspectiveCamera& camera = m_pClient->GetCamera();

		ViewRenderDesc renderDesc;
		renderDesc.ViewTransform = camera.GetViewTransform();
		renderDesc.ViewID = GetUUID();
		renderDesc.RenderFeatures = m_pClient->GetRenderFeatures();
		renderDesc.RenderQualitySettings = m_pClient->GetRenderQualitySettings();
		renderDesc.RenderViewMode = m_pClient->GetViewMode();
		renderDesc.MouseHoverCoordinates = IsClientAreaHovered() ? GetClientHoverCoordinates() : Vector2i(-1, -1);
		renderDesc.RenderTarget = m_pSurface->GetTexture();

		const Vector2u& region = m_pSurface->GetSize();
		renderDesc.ViewTransform.Viewport = FloatRect(0.0f, 0.0f, Math::Max(1.0f, (float)region.x), Math::Max(1.0f, (float)region.y));

		return renderDesc;
	}

	Canvas* ViewportPanel::GetCanvas() const noexcept
	{
		return m_pCanvas;
	}

	ViewportClient& ViewportPanel::GetClient() noexcept
	{
		return *m_pClient;
	}

	const ViewportClient& ViewportPanel::GetClient() const noexcept
	{
		return *m_pClient;
	}

	const ViewportSurface& ViewportPanel::GetSurface() const noexcept
	{
		return *m_pSurface;
	}

	ViewportSurface& ViewportPanel::GetSurface() noexcept
	{
		return *m_pSurface;
	}

	Vector2u ViewportPanel::GetViewportSize() const noexcept
	{
		return m_pSurface->GetSize();
	}

	Vector2i ViewportPanel::GetClientHoverCoordinates() const noexcept
	{
		if (!IsClientAreaHovered())
			return Vector2i(-1, -1);

		const Vector2u& windowPos = GetClientScreenPosition();
		const Vector2u mouseScreenPosition = Mouse::GetCursorScreenPosition();

		// Compute relative coordinates by subtracting window top-left position
		const Vector2i clientPosition = Vector2i(mouseScreenPosition.x - windowPos.x, mouseScreenPosition.y - windowPos.y);
		if (clientPosition.x < 0 || clientPosition.y < 0)
			return Vector2i(-1, -1);

		return clientPosition;
	}

	const Vector2u& ViewportPanel::GetClientScreenPosition() const noexcept
	{
		return m_ScreenPosition;
	}

	const UUID& ViewportPanel::GetUUID() const noexcept
	{
		return m_UUID;
	}

	void ViewportPanel::Initialize()
	{
		m_pSurface = CreateSurface();
		m_pClient = CreateClient();

		RegisterExtensions();
		SetRoot(BuildWindowLayout());
		OnInitialized();
	}

	bool ViewportPanel::IsClientAreaHovered() const noexcept
	{
		return m_ClientAreaHovered;
	}

	void ViewportPanel::PreRender() noexcept
	{
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
	}

	void ViewportPanel::PostRender() noexcept
	{
		ImGui::PopStyleVar();
	}

	void ViewportPanel::Update() noexcept
	{
		RecomputeCameraValidScreenRect();

		if (!Mouse::IsButtonDown(RLS_Button::Left) && !Mouse::IsButtonDown(RLS_Button::Right) && !Mouse::IsButtonDown(RLS_Button::Wheel))
			m_OwnsCurrentPress = false;

		const float deltaTime = Time::GetDeltaTime();

		bool exclusiveInput = false;
		for (const auto& pExtension : m_Extensions)
			exclusiveInput |= pExtension->WantsExclusiveInput();

		const bool canNavigate = m_OwnsCurrentPress && !exclusiveInput && (m_pClient->IsNavigating() || IsCameraValidClientAreaHovered());

		m_pClient->SetNavigationEnabled(canNavigate);
		m_pClient->Update(deltaTime);

		for (auto& pExtension : m_Extensions)
			pExtension->OnUpdate(deltaTime);

		m_pSurface->Flush();
		UpdateCursorCapture();
	}

	Ref<IBaseWidget> ViewportPanel::BuildToolbarWidget()
	{
		const ViewportToolbarDesc desc = CreateToolbarDesc();
		if (!desc.IsEnabled())
			return nullptr;

		HorizontalBox* pBar = RLS_NEW HorizontalBox();
		pBar->SetHorizontalSizePolicy(ESizePolicy::Stretch);
		pBar->SetVerticalSizePolicy(ESizePolicy::Fixed);
		pBar->SetSize(Vector2(-1.0f, desc.Height));
		pBar->SetMargin(FloatRect::Uniform(desc.Margin));

		HorizontalBox* pLeft = pBar->AddWidget(RLS_NEW HorizontalBox());
		pLeft->SetSpacing(desc.GroupSpacing);

		pBar->AddWidget(RLS_NEW Spacer())->SetHorizontalSizePolicy(ESizePolicy::Stretch);
		
		HorizontalBox* pRight = pBar->AddWidget(RLS_NEW HorizontalBox());
		pRight->SetSpacing(desc.GroupSpacing);

		m_ToolbarSlots.Left = pLeft; 
		m_ToolbarSlots.Right = pRight;

		ExtendToolbar(m_ToolbarSlots);

		for (auto& pExtension : m_Extensions)
			pExtension->ExtendToolbar(m_ToolbarSlots);

		return pBar;
	}

	Ref<VerticalBox> ViewportPanel::BuildSidePanelWidget()
	{
		const ViewportSidePanelDesc desc = CreateSidePanelDesc();
		if (!desc.IsEnabled())
			return nullptr;

		Ref<VerticalBox> pSettingsBox = RLS_NEW VerticalBox();
		pSettingsBox->SetHorizontalSizePolicy(ESizePolicy::Fixed);
		pSettingsBox->SetVerticalSizePolicy(ESizePolicy::Stretch);
		pSettingsBox->SetSize(Vector2(desc.Width, -1.0f));
		pSettingsBox->SetIsVisible(desc.StartVisible);

		ExtendSidePanel(pSettingsBox);

		for (auto& pExtension : m_Extensions)
			pExtension->ExtendSidePanel(pSettingsBox);

		return pSettingsBox;
	}

	void ViewportPanel::BroadcastEvent(const ViewportInputEvent& aInputEvent)
	{
		for (const auto& pExtension : m_Extensions)
			pExtension->HandleInput(aInputEvent);

		m_pClient->HandleInput(aInputEvent);
	}

	void ViewportPanel::ConfineAndHideMouseAtCursorPosition() noexcept
	{
		const Vector2 cursorScreenPosition = Vector2(static_cast<float>(Mouse::GetCursorScreenPosition().x), static_cast<float>(Mouse::GetCursorScreenPosition().y));
		Mouse::ConfineCursor(cursorScreenPosition.x, cursorScreenPosition.x, cursorScreenPosition.y, cursorScreenPosition.y);
		Mouse::HideCursor();
	}

	bool ViewportPanel::IsCameraValidClientAreaHovered() const noexcept
	{
		const Vector2u cursorScreenPos = Mouse::GetCursorScreenPosition();
		return m_CameraValidScreenRect.Contains(Vector2i(cursorScreenPos.x, cursorScreenPos.y));
	}

	ViewportInputEvent ViewportPanel::MakeInputEvent(EViewportInputType aInputType) const noexcept
	{
		ViewportInputEvent event;
		event.Type = aInputType;
		event.ClientCoordinates = GetClientHoverCoordinates();
		event.PointerInfo = Mouse::CreatePointerInfo();
		event.KeyboardModifiers = Keyboard::GetModifierMask();

		return event;
	}

	void ViewportPanel::OnCanvasHoverStateChanged(bool newState) noexcept
	{
		m_ClientAreaHovered = newState;
	}

	void ViewportPanel::OnCanvasResize(const Vector2i& newSize) noexcept
	{
		if (newSize.x <= 0 || newSize.y <= 0)
			return;

		const float width = Math::Max(1.0f, static_cast<float>(newSize.x));
		const float height = Math::Max(1.0f, static_cast<float>(newSize.y));

		m_pSurface->RequestResize(Vector2u(width, height));
		m_pClient->GetCameraController()->SetViewport(FloatRect(0.0f, 0.0f, width, height));
	}

	Texture* ViewportPanel::OnCanvasTargetRequest() const noexcept
	{
		Texture* pTexture = m_pSurface->GetTexture();
		return pTexture ? pTexture : GraphicsCommon::GetDefaultTexture(DefaultTextureType::Black2D);
	}

	void ViewportPanel::OnCanvasRenderEnd() noexcept
	{
		PROFILE_FUNC;

		for (const auto& pExtension : m_Extensions)
			pExtension->OnCanvasRenderEnd();
	}

	void ViewportPanel::OnFocusLost(MAYBE_UNUSED PanelBase* aPanelBase) noexcept
	{
		ViewportInputEvent inputEvent = MakeInputEvent(EViewportInputType::FocusLost);
		BroadcastEvent(inputEvent);
		UpdateCursorCapture();
	}

	Ref<VerticalBox> ViewportPanel::BuildWindowLayout() noexcept
	{
		Ref<VerticalBox> pRoot = RLS_NEW VerticalBox();

		//Toolbar box:
		{
			if (Ref<IBaseWidget> pToolbar = BuildToolbarWidget())
				pRoot->AddWidget(pToolbar);
		}

		HorizontalBox* pCanvasAndSettingsBox = pRoot->AddWidget(RLS_NEW HorizontalBox());
		pCanvasAndSettingsBox->SetHorizontalSizePolicy(ESizePolicy::Stretch);
		pCanvasAndSettingsBox->SetVerticalSizePolicy(ESizePolicy::Stretch);

		//Canvas box:
		{
			pCanvasAndSettingsBox->AddWidget(BuildDefaultCanvasWidget());
		}
		//Viewport Settings box:
		{
			if (Ref<VerticalBox> pSidePanel = BuildSidePanelWidget())
			{
				pCanvasAndSettingsBox->AddWidget(pSidePanel);

				if (m_ToolbarSlots.Right)
				{
					Button* pButton = m_ToolbarSlots.Right->AddWidget(RLS_NEW Button(ICON_FA_GEAR));
					pButton->OnClicked([sidePanel = pSidePanel.Get(), pButton]()
						{ 
							sidePanel->SetIsVisible(!sidePanel->IsVisible()); 
							pButton->SetTextColor(sidePanel->IsVisible() ? Colors::SoftOrange : Colors::TextInactive);
						});
					pButton->SetTextColor(pSidePanel->IsVisible() ? Colors::SoftOrange : Colors::TextInactive);
					pButton->OnMouseEnter([sidePanel = pSidePanel.Get()](Button* aButton) { aButton->SetTextColor(sidePanel->IsVisible() ? Colors::SoftOrange : Colors::TextDefault); });
					pButton->OnMouseExit([sidePanel = pSidePanel.Get()](Button* aButton) { aButton->SetTextColor(sidePanel->IsVisible() ? Colors::SoftOrange : Colors::TextInactive); });
					pButton->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);
					pButton->SetTooltipText("Toggle the viewport settings panel.");
				}
			}
		}

		return pRoot;
	}

	UniquePtr<ViewportClient> ViewportPanel::CreateClient()
	{
		return MakeUnique<ViewportClient>(CreateClientDesc());
	}

	ViewportClient::Desc ViewportPanel::CreateClientDesc()
	{
		return ViewportClient::Desc{};
	}

	UniquePtr<ViewportSurface> ViewportPanel::CreateSurface()
	{
		return MakeUnique<ViewportSurface>(CreateSurfaceDesc());
	}

	ViewportSurface::Desc ViewportPanel::CreateSurfaceDesc()
	{
		return ViewportSurface::Desc::Native();
	}

	ViewportToolbarDesc ViewportPanel::CreateToolbarDesc()
	{
		return ViewportToolbarDesc::Disabled();
	}

	 ViewportSidePanelDesc ViewportPanel::CreateSidePanelDesc()
	{
		return ViewportSidePanelDesc::Disabled();
	}

	bool ViewportPanel::OnKeyPressedEvent(KeyPressedEvent& event) noexcept
	{
		ViewportInputEvent inputEvent = MakeInputEvent(EViewportInputType::KeyPressed);
		inputEvent.Key = event.key;

		if (!RouteInput(inputEvent))
		{
			OnHotkeyPressed(this, event.key);
			return false;
		}

		return true;
	}

	bool ViewportPanel::OnKeyReleasedEvent(KeyReleasedEvent& event) noexcept
	{
		ViewportInputEvent inputEvent = MakeInputEvent(EViewportInputType::KeyReleased);
		inputEvent.Key = event.key;
		return RouteInput(inputEvent);
	}

	bool ViewportPanel::OnLeftMouseButtonPressedEvent(LeftMouseButtonPressedEvent&) noexcept
	{
		if (!IsCameraValidClientAreaHovered())
			return false;

		m_OwnsCurrentPress = true;

		ViewportInputEvent inputEvent = MakeInputEvent(EViewportInputType::MouseButtonPressed);
		inputEvent.Button = RLS_Button::Left;
		const bool consumed = RouteInput(inputEvent);
		UpdateCursorCapture();
		return consumed;
	}

	bool ViewportPanel::OnLeftMouseButtonReleasedEvent(LeftMouseButtonReleasedEvent&) noexcept
	{
		ViewportInputEvent inputEvent = MakeInputEvent(EViewportInputType::MouseButtonReleased);
		inputEvent.Button = RLS_Button::Left;
		const bool consumed = RouteInput(inputEvent);
		UpdateCursorCapture();
		return consumed;
	}

	bool ViewportPanel::OnRightMouseButtonPressedEvent(RightMouseButtonPressedEvent&) noexcept
	{
		if (!IsFocused())
			ImGui::SetWindowFocus(GetName().c_str());

		m_OwnsCurrentPress = true;

		ViewportInputEvent inputEvent = MakeInputEvent(EViewportInputType::MouseButtonPressed);
		inputEvent.Button = RLS_Button::Right;
		return RouteInput(inputEvent);
	}

	bool ViewportPanel::OnRightMouseButtonReleasedEvent(RightMouseButtonReleasedEvent&) noexcept
	{
		ViewportInputEvent inputEvent = MakeInputEvent(EViewportInputType::MouseButtonReleased);
		inputEvent.Button = RLS_Button::Right;
		return RouteInput(inputEvent);
	}

	bool ViewportPanel::OnMiddleMouseButtonPressedEvent(MiddleMouseButtonPressedEvent&) noexcept
	{
		if (!IsFocused())
			ImGui::SetWindowFocus(GetName().c_str());

		ViewportInputEvent inputEvent = MakeInputEvent(EViewportInputType::MouseButtonPressed);
		inputEvent.Button = RLS_Button::Wheel;
		return RouteInput(inputEvent);
	}

	bool ViewportPanel::OnMiddleMouseButtonReleasedEvent(MiddleMouseButtonReleasedEvent&) noexcept
	{
		ViewportInputEvent inputEvent = MakeInputEvent(EViewportInputType::MouseButtonReleased);
		inputEvent.Button = RLS_Button::Wheel;
		return RouteInput(inputEvent);
	}

	bool ViewportPanel::OnMouseBeginDragEvent(MAYBE_UNUSED MouseBeginDragEvent& aEvent) noexcept
	{
		if (!IsCameraValidClientAreaHovered())
			return false;

		ViewportInputEvent inputEvent = MakeInputEvent(EViewportInputType::MouseDragBegin);
		inputEvent.Button = RLS_Button::Left;
		return RouteInput(inputEvent);
	}

	bool ViewportPanel::OnMouseDragEvent(MouseDragEvent& aEvent) noexcept
	{
		if (!IsCameraValidClientAreaHovered())
			return false;

		ViewportInputEvent inputEvent = MakeInputEvent(EViewportInputType::MouseDrag);
		inputEvent.Button = RLS_Button::Left;
		inputEvent.MouseDelta = aEvent.DeltaCoordinates;
		return RouteInput(inputEvent);
	}

	bool ViewportPanel::OnMouseEndDragEvent(MAYBE_UNUSED MouseEndDragEvent& aEvent) noexcept
	{
		ViewportInputEvent inputEvent = MakeInputEvent(EViewportInputType::MouseDragEnd);
		inputEvent.Button = RLS_Button::Left;
		return RouteInput(inputEvent);
	}

	bool ViewportPanel::OnMouseWheelScrolledEvent(MouseWheelScrolledEvent& event) noexcept
	{
		if (!IsCameraValidClientAreaHovered())		
			return false;

		ViewportInputEvent inputEvent = MakeInputEvent(EViewportInputType::MouseWheel);
		inputEvent.Button = RLS_Button::Wheel;
		inputEvent.WheelDelta = event.Delta;
		return RouteInput(inputEvent);
	}

	void ViewportPanel::RecomputeCameraValidScreenRect() noexcept
	{
		const IntRect screenRect = m_pCanvas->GetScreenRect();
		m_ScreenPosition = Vector2u(screenRect.Left, screenRect.Top);

		constexpr int CAMERA_RESIZE_GRIP_INSET = 8;
		m_CameraValidScreenRect.Left = screenRect.Left + CAMERA_RESIZE_GRIP_INSET;
		m_CameraValidScreenRect.Top = screenRect.Top + CAMERA_RESIZE_GRIP_INSET;
		m_CameraValidScreenRect.Right = screenRect.Right - CAMERA_RESIZE_GRIP_INSET;
		m_CameraValidScreenRect.Bottom = screenRect.Bottom - CAMERA_RESIZE_GRIP_INSET;
	}

	bool ViewportPanel::RouteInput(const ViewportInputEvent& aInputEvent)
	{
		for (const auto& pExtension : m_Extensions)
		{
			if (pExtension->HandleInput(aInputEvent))
				return true;

			if (pExtension->WantsExclusiveInput())
				return true;
		}

		return m_pClient->HandleInput(aInputEvent);
	}

	void ViewportPanel::UpdateCursorCapture()
	{
		const bool shouldCapture = m_pClient->IsNavigating();
		if (shouldCapture == m_CursorCaptured)
			return;

		m_CursorCaptured = shouldCapture;

		if (shouldCapture)
			ConfineAndHideMouseAtCursorPosition();
		else
		{
			Mouse::FreeCursor();
			Mouse::ShowCursor();
		}
	}
}
