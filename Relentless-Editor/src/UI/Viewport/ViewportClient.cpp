#include "ViewportClient.h"

#include "Core/Editor.h"

#include "ViewportSurface.h"

namespace Relentless
{
	ViewportClient::ViewportClient(const Desc& aDesc) noexcept
		: m_Desc{aDesc}
	{
		m_pCamera = MakeUnique<PerspectiveCamera>();
		m_pCamera->SetLocation(m_Desc.Location);
		m_pCamera->SetRotation(Math::CreateLookToRotation(m_Desc.Location, m_Desc.FocusTarget));
		m_pCamera->SetNearPlane(m_Desc.NearPlane);
		m_pCamera->SetFarPlane(m_Desc.FarPlane);

		if (m_Desc.NavigationPreset != ECameraNavigationPreset::None)
			m_pCameraController = MakeUnique<PerspectiveCameraController>(m_pCamera.get());
	}

	void ViewportClient::DisableAllRenderFeatures() noexcept
	{
		m_Desc.RenderFeatures.DisableAll();
	}

	void ViewportClient::EnableAllRenderFeatures() noexcept
	{
		m_Desc.RenderFeatures.EnableAll();
	}

	PerspectiveCamera& ViewportClient::GetCamera() noexcept
	{
		return *m_pCamera;
	}

	const PerspectiveCamera& ViewportClient::GetCamera() const noexcept
	{
		return *m_pCamera;
	}

	PerspectiveCameraController* ViewportClient::GetCameraController() noexcept
	{
		return m_pCameraController ? m_pCameraController.get() : nullptr;
	}

	const PerspectiveCameraController* ViewportClient::GetCameraController() const noexcept
	{
		return m_pCameraController ? m_pCameraController.get() : nullptr;
	}

	const RenderFeatures& ViewportClient::GetRenderFeatures() const noexcept
	{
		return m_Desc.RenderFeatures;
	}

	const RenderQualitySettings& ViewportClient::GetRenderQualitySettings() const noexcept
	{
		return m_Desc.RenderQualitySettings;
	}

	ERenderViewMode ViewportClient::GetViewMode() const noexcept
	{
		return m_Desc.ViewMode;
	}

	bool ViewportClient::HandleInput(const ViewportInputEvent& aInputEvent)
	{
		if (!m_pCameraController)
			return false;

		switch (aInputEvent.Type)
		{
		case EViewportInputType::KeyPressed:
		{
			if (!aInputEvent.PointerInfo.PressedButtons.contains(RLS_Button::Right))
				return false;

			switch (aInputEvent.Key)
			{
			case RLS_Key::A: m_CameraInput.MoveAxis.x -= 1.0f;	return true;
			case RLS_Key::D: m_CameraInput.MoveAxis.x += 1.0f;	return true;
			case RLS_Key::W: m_CameraInput.MoveAxis.z += 1.0f;	return true;
			case RLS_Key::S: m_CameraInput.MoveAxis.z -= 1.0f;	return true;
			case RLS_Key::Q: m_CameraInput.MoveAxis.y -= 1.0f;	return true;
			case RLS_Key::E: m_CameraInput.MoveAxis.y += 1.0f;	return true;
			case RLS_Key::Alt: ResolveAndSetCameraMode();		return true;
			default: break;
			}
			break;
		}
		case EViewportInputType::KeyReleased:
		{
			if (!aInputEvent.PointerInfo.PressedButtons.contains(RLS_Button::Right))
				return false;

			switch (aInputEvent.Key)
			{
			case RLS_Key::A: m_CameraInput.MoveAxis.x += 1.0f;	return true;
			case RLS_Key::D: m_CameraInput.MoveAxis.x -= 1.0f;	return true;
			case RLS_Key::W: m_CameraInput.MoveAxis.z -= 1.0f;	return true;
			case RLS_Key::S: m_CameraInput.MoveAxis.z += 1.0f;	return true;
			case RLS_Key::Q: m_CameraInput.MoveAxis.y += 1.0f;	return true;
			case RLS_Key::E: m_CameraInput.MoveAxis.y -= 1.0f;	return true;
			case RLS_Key::Alt: ResolveAndSetCameraMode();		return true;
			default: break;
			}
			break;
		}
		case EViewportInputType::MouseButtonPressed:
		{
			switch (aInputEvent.Button)
			{
			case RLS_Button::Left:
			case RLS_Button::Right:
			case RLS_Button::Wheel:
				ResolveAndSetCameraMode();
				return m_IsNavigating;
			default:
				return false;
			}
		}
		case EViewportInputType::MouseButtonReleased:
		{
			switch (aInputEvent.Button)
			{
			case RLS_Button::Left:
			case RLS_Button::Right:
			case RLS_Button::Wheel:
			{
				const bool wasNavigating = m_IsNavigating;
				ResolveAndSetCameraMode();
				return wasNavigating;
			}

			default:
				return false;
			}
		}
		case EViewportInputType::MouseWheel:
		{
			const bool scrolledUp = aInputEvent.WheelDelta > 0.0f;

			if (m_pCameraController->GetMode() == ECameraControllerNavigationMode::Orbit)
			{
				m_pCameraController->ZoomOrbit(scrolledUp ? -1.0f : 1.0f);
				return true;
			}
			else if (m_pCameraController->GetMode() == ECameraControllerNavigationMode::Fly)
			{
				m_pCameraController->StepSpeed(scrolledUp);
				return true;
			}
			break;
		}
		case EViewportInputType::MouseDragBegin:
		{
			ResolveAndSetCameraMode();
			return true;
		}
		case EViewportInputType::MouseDrag:
		{
			m_CameraInput.MouseDelta = aInputEvent.MouseDelta;
			return true;
		}
		case EViewportInputType::MouseDragEnd:
		{
			ResolveAndSetCameraMode();
			return true;
		}
		case EViewportInputType::FocusLost:
		{
			m_CameraInput.MoveAxis = Vector3::Zero;
			return true;
		}
		default: break;
		}

		return false;
	}

	bool ViewportClient::HasRenderFeature(ERenderFeature aRenderFeature) const noexcept
	{
		return m_Desc.RenderFeatures.IsEnabled(aRenderFeature);
	}

	bool ViewportClient::IsNavigating() const noexcept
	{
		return m_IsNavigating;
	}

	bool ViewportClient::IsNavigationEnabled() const noexcept
	{
		return m_NavigationEnabled;
	}

	void ViewportClient::SetRenderFeature(ERenderFeature aRenderFeature, bool aEnabled) noexcept
	{
		if (aEnabled)
			m_Desc.RenderFeatures.Enable(aRenderFeature);
		else
			m_Desc.RenderFeatures.Disable(aRenderFeature);
	}

	void ViewportClient::SetNavigationEnabled(bool aEnable) noexcept
	{
		m_NavigationEnabled = aEnable;

		if (m_pCameraController)
			m_pCameraController->SetEnabled(aEnable);
	}

	void ViewportClient::SetViewMode(ERenderViewMode aViewMode)
	{
		if (m_Desc.ViewMode == aViewMode)
			return;

		m_Desc.ViewMode = aViewMode;
		OnViewModeChanged(m_Desc.ViewMode);
	}

	void ViewportClient::ToggleRenderFeature(ERenderFeature aRenderFeature) noexcept
	{
		if (m_Desc.RenderFeatures.IsEnabled(aRenderFeature))
			m_Desc.RenderFeatures.Disable(aRenderFeature);
		else
			m_Desc.RenderFeatures.Enable(aRenderFeature);
	}

	void ViewportClient::Update(MAYBE_UNUSED float aDeltaTime)
	{
		ResolveAndSetCameraMode();

		if (m_pCameraController)
			m_pCameraController->Update(m_CameraInput);
		
		m_pCamera->Update();

		m_CameraInput.MouseDelta = Vector2i::Zero();
	}

	void ViewportClient::ResolveAndSetCameraMode() noexcept
	{
		if (!m_pCameraController || !m_NavigationEnabled)
		{
			if (m_pCameraController)
				m_pCameraController->SetMode(ECameraControllerNavigationMode::None);

			m_IsNavigating = false;
			return;
		}

		m_IsNavigating = true;

		if (m_Desc.NavigationPreset == ECameraNavigationPreset::OrbitOnly)
		{
			m_pCameraController->SetMode(ECameraControllerNavigationMode::Orbit);
			return;
		}

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
		{
			m_pCameraController->SetMode(ECameraControllerNavigationMode::None);
			m_IsNavigating = false;
			m_CameraInput.MoveAxis = Vector3::Zero;
		}
	}
}