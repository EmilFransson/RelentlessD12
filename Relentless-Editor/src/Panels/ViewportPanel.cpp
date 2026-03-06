#include "ViewportPanel.h"
#include "Core/Editor.h"

#include "UI/Widgets/Button.h"
#include "UI/Widgets/HorizontalBox.h"
#include "UI/Widgets/Spacer.h"
#include "UI/Widgets/VerticalBox.h"
#include "UI/Views/Details/ViewportDetailsView.h"

#include "Subsystem/EditorViewportSubsystem.h"
#include "Subsystem/SelectionSubsystem.h"

namespace Relentless
{
	ViewportPanel::ViewportPanel(uint32 aRenderViewIndex) noexcept
		:PanelBase(std::format("Scene Viewport {}", aRenderViewIndex + 1).c_str(), ImGuiWindowFlags_None),
		 m_RenderViewIndex{aRenderViewIndex}
	{
		m_pCamera = PerspectiveCamera::Create();

		const Vector3 intitialLocation = Vector3(13.0f, 13.0f, -13.0f);
		const Quaternion initialRotation = Math::CreateLookToRotation(intitialLocation, Vector3::Zero);

		m_pCamera->SetLocation(intitialLocation);
		m_pCamera->SetRotation(initialRotation);
		m_pCamera->SetNearPlane(0.1f);
		m_pCamera->SetFarPlane(1'000.0f);

		m_pCameraController = MakeUnique<PerspectiveCameraController>(m_pCamera.get());
		m_pTransformController = MakeUnique<TransformGizmoController>();

		SetRoot(BuildWindowLayout());
		SetPadding(Vector2(0.0f, 0.0f));
	}

	ViewportPanel::~ViewportPanel() noexcept = default;

	bool ViewportPanel::AcceptsMouseInput() const noexcept
	{
		return IsCameraValidClientAreaHovered();
	}

	Ref<VerticalBox> ViewportPanel::BuildWindowLayout() noexcept
	{
		Ref<VerticalBox> pRoot = new VerticalBox();

		//Toolbar box:
		{
			HorizontalBox* pToolbarBox = pRoot->AddWidget(RLS_NEW HorizontalBox());
			pToolbarBox->SetHorizontalSizePolicy(ESizePolicy::Stretch);
			pToolbarBox->SetVerticalSizePolicy(ESizePolicy::Fixed);
			pToolbarBox->SetSize(Vector2(-1.0f, 30.0f));
			pToolbarBox->SetMargin(FloatRect(5.0f, 5.0f, 5.0f, 5.0f));

			pToolbarBox->AddWidget(RLS_NEW Spacer())
				->SetHorizontalSizePolicy(ESizePolicy::Stretch);

			pToolbarBox->AddWidget(RLS_NEW Button(ICON_FA_GEAR))
				->OnClicked(this, &ViewportPanel::OnSettingsButtonClicked)
				->SetTextColor(Color(1.0f, 1.0f, 1.0f, 0.75f))
				->SetFont(ImGui::GetIO().Fonts->Fonts[2])
				->SetBorderColor(Colors::Transparent)
				->SetPadding(Vector2(6.0f, 0.0f))
				->SetVerticalSizePolicy(ESizePolicy::Stretch);
		} 

		HorizontalBox* pCanvasAndSettingsBox = pRoot->AddWidget(RLS_NEW HorizontalBox());
		pCanvasAndSettingsBox->SetHorizontalSizePolicy(ESizePolicy::Stretch);
		pCanvasAndSettingsBox->SetVerticalSizePolicy(ESizePolicy::Stretch);

		//Canvas box:
		{
			HorizontalBox* pCanvasBox = pCanvasAndSettingsBox->AddWidget(RLS_NEW HorizontalBox());
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
		}
		//Viewport Settings box:
		{
			m_pSettingsBox = pCanvasAndSettingsBox->AddWidget(RLS_NEW VerticalBox());
			m_pSettingsBox->SetHorizontalSizePolicy(ESizePolicy::Fixed);
			m_pSettingsBox->SetVerticalSizePolicy(ESizePolicy::Stretch);
			m_pSettingsBox->SetSize(Vector2(450.0f, -1.0f));
			m_pSettingsBox->SetIsVisible(false);

			m_pViewportDetailsView = m_pSettingsBox->AddWidget(RLS_NEW ViewportDetailsView(this));
			m_pViewportDetailsView->SetHorizontalSizePolicy(ESizePolicy::Stretch);
			m_pViewportDetailsView->SetVerticalSizePolicy(ESizePolicy::Stretch);
		}

		return pRoot;
	}

    std::shared_ptr<PerspectiveCamera> ViewportPanel::GetCamera() const noexcept
	{
		return m_pCamera;
	}

	const UniquePtr<PerspectiveCameraController>& ViewportPanel::GetCameraController() const noexcept
	{
		return m_pCameraController;
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
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
	}

	void ViewportPanel::PostRender() noexcept
	{
		ImGui::PopStyleVar();
	}

	void ViewportPanel::Update() noexcept
	{
		RecomputeCameraValidScreenRect();

		m_pCameraController->Update(m_CameraInput);
		m_pCamera->Update();

		m_CameraInput.MouseDelta = Vector2i::Zero();
		m_CameraDeactivatedThisFrame = false;
	}

	void ViewportPanel::ConfineAndHideMouseAtCursorPosition() noexcept
	{
		const Vector2 cursorScreenPosition = Vector2(static_cast<float>(Mouse::GetCursorScreenPosition().x), static_cast<float>(Mouse::GetCursorScreenPosition().y));
		Mouse::ConfineCursor(cursorScreenPosition.x, cursorScreenPosition.x, cursorScreenPosition.y, cursorScreenPosition.y);
		Mouse::HideCursor();
	}

	void ViewportPanel::DrawCameraValidClientAreaRect() noexcept
	{
		ImGui::GetWindowDrawList()->AddRect(ImVec2((float)m_CameraValidScreenRect.Left, (float)m_CameraValidScreenRect.Top), ImVec2((float)m_CameraValidScreenRect.Right, (float)m_CameraValidScreenRect.Bottom), IM_COL32(255, 255, 255, 255));
	}

	void ViewportPanel::HandleTransformGizmoInteraction() noexcept
	{
		auto pEditor = Editor::Get();
		
		std::vector<entity> participatingEntities = pEditor->GetTransformSelection();
		if (participatingEntities.empty())
			return;

		const TransformGizmoControllerContext transformContext
		{
			.Entities = std::move(participatingEntities),
			.WorldToView = m_pCamera->GetViewTransform().WorldToView,
			.ViewToClip = m_pCamera->GetViewTransform().ViewToClip,
			.Rect = m_pCanvas->GetScreenRect(),
			.pScene = pEditor->GetActiveScene(),
		};

		m_pTransformController->Execute(transformContext);
	}

	bool ViewportPanel::IsCameraValidClientAreaHovered() const noexcept
	{
		const Vector2u cursorScreenPos = Mouse::GetCursorScreenPosition();
		return m_CameraValidScreenRect.Contains(Vector2i(cursorScreenPos.x, cursorScreenPos.y));
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

		m_ViewportSize = Vector2i(static_cast<int32>(width), static_cast<int32>(height));
		m_pCameraController->SetViewport(FloatRect(0.0f, 0.0f, width, height));
	}

	Texture* ViewportPanel::OnCanvasTargetRequest() const noexcept
	{
		return Editor::Get()->GetSubsystem<EditorViewportSubsystem>()->GetRenderView(m_RenderViewIndex).pTarget.Get();
	}

	void ViewportPanel::OnCanvasRenderEnd() noexcept
	{
		PROFILE_FUNC;
		HandleTransformGizmoInteraction();
	}

	bool ViewportPanel::OnKeyPressedEvent(KeyPressedEvent& event) noexcept
	{
		switch (event.key)
		{
		case RLS_Key::A: m_CameraInput.MoveAxis.x -= 1.0f;	break;
		case RLS_Key::D: m_CameraInput.MoveAxis.x += 1.0f;	break;
		case RLS_Key::W: m_CameraInput.MoveAxis.z += 1.0f;	break;
		case RLS_Key::S: m_CameraInput.MoveAxis.z -= 1.0f;	break;
		case RLS_Key::Q: m_CameraInput.MoveAxis.y -= 1.0f;	break;
		case RLS_Key::E: m_CameraInput.MoveAxis.y += 1.0f;	break;
		case RLS_Key::Alt: ResolveAndSetCameraMode();		break;
		default: break;
		}

		if (!(Mouse::IsButtonDown(RLS_Button::Left) || Mouse::IsButtonDown(RLS_Button::Right) || Mouse::IsButtonDown(RLS_Button::Wheel)))
		{
			//Transform Gizmo:
			switch (event.key)
			{
			case RLS_Key::Q: m_pTransformController->SetActiveType(ETransformGizmoType::None);		break;
			case RLS_Key::W: m_pTransformController->SetActiveType(ETransformGizmoType::Translate); break;
			case RLS_Key::E: m_pTransformController->SetActiveType(ETransformGizmoType::Rotate);	break;
			case RLS_Key::R: m_pTransformController->SetActiveType(ETransformGizmoType::Scale);		break;
			case RLS_Key::T: m_pTransformController->ToggleActiveMode(); break;
			default: OnHotkeyPressed(this, event.key); break;
			}
		}

		return true;
	}

	bool ViewportPanel::OnKeyReleasedEvent(KeyReleasedEvent& event) noexcept
	{
		switch (event.key)
		{
		case RLS_Key::A: m_CameraInput.MoveAxis.x += 1.0f;	break;
		case RLS_Key::D: m_CameraInput.MoveAxis.x -= 1.0f;	break;
		case RLS_Key::W: m_CameraInput.MoveAxis.z -= 1.0f;	break;
		case RLS_Key::S: m_CameraInput.MoveAxis.z += 1.0f;	break;
		case RLS_Key::Q: m_CameraInput.MoveAxis.y += 1.0f;	break;
		case RLS_Key::E: m_CameraInput.MoveAxis.y -= 1.0f;	break;
		case RLS_Key::Alt: ResolveAndSetCameraMode();		break;
		default: break;
		}
		return true;
	}

	bool ViewportPanel::OnLeftMouseButtonPressedEvent(LeftMouseButtonPressedEvent&) noexcept
	{
		if (m_CameraIsActive)
		{
			ResolveAndSetCameraMode();
			return true;
		}

		SelectionSubsystem* pSelection = Editor::Get()->GetSubsystem<SelectionSubsystem>();
		if (pSelection->GetSelectedEntityCount() > 0u && m_pTransformController->IsInteracting())
		{
			switch (m_pTransformController->GetActiveTransformType())
			{
			case ETransformGizmoType::Translate:
			case ETransformGizmoType::Rotate:
			{
				if (Keyboard::IsKeyDown(RLS_Key::Alt))
					Editor::Get()->OnViewportEntityDuplicationRequest();
				break;
			}
			default:
				break;
			}
		
			return true;
		}

		ConfineAndHideMouseAtCursorPosition();

		return true;
	}

	bool ViewportPanel::OnLeftMouseButtonReleasedEvent(LeftMouseButtonReleasedEvent&) noexcept
	{
		if (!m_CameraIsActive)
		{
			Mouse::FreeCursor();
			Mouse::ShowCursor();
		}

		ResolveAndSetCameraMode();

		SelectionSubsystem* pSelection = Editor::Get()->GetSubsystem<SelectionSubsystem>();
		if (pSelection->GetSelectedEntityCount() > 0u && m_pTransformController->IsInteracting())
			return true;

		if (!m_CameraIsActive && !m_CameraDeactivatedThisFrame)
		{
			const Vector2i hoverCoords = GetClientHoverCoordinates();
			OnClickedOnViewport(this, Vector2u(hoverCoords.x, hoverCoords.y));
		}

		return true;
	}

	bool ViewportPanel::OnRightMouseButtonPressedEvent(RightMouseButtonPressedEvent&) noexcept
	{
		if (!IsFocused())
			ImGui::SetWindowFocus(GetName().c_str());
		
		ConfineAndHideMouseAtCursorPosition();
		ResolveAndSetCameraMode();

		return true;
	}

	bool ViewportPanel::OnRightMouseButtonReleasedEvent(RightMouseButtonReleasedEvent&) noexcept
	{
		if (!m_CameraIsActive)
		{
			Mouse::FreeCursor();
			Mouse::ShowCursor();
		}

		ResolveAndSetCameraMode();

		if (!m_CameraIsActive && !m_CameraDeactivatedThisFrame)
		{
			const Vector2i hoverCoords = GetClientHoverCoordinates();
			OnClickedOnViewport(this, Vector2u(hoverCoords.x, hoverCoords.y));
		}

		return true;
	}

	bool ViewportPanel::OnMiddleMouseButtonPressedEvent(MiddleMouseButtonPressedEvent&) noexcept
	{
		if (!IsFocused())
			ImGui::SetWindowFocus(GetName().c_str());

		ConfineAndHideMouseAtCursorPosition();
		ResolveAndSetCameraMode();

		return true;
	}

	bool ViewportPanel::OnMiddleMouseButtonReleasedEvent(MiddleMouseButtonReleasedEvent&) noexcept
	{
		Mouse::FreeCursor();
		Mouse::ShowCursor();
		
		return true;
	}

	bool ViewportPanel::OnMouseBeginDragEvent(MAYBE_UNUSED MouseBeginDragEvent& aEvent) noexcept
	{
		SelectionSubsystem* pSelection = Editor::Get()->GetSubsystem<SelectionSubsystem>();

		if (pSelection->GetSelectedEntityCount() > 0 && m_pTransformController->IsInteracting())
			return true;

		ResolveAndSetCameraMode();
		m_CameraIsActive = true;

		return true;
	}

	bool ViewportPanel::OnMouseDragEvent(MouseDragEvent& aEvent) noexcept
	{
		if (IsCameraValidClientAreaHovered())
			m_CameraInput.MouseDelta = aEvent.DeltaCoordinates;

		return true;
	}

	bool ViewportPanel::OnMouseEndDragEvent(MAYBE_UNUSED MouseEndDragEvent& aEvent) noexcept
	{
		ResolveAndSetCameraMode();

		m_CameraIsActive = false;
		m_CameraDeactivatedThisFrame = true;
		
		return true;
	}

	bool ViewportPanel::OnMouseWheelScrolledEvent(MouseWheelScrolledEvent& event) noexcept
	{
		const bool scrolledUp = event.Delta > 0.0f;

		if (m_pCameraController->GetMode() == ECameraControllerNavigationMode::Orbit)
		{
			if (scrolledUp)
				m_pCameraController->ZoomOrbit(-1.0f);
			else
				m_pCameraController->ZoomOrbit(1.0f);
		}
		else if (m_pCameraController->GetMode() == ECameraControllerNavigationMode::Fly)
				m_pCameraController->StepSpeed(scrolledUp);

		return true;
	}

	void ViewportPanel::OnSettingsButtonClicked()
	{
		m_pSettingsBox->SetIsVisible(!m_pSettingsBox->IsVisible());
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

	void ViewportPanel::RecomputeCameraValidScreenRect() noexcept
	{
		const IntRect screenRect = m_pCanvas->GetScreenRect();
		m_ScreenPosition = Vector2u(screenRect.Left, screenRect.Top);

		constexpr int edgeOffset = 8;
		m_CameraValidScreenRect.Left = screenRect.Left + edgeOffset;
		m_CameraValidScreenRect.Top = screenRect.Top + edgeOffset;
		m_CameraValidScreenRect.Right = screenRect.Right - edgeOffset;
		m_CameraValidScreenRect.Bottom = screenRect.Bottom - edgeOffset;
	}

	void ViewportPanel::ResolveAndSetCameraMode() noexcept
	{
		if (Mouse::IsButtonDown(RLS_Button::Left) && Mouse::IsButtonDown(RLS_Button::Right))
			m_pCameraController->SetMode(ECameraControllerNavigationMode::Pan);
		else if (Mouse::IsButtonDown(RLS_Button::Left))
		{
			if (Keyboard::IsKeyDown(RLS_Key::Alt))
				m_pCameraController->SetMode(ECameraControllerNavigationMode::Orbit);
			else
				m_pCameraController->SetMode(ECameraControllerNavigationMode::Dolly);
		}
		else if (Mouse::IsButtonDown(RLS_Button::Right))
			m_pCameraController->SetMode(ECameraControllerNavigationMode::Fly);
		else if (Mouse::IsButtonDown(RLS_Button::Wheel))
			m_pCameraController->SetMode(ECameraControllerNavigationMode::Pan);
		else 
			m_pCameraController->SetMode(ECameraControllerNavigationMode::None);
	}
}
