#include "ViewportPanel.h"
#include "../Core/Editor.h"

namespace Relentless
{
	ViewportPanel::ViewportPanel(const char* pName, ImGuiWindowFlags flags, Editor* pEditor, uint32 renderViewIndex) noexcept
		: PanelBase(pName, flags), m_pEditor{pEditor}, m_RenderViewIndex{renderViewIndex}
	{
		m_pCamera = PerspectiveCamera::Create();

		const Vector3 intitialLocation = Vector3(13.0f, 13.0f, -13.0f);
		const Quaternion initialRotation = Math::CreateLookToRotation(intitialLocation, Vector3::Zero);

		m_pCamera->SetLocation(intitialLocation);
		m_pCamera->SetRotation(initialRotation);
		m_pCamera->SetNearPlane(0.1f);
		m_pCamera->SetFarPlane(1'000.0f);

		m_pCameraController = std::make_unique<PerspectiveCameraController>(m_pCamera.get());
		m_pCameraController->OnBeginTransform.Connect(this, &ViewportPanel::OnCameraBeginMove);
		m_pCameraController->OnEndTransform.Connect(this, &ViewportPanel::OnCameraEndMove);

		m_pTransformController = std::make_unique<TransformGizmoController>();
		m_pTransformController->OnInteractionStateChanged.Connect(this, &ViewportPanel::OnTransformGizmoInteractionStateChanged);

		OnGainedFocus.Connect(this, &ViewportPanel::OnFocusGained);
		OnLostFocus.Connect(this, &ViewportPanel::OnFocusLost);

		m_ToolbarID = std::format("{}_Toolbar_{}", pName, renderViewIndex + 1);

		Ref<VerticalBox> pRoot = new VerticalBox();
		pRoot->SetSpacing(Vector2(0.0f, 0.0f));

		m_pToolbarBox = new HorizontalBox(true, Vector2(0, 40));
		//m_pToolbarBox->SetMargin(FloatRect(5.0f, 1.0f, 0.0f, 0.0f));
		m_pToolbarBox->SetFlags(ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);

		Ref<HorizontalBox> pToolBarLeftBox = new HorizontalBox("##ToolbarLeftAlignedBox");
		pToolBarLeftBox->SetAlignmentPolicy(EAlignmentPolicy::Right);

		Ref<Button> m_pWireFrameButton = new Button("Wireframe", Vector2(0, 40));
		Ref<Button> m_pSolidButton = new Button("Solid", Vector2(0, 40));
		Ref<Button> m_pTSnappingButton = new Button("T-Snap", Vector2(0, 40));
		Ref<Button> m_pRSnappingButton = new Button("R-Snap", Vector2(0, 40));
		
		pToolBarLeftBox->Add(m_pWireFrameButton);
		pToolBarLeftBox->Add(m_pSolidButton);
		pToolBarLeftBox->Add(m_pTSnappingButton);
		pToolBarLeftBox->Add(m_pRSnappingButton);

		m_pToolbarBox->Add(pToolBarLeftBox);

		//Ref<HorizontalBox> pToolBarRightBox = new HorizontalBox("##ToolbarRightAlignedBox");
		//pToolBarRightBox->SetAlignmentPolicy(EAlignmentPolicy::Right);
		//
		//Ref<Button> pGearIconButton = new Button("HEY", Vector2(0, 20));
		//pGearIconButton->OnClicked(this, &ViewportPanel::OnSettingsButtonClicked);
		////pGearIconButton->SetFont(ImGui::GetIO().Fonts->Fonts[2]);
		//pToolBarRightBox->Add(pGearIconButton);
		//
		//m_pToolbarBox->Add(pToolBarRightBox);
		
		pRoot->Add(m_pToolbarBox);

		m_pCanvasAndSettingsBox = new HorizontalBox("##CanvasAndSettingsBox");
		m_pCanvasAndSettingsBox->SetSpacing(Vector2(0.0f, 0.0f));

		m_pCanvasHBox = m_pCanvasAndSettingsBox->Add(new HorizontalBox(true, Vector2(0.0f, 0.0f)));

		m_pCanvas = m_pCanvasHBox->Add(new Canvas());
		m_pCanvas
			->Target(this, &ViewportPanel::OnCanvasTargetRequest)
			->OnHoverStateChanged(this, &ViewportPanel::OnCanvasHoverStateChanged)
			->OnResize(this, &ViewportPanel::OnCanvasResize)
			->OnRenderEnd.Connect(this, &ViewportPanel::OnCanvasRenderEnd);

		//m_pSettingsBox = m_pCanvasAndSettingsBox->Add(new VerticalBox(Vector2(350.0f, 0.0f), true));
		//m_pSettingsBox->SetIsVisible(false);

		//CollapsibleSection* pCameraSection = m_pSettingsBox->Add(new CollapsibleSection(ICON_FA_CAMERA "  Camera"));
		//
		//Table* pCameraSettingsTable = pCameraSection->Add(new Table());
		//uint32 currentRow = 0u;
		//
		//{
		//	pCameraSettingsTable->Add(new Label("Speed Multiplier"), 0, currentRow);
		//	pCameraSettingsTable->Add(new FloatSlider(m_pCameraController->GetMinSpeedMultiplierLimit(), m_pCameraController->GetMaxSpeedMultiplierLimit(), "%.3f"), 1, currentRow)
		//		->Value(this, &ViewportPanel::OnCameraSpeedMultiplierRequested)
		//		->OnValueChanged(this, &ViewportPanel::OnCameraSpeedMultiplierChanged);
		//	currentRow++;
		//}
		//
		//{
		//	pCameraSettingsTable->Add(new Label("Field of View (H)"), 0, currentRow);
		//	pCameraSettingsTable->Add(new FloatSlider(5.0f, 170.0f, "%.3f"), 1, currentRow)
		//		->Value(this, &ViewportPanel::OnHorizontalFOVRequested)
		//		->OnValueChanged(this, &ViewportPanel::OnHorizontalFOVChanged);
		//	currentRow++;
		//}
		//
		//{
		//	pCameraSettingsTable->Add(new Label("Near View Plane"), 0, currentRow);
		//	pCameraSettingsTable->Add(new FloatSlider(0.01f, 100'000.0f, "%.3f", ImGuiSliderFlags_Logarithmic), 1, currentRow)
		//		->Value(this, &ViewportPanel::OnCameraNearViewPlaneRequested)
		//		->OnValueChanged(this, &ViewportPanel::OnCameraNearViewPlaneChanged);
		//	currentRow++;
		//}
		//
		//{
		//	pCameraSettingsTable->Add(new Label("Far View Plane"), 0, currentRow);
		//	pCameraSettingsTable->Add(new FloatSlider(0.01f, 100'000.0f, "%.3f", ImGuiSliderFlags_Logarithmic), 1, currentRow)
		//		->Value(this, &ViewportPanel::OnCameraFarViewPlaneRequested)
		//		->OnValueChanged(this, &ViewportPanel::OnCameraFarViewPlaneChanged);
		//	currentRow++;
		//}
		//
		//{
		//	pCameraSettingsTable->Add(new Label("EV100"), 0, currentRow);
		//	pCameraSettingsTable->Add(new FloatSlider(-10.0f, 10.0f, "%.3f"), 1, currentRow)
		//		->Value(this, &ViewportPanel::OnEV100Requested)
		//		->OnValueChanged(this, &ViewportPanel::OnEV100Changed);
		//	currentRow++;
		//}
		

		pRoot->Add(m_pCanvasAndSettingsBox);
		SetRoot(pRoot);
	}

	std::shared_ptr<PerspectiveCamera> ViewportPanel::GetCamera() const noexcept
	{
		return m_pCamera;
	}

	uint32 ViewportPanel::GetRenderViewIndex() const noexcept
	{
		return m_RenderViewIndex;
	}

	const Vector2i& ViewportPanel::GetViewportSize() const noexcept
	{
		return m_ViewportSize;
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

	bool ViewportPanel::IsClientAreaHovered() const noexcept
	{
		return m_ClientAreaHovered;
	}

	void ViewportPanel::PreRender() noexcept
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
	}

	void ViewportPanel::PostRender() noexcept
	{
		ImGui::PopStyleVar(2);
	}

	void ViewportPanel::Update() noexcept
	{
		m_pCameraController->Update();
		m_pCamera->Update();
	}

	bool ViewportPanel::CanHandleHotkeys() const noexcept
	{
		return IsFocused() && !Mouse::IsButtonDown(RLS_Button::Right) && !Mouse::IsButtonDown(RLS_Button::Left);
	}

	bool ViewportPanel::CanHandleMouseInputs() const noexcept
	{
		return IsHovered();
	}

	void ViewportPanel::ConfineAndHideMouseAtCursorPosition() noexcept
	{
		const Vector2 cursorScreenPosition = Vector2(static_cast<float>(Mouse::GetCursorScreenPosition().x), static_cast<float>(Mouse::GetCursorScreenPosition().y));
		Mouse::ConfineCursor(cursorScreenPosition.x, cursorScreenPosition.x, cursorScreenPosition.y, cursorScreenPosition.y);
		Mouse::HideCursor();
	}

	void ViewportPanel::DetermineCameraAreaHoverState() noexcept
	{
		const bool isHoveringCameraValidArea = ImGui::IsMouseHoveringRect(ImVec2((float)m_CameraValidScreenRect.Left, (float)m_CameraValidScreenRect.Top), ImVec2((float)m_CameraValidScreenRect.Right, (float)m_CameraValidScreenRect.Bottom));
		if (isHoveringCameraValidArea && !m_CameraValidAreaHovered)
			OnBeginCameraValidAreaHover();
		else if (!isHoveringCameraValidArea && m_CameraValidAreaHovered)
			OnEndCameraValidAreaHover();

		m_CameraValidAreaHovered = isHoveringCameraValidArea;
	}

	void ViewportPanel::DrawCameraValidClientAreaRect() noexcept
	{
		ImGui::GetWindowDrawList()->AddRect(ImVec2((float)m_CameraValidScreenRect.Left, (float)m_CameraValidScreenRect.Top), ImVec2((float)m_CameraValidScreenRect.Right, (float)m_CameraValidScreenRect.Bottom), IM_COL32(255, 255, 255, 255));
	}

	bool ViewportPanel::HandleKeyPressed(RLS_Key key) noexcept
	{
		switch (key)
		{
		case RLS_Key::Q: m_pTransformController->SetActiveType(ETransformGizmoType::None); return true;
		case RLS_Key::W: m_pTransformController->SetActiveType(ETransformGizmoType::Translate); return true;
		case RLS_Key::E: m_pTransformController->SetActiveType(ETransformGizmoType::Rotate); return true;
		case RLS_Key::R: m_pTransformController->SetActiveType(ETransformGizmoType::Scale); return true;
		case RLS_Key::T: m_pTransformController->ToggleActiveMode(); return true;
		default: OnHotkeyPressed(this, key); return true;
		}
	}

	void ViewportPanel::HandleTransformGizmoInteraction() noexcept
	{
		std::vector<entity> participatingEntities = m_pEditor->GetTransformSelection();
		if (participatingEntities.empty())
			return;

		const TransformGizmoControllerContext transformContext
		{
			.Entities = std::move(participatingEntities),
			.WorldToView = m_pCamera->GetViewTransform().WorldToView,
			.ViewToClip = m_pCamera->GetViewTransform().ViewToClip,
			.Rect = m_pCanvas->GetScreenRect(),
			.pScene = m_pEditor->GetActiveScene(),
		};

		m_pTransformController->Execute(transformContext);
	}

	bool ViewportPanel::IsCameraValidClientAreaHovered() const noexcept
	{
		const Vector2i hoverCoords = GetClientHoverCoordinates();
		if (hoverCoords == Vector2i(-1, -1))
			return false;

		constexpr int protectedPixelSize = 5;
		if (hoverCoords.x >= protectedPixelSize && hoverCoords.x <= m_ViewportSize.x - protectedPixelSize
			&& hoverCoords.y >= protectedPixelSize && hoverCoords.y <= m_ViewportSize.y - protectedPixelSize)
		{
			return true;
		}

		return false;
	}

	void ViewportPanel::OnCameraSpeedMultiplierChanged(float speed) noexcept
	{
		m_pCameraController->SetSpeed(speed);
	}

	float ViewportPanel::OnCameraSpeedMultiplierRequested() const noexcept
	{
		return m_pCameraController->GetSpeedMultiplier();
	}

	void ViewportPanel::OnCanvasHoverStateChanged(bool newState) noexcept
	{
		m_ClientAreaHovered = newState;
		
		if (m_ClientAreaHovered)
			OnBeginViewportHover();
		else
			OnEndViewportHover();
	}

	void ViewportPanel::OnCanvasResize(const Vector2i& newSize) noexcept
	{
		if (newSize.x <= 0 || newSize.y <= 0)
			return;

		const IntRect screenRect = m_pCanvas->GetScreenRect();

		m_ScreenPosition = Vector2u(screenRect.Left, screenRect.Top);

		constexpr int cameraRectHorizontalOffset = 4;
		constexpr int cameraRectTopOffset = 60;
		constexpr int cameraRectBottomOffset = 8;

		m_CameraValidScreenRect.Left = screenRect.Left + cameraRectHorizontalOffset;
		m_CameraValidScreenRect.Top = screenRect.Top + cameraRectTopOffset;
		m_CameraValidScreenRect.Right = screenRect.Right - cameraRectHorizontalOffset;
		m_CameraValidScreenRect.Bottom = screenRect.Bottom - cameraRectBottomOffset;

		const float width = Math::Max(1.0f, static_cast<float>(newSize.x));
		const float height = Math::Max(1.0f, static_cast<float>(newSize.y));

		m_ViewportSize = Vector2i((int32)width, (int32)height);
		m_pCameraController->SetViewport(FloatRect(0.0f, 0.0f, width, height));
	}

	Texture* ViewportPanel::OnCanvasTargetRequest() const noexcept
	{
		return m_pEditor->GetRenderView(m_RenderViewIndex).pTarget.Get();
	}

	void ViewportPanel::OnCanvasRenderEnd() noexcept
	{
		PROFILE_FUNC;

		DetermineCameraAreaHoverState();
		HandleTransformGizmoInteraction();
	}

	float ViewportPanel::OnCameraFarViewPlaneRequested() const noexcept
	{
		return m_pCameraController->GetFarPlane();
	}

	float ViewportPanel::OnCameraNearViewPlaneRequested() const noexcept
	{
		return m_pCameraController->GetNearPlane();
	}

	void ViewportPanel::SetState(EViewportState newState) noexcept
	{
		if (newState == m_CurrentState)
			return;

		m_CurrentState = newState;

		if (m_CurrentState == EViewportState::TransformingGizmo)
			m_pCameraController->SetAllowMovement(false);
		else if (m_CameraValidAreaHovered)
			m_pCameraController->SetAllowMovement(true);
	}

	void ViewportPanel::OnCameraNearViewPlaneChanged(float nearPlane) noexcept
	{
		m_pCameraController->SetNearPlane(nearPlane);
	}

	bool ViewportPanel::OnKeyPressedEvent(KeyPressedEvent& event) noexcept
	{
		if (!CanHandleHotkeys())
			return false;

		return HandleKeyPressed(event.key);
	}

	bool ViewportPanel::OnLeftMouseButtonPressedEvent(LeftMouseButtonPressedEvent&) noexcept
	{
		if (!CanHandleMouseInputs())
			return false;
		
		switch (m_CurrentState)
		{
		case EViewportState::Default:
		case EViewportState::None:
		{
			if (m_CameraValidAreaHovered)
				ConfineAndHideMouseAtCursorPosition();

			if (m_CameraValidAreaHovered)
				m_pCameraController->SetAllowMovement(true);

			break;
		}
		case EViewportState::TransformingGizmo:
		{
			switch (m_pTransformController->GetActiveTransformType())
			{
			case ETransformGizmoType::Translate:
			case ETransformGizmoType::Rotate:
			{
				if (Keyboard::IsKeyDown(RLS_Key::Alt))
					m_pEditor->OnViewportEntityDuplicationRequest();
				break;
			}
			}

			break;
		}
		}

		return true;
	}

	bool ViewportPanel::OnLeftMouseButtonReleasedEvent(LeftMouseButtonReleasedEvent&) noexcept
	{
		if (!CanHandleMouseInputs() || !m_CameraValidAreaHovered)
			return false;

		Mouse::FreeCursor();
		Mouse::ShowCursor();

		switch (m_CurrentState)
		{
		case EViewportState::Default:
		{
			const Vector2i hoverCoords = GetClientHoverCoordinates();
			OnClickedOnViewport(this, Vector2u(hoverCoords.x, hoverCoords.y));

			break;
		}
		case EViewportState::NavigatingScene:
		{
			SetState(EViewportState::Default);
			break;
		}
		}

		return true;
	}

	bool ViewportPanel::OnRightMouseButtonPressedEvent(RightMouseButtonPressedEvent&) noexcept
	{
		if (!CanHandleMouseInputs())
			return false;

		if (!IsFocused())
			ImGui::SetWindowFocus(GetName().c_str());
		
		if (m_CameraValidAreaHovered)
			ConfineAndHideMouseAtCursorPosition();

		if (m_CameraValidAreaHovered)
			m_pCameraController->SetAllowMovement(true);

		return true;
	}

	bool ViewportPanel::OnRightMouseButtonReleasedEvent(RightMouseButtonReleasedEvent&) noexcept
	{
		if (!CanHandleMouseInputs())
			return false;

		Mouse::FreeCursor();
		Mouse::ShowCursor();

		switch (m_CurrentState)
		{
		case EViewportState::Default:
		{
			const Vector2i hoverCoords = GetClientHoverCoordinates();
			OnClickedOnViewport(this, Vector2u(hoverCoords.x, hoverCoords.y));
			break;
		}
		case EViewportState::NavigatingScene:
			SetState(EViewportState::Default);
			break;
		}

		return true;
	}

	bool ViewportPanel::OnMiddleMouseButtonPressedEvent(MiddleMouseButtonPressedEvent&) noexcept
	{
		if (!CanHandleMouseInputs())
			return false;

		if (!IsFocused())
			ImGui::SetWindowFocus(GetName().c_str());

		if (m_CameraValidAreaHovered)
			ConfineAndHideMouseAtCursorPosition();

		if (m_CameraValidAreaHovered)
			m_pCameraController->SetAllowMovement(true);

		return true;
	}

	bool ViewportPanel::OnMiddleMouseButtonReleasedEvent(MiddleMouseButtonReleasedEvent&) noexcept
	{
		if (!CanHandleMouseInputs())
			return false;

		Mouse::FreeCursor();
		Mouse::ShowCursor();

		if (m_CurrentState == EViewportState::NavigatingScene)
			SetState(EViewportState::Default);
		
		return true;
	}

	bool ViewportPanel::OnMouseWheelScrolledEvent(MouseWheelScrolledEvent& event) noexcept
	{
		if (!CanHandleMouseInputs())
			return false;

		const bool scrolledUp = event.Delta > 0.0f;

		if (m_pCameraController->GetState() == ECameraControllerNavigationState::Orbit)
		{
			if (scrolledUp)
				m_pCameraController->ZoomOrbit(-1.0f);
			else
				m_pCameraController->ZoomOrbit(1.0f);
		}
		else if (m_pCameraController->GetState() == ECameraControllerNavigationState::Fly)
				m_pCameraController->StepSpeed(scrolledUp);

		return true;
	}

	void ViewportPanel::OnBeginViewportHover() noexcept
	{
		OnMouseEnterViewport(this);
		m_pTransformController->SetAllowManipulation(true);
	}

	void ViewportPanel::OnEndViewportHover() noexcept
	{
		OnMouseExitViewport(this);
		m_pTransformController->SetAllowManipulation(false);
	}

	void ViewportPanel::OnBeginCameraValidAreaHover() noexcept
	{
		if (!Mouse::IsButtonDown(RLS_Button::Left) && !Mouse::IsButtonDown(RLS_Button::Right) && !Mouse::IsButtonDown(RLS_Button::Wheel))
			m_pCameraController->SetAllowMovement(true);
	}

	void ViewportPanel::OnEndCameraValidAreaHover() noexcept
	{
		m_pCameraController->SetAllowMovement(false);
	}

	void ViewportPanel::OnCameraBeginMove() noexcept
	{
		SetState(EViewportState::NavigatingScene);
	}

	void ViewportPanel::OnCameraEndMove() noexcept
	{
		if (!IsFocused())
			SetState(EViewportState::None);
	
		//Otherwise, continue to assume the user is still navigating scene (i.e. don't change state)!
	}

	void ViewportPanel::OnFocusGained(PanelBase*) noexcept
	{
		if (m_pTransformController->GetCurrentInteractionState() == ETransformGizmoInteractionState::None)
			SetState(EViewportState::Default);
		else
			SetState(EViewportState::TransformingGizmo);
	}

	void ViewportPanel::OnFocusLost(PanelBase*) noexcept
	{
		SetState(EViewportState::None);
		m_pCameraController->SetVelocity(Vector3::Zero);
		m_pCameraController->SetAllowMovement(false);
	}

	void ViewportPanel::OnHorizontalFOVChanged(float value) noexcept
	{
		m_pCameraController->SetHorizontalFoV(Math::DegToRad(value));
	}

	float ViewportPanel::OnHorizontalFOVRequested() const noexcept
	{
		return Math::RadToDeg(m_pCameraController->GetHorizontalFoV());
	}

	void ViewportPanel::OnSettingsButtonClicked()
	{
		m_ShowSettingsPanel = !m_ShowSettingsPanel;

		if (m_ShowSettingsPanel)
		{
			m_pCanvasHBox->SetSize(Vector2(-350.0f, 0.0f));
			m_pSettingsBox->SetIsVisible(true);
		}
		else
		{
			m_pCanvasHBox->SetSize(Vector2(0, 0));
			m_pSettingsBox->SetIsVisible(false);
		}
	}

	void ViewportPanel::OnTransformGizmoInteractionStateChanged(ETransformGizmoInteractionState newState) noexcept
	{
		//If the cursor crosses a gizmo handle while navigating we do not switch state.
		if (m_CurrentState == EViewportState::NavigatingScene)
			return;

		if (newState == ETransformGizmoInteractionState::Hovering || newState == ETransformGizmoInteractionState::Using)
			SetState(EViewportState::TransformingGizmo);
		else
			SetState(EViewportState::Default);
	}

	void ViewportPanel::OnViewportResize(const Vector2i& newSize)
	{
		if (newSize.x <= 0 || newSize.y <= 0)
			return;

		const float width = Math::Max(1.0f, static_cast<float>(newSize.x));
		const float height = Math::Max(1.0f, static_cast<float>(newSize.y));

		m_ViewportSize = Vector2i((int32)width, (int32)height);
		m_pCameraController->SetViewport(FloatRect(0.0f, 0.0f, width, height));
	}

	float ViewportPanel::OnEV100Requested() const noexcept
	{
		return Math::Log2f(m_pEditor->GetRenderView(m_RenderViewIndex).Exposure);
	}

	void ViewportPanel::OnEV100Changed(float ev100) noexcept
	{
		m_pEditor->GetRenderView(m_RenderViewIndex).Exposure = Math::Pow2f(ev100);
	}

	void ViewportPanel::OnCameraFarViewPlaneChanged(float farPlane) noexcept
	{
		m_pCameraController->SetFarPlane(farPlane);
	}
}
