#include "ViewportCameraDetailCustomization.h"

#include "UI/Views/Details/IDetailsView.h"
#include "UI/Views/Details/LayoutBuilders/DetailPropertyRowBuilder.h"
#include "UI/Views/Details/LayoutBuilders/IDetailLayoutBuilder.h"
#include "UI/Views/Details/ViewportDetailsContext.h"

namespace Relentless
{
	void ViewportCameraDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& aDetailLayoutBuilder) noexcept
	{
		ViewportDetailsContext& detailsContext = aDetailLayoutBuilder.GetDetailsView()->GetContext<ViewportDetailsContext>();
		if (!detailsContext.CameraController)
			return;

		IDetailCategoryBuilder& categoryBuilder = aDetailLayoutBuilder.EditCategory(ICON_FA_CAMERA "  Camera");

		categoryBuilder.AddProperty<float>("Speed Multiplier",
			[cameraController = detailsContext.CameraController]() { return cameraController->GetSpeedMultiplier(); },
			[cameraController = detailsContext.CameraController](const float& aValue) { cameraController->SetSpeedMultiplier(aValue); },
			1.0f)
		.Slider()
			.Range(detailsContext.CameraController->GetMinSpeedMultiplierLimit(), detailsContext.CameraController->GetMaxSpeedMultiplierLimit());
		
		categoryBuilder.AddProperty<float>("Field of View (H)",
			[cameraController = detailsContext.CameraController]() { return Math::RadToDeg(cameraController->GetHorizontalFoV()); },
			[cameraController = detailsContext.CameraController](const float& aValue) { cameraController->SetHorizontalFoV(Math::DegToRad(aValue)); },
			60.0f)
		.Unit("\xC2\xB0")
		.Slider()
			.Range(5.0f, 170.0f);
		
		categoryBuilder.AddProperty<float>("Near View Plane",
			[cameraController = detailsContext.CameraController]() { return cameraController->GetNearPlane(); },
			[cameraController = detailsContext.CameraController](const float& aValue) { cameraController->SetNearPlane(aValue); },
			0.1f)
		.Unit("m")
		.Slider()
			.Range(0.01f, 100'000.0f)
			.Logarithmic();
		
		categoryBuilder.AddProperty<float>("Far View Plane",
			[cameraController = detailsContext.CameraController]() { return cameraController->GetFarPlane(); },
			[cameraController = detailsContext.CameraController](const float& aValue) { cameraController->SetFarPlane(aValue); },
			1'000.0f)
		.Unit("m")
		.Slider()
			.Range(0.01f, 100'000.0f)
			.Logarithmic();
	}
}