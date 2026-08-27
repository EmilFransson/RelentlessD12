#pragma once
#include <Relentless.h>

#include "Controller/PerspectiveCameraController.h"

#include "ViewportInputEvent.h"

namespace Relentless
{
	class ViewportSurface;

	class ViewportClient
	{
	public:
		struct Desc
		{
			Vector3 Location = Vector3(13.0f, 13.0f, -13.0f);
			Vector3 FocusTarget = Vector3::Zero;

			float NearPlane = 0.01f;
			float FarPlane = 1'000.0f;
			float VerticalFOV = Math::DegToRad(60.0f);

			ECameraNavigationPreset NavigationPreset = ECameraNavigationPreset::Default;
			ERenderViewMode ViewMode = ERenderViewMode::Lit;
			RenderFeatures RenderFeatures = RenderFeatures::Enabled();
			RenderQualitySettings RenderQualitySettings;
		};

		explicit ViewportClient(const Desc& aDesc) noexcept;
		virtual ~ViewportClient() = default;

		ViewportClient(const ViewportClient&) = delete;
		ViewportClient& operator=(const ViewportClient&) = delete;

		void DisableAllRenderFeatures() noexcept;

		void EnableAllRenderFeatures() noexcept;

		NO_DISCARD PerspectiveCamera& GetCamera() noexcept;
		NO_DISCARD const PerspectiveCamera& GetCamera() const noexcept;
		NO_DISCARD PerspectiveCameraController* GetCameraController() noexcept;
		NO_DISCARD const PerspectiveCameraController* GetCameraController() const noexcept;

		NO_DISCARD const RenderFeatures& GetRenderFeatures() const noexcept;
		NO_DISCARD const RenderQualitySettings& GetRenderQualitySettings() const noexcept;
		NO_DISCARD ERenderViewMode GetViewMode() const noexcept;

		virtual bool HandleInput(const ViewportInputEvent& aInputEvent);
		NO_DISCARD bool HasRenderFeature(ERenderFeature aRenderFeature) const noexcept;

		NO_DISCARD bool IsNavigating() const noexcept;
		NO_DISCARD bool IsNavigationEnabled() const noexcept;

		void SetMSAASamples(EMSAASampleCount aSamples) noexcept;
		void SetNavigationEnabled(bool aEnable) noexcept;
		void SetRenderFeature(ERenderFeature aRenderFeature, bool aEnabled) noexcept;
		void SetViewMode(ERenderViewMode aViewMode);

		void ToggleRenderFeature(ERenderFeature aRenderFeature) noexcept;

		virtual void Update(MAYBE_UNUSED float aDeltaTime);

		Broadcaster<void(ERenderViewMode)> OnViewModeChanged;
	private:
		void ResolveAndSetCameraMode() noexcept;
	private:
		Desc m_Desc;
		PerspectiveCameraController::Input m_CameraInput;

		UniquePtr<PerspectiveCamera> m_pCamera = nullptr;
		UniquePtr<PerspectiveCameraController> m_pCameraController = nullptr;

		bool m_IsNavigating = false;
		bool m_NavigationEnabled = true;
	};
}