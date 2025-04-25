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

		m_ViewportID = std::format("{}_Viewport_{}", pName, renderViewIndex + 1);
		m_ToolbarID = std::format("{}_Toolbar_{}", pName, renderViewIndex + 1);
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
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	}

	void ViewportPanel::OnRender() noexcept
	{
		if (!IsVisible())
			return;

		const Vector2u& region = GetContentRegionAvail();
		if (region.x == 0u || region.y == 0u)
			return;

		ViewportRenderView& renderView = m_pEditor->GetRenderView(m_RenderViewIndex);

		DrawToolbar(renderView);
		DrawViewport(renderView);

		DetermineViewportHoverState();
		DetermineCameraAreaHoverState();

		HandleTransformGizmoInteraction();

		ImGui::EndChild();
	}

	void ViewportPanel::PostRender() noexcept
	{
		ImGui::PopStyleVar();
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

	void ViewportPanel::DetermineViewportHoverState() noexcept
	{
		const bool isHoveringViewport = ImGui::IsMouseHoveringRect(ImVec2(m_ViewportRect.Left, m_ViewportRect.Top), ImVec2(m_ViewportRect.Right, m_ViewportRect.Bottom));
		if (isHoveringViewport && !m_ClientAreaHovered)
			OnBeginViewportHover();
		else if (!isHoveringViewport && m_ClientAreaHovered)
			OnEndViewportHover();

		m_ClientAreaHovered = isHoveringViewport;
	}

	void ViewportPanel::DetermineCameraAreaHoverState() noexcept
	{
		const bool isHoveringCameraValidArea = ImGui::IsMouseHoveringRect(ImVec2(m_CameraValidScreenRect.Left, m_CameraValidScreenRect.Top), ImVec2(m_CameraValidScreenRect.Right, m_CameraValidScreenRect.Bottom));
		if (isHoveringCameraValidArea && !m_CameraValidAreaHovered)
			OnBeginCameraValidAreaHover();
		else if (!isHoveringCameraValidArea && m_CameraValidAreaHovered)
			OnEndCameraValidAreaHover();

		m_CameraValidAreaHovered = isHoveringCameraValidArea;
	}

	void ViewportPanel::DrawCameraValidClientAreaRect() noexcept
	{
		ImGui::GetWindowDrawList()->AddRect(ImVec2(m_CameraValidScreenRect.Left, m_CameraValidScreenRect.Top), ImVec2(m_CameraValidScreenRect.Right, m_CameraValidScreenRect.Bottom), IM_COL32(255, 255, 255, 255));
	}

	void ViewportPanel::DrawToolbar(ViewportRenderView& renderView) noexcept
	{
		ImGui::BeginChild(m_ToolbarID.c_str(), ImVec2(0, 40), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);

		if (ImGui::Button("Wireframe"))
			renderView.RenderMode = renderView.RenderMode == RenderModeEx::Solid ? RenderModeEx::Wireframe : RenderModeEx::Solid;

		ImGui::SameLine();

		if (ImGui::Button("Solid"))
			renderView.DrawGrid = !renderView.DrawGrid;

		ImGui::EndChild();
	}

	void ViewportPanel::DrawViewport(const ViewportRenderView& renderView)
	{
		ImGui::BeginChild(m_ViewportID.c_str(), ImVec2(0.0f, 0.0f), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);

		const Vector2i viewportSize = Vector2i(static_cast<int32>(ImGui::GetContentRegionAvail().x), static_cast<int32>(ImGui::GetContentRegionAvail().y));
		if (m_ViewportSize != viewportSize)
			OnViewportResize(viewportSize);

		ImGui::Image((ImTextureID)renderView.pTarget->GetSRV()->GetGPUHandle().ptr, ImVec2(static_cast<float>(m_ViewportSize.x), static_cast<float>(m_ViewportSize.y)));

		const ImVec2 minRect = ImGui::GetItemRectMin();
		const ImVec2 maxRect = ImGui::GetItemRectMax();
		m_ScreenPosition = Vector2u(minRect.x, minRect.y);

		m_ViewportRect.Left = minRect.x;
		m_ViewportRect.Top = minRect.y;
		m_ViewportRect.Right = maxRect.x;
		m_ViewportRect.Bottom = maxRect.y;

		constexpr int cameraRectHorizontalOffset = 5;
		constexpr int cameraRectTopOffset = 60;
		constexpr int cameraRectBottomOffset = 8;

		m_CameraValidScreenRect.Left = m_ViewportRect.Left + cameraRectHorizontalOffset;
		m_CameraValidScreenRect.Top = m_ViewportRect.Top + cameraRectTopOffset;
		m_CameraValidScreenRect.Right = m_ViewportRect.Right - cameraRectHorizontalOffset;
		m_CameraValidScreenRect.Bottom = m_ViewportRect.Bottom - cameraRectBottomOffset;
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
			.Rect = m_ViewportRect,
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

	bool ViewportPanel::OnKeyPressedEvent(KeyPressedEvent& event) noexcept
	{
		if (!CanHandleHotkeys())
			return false;

		return HandleKeyPressed(event.key);
	}

	bool ViewportPanel::OnLeftMouseButtonPressedEvent(LeftMouseButtonPressedEvent& event) noexcept
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

			break;
		}
		case EViewportState::TransformingGizmo:
		{
			if (m_pTransformController->GetActiveTransformType() == ETransformGizmoType::Translate)
			{
				if (Keyboard::IsKeyDown(RLS_Key::Alt))
					m_pEditor->OnViewportEntityDuplicationRequest();
			}

			break;
		}
		}

		return true;
	}

	bool ViewportPanel::OnLeftMouseButtonReleasedEvent(LeftMouseButtonReleasedEvent& event) noexcept
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

	bool ViewportPanel::OnRightMouseButtonPressedEvent(RightMouseButtonPressedEvent& event) noexcept
	{
		if (!CanHandleMouseInputs())
			return false;

		if (!IsFocused())
			ImGui::SetWindowFocus(GetName().c_str());
		
		if (m_CameraValidAreaHovered)
			ConfineAndHideMouseAtCursorPosition();

		return true;
	}

	bool ViewportPanel::OnRightMouseButtonReleasedEvent(RightMouseButtonReleasedEvent& event) noexcept
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

	bool ViewportPanel::OnMiddleMouseButtonPressedEvent(MiddleMouseButtonPressedEvent& event) noexcept
	{
		if (!CanHandleMouseInputs())
			return false;

		if (!IsFocused())
			ImGui::SetWindowFocus(GetName().c_str());

		if (m_CameraValidAreaHovered)
			ConfineAndHideMouseAtCursorPosition();

		return true;
	}

	bool ViewportPanel::OnMiddleMouseButtonReleasedEvent(MiddleMouseButtonReleasedEvent& event) noexcept
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

		const FloatRect viewport = FloatRect(0.0f, 0.0f, width, height);
		m_pCameraController->SetViewport(viewport);

		const float aspectRatio = width / height;
		constexpr float horizontalFoV = Math::DegToRad(90.0f);
		const float verticalFoV = 2.0f * std::atan(std::tan(horizontalFoV * 0.5f) * (1.0f / aspectRatio));

		m_pCameraController->SetFoV(verticalFoV);
	}
}
