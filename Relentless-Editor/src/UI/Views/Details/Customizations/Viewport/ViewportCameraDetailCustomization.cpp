#include "ViewportCameraDetailCustomization.h"

#include "UI/Views/Details/IDetailsView.h"
#include "UI/Views/Details/LayoutBuilders/DetailPropertyRowBuilder.h"
#include "UI/Views/Details/LayoutBuilders/IDetailLayoutBuilder.h"
#include "UI/Views/Details/Context/ViewportDetailsContext.h"

namespace Relentless
{
	void ViewportCameraDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& aDetailLayoutBuilder) noexcept
	{
		ViewportDetailsContext& detailsContext = aDetailLayoutBuilder.GetDetailsView()->GetContext<ViewportDetailsContext>();
		PerspectiveCameraController* pCameraController = detailsContext.ViewportPanel->GetClient().GetCameraController();
		if (!pCameraController)
			return;

		IDetailCategoryBuilder& cameraCategoryBuilder = aDetailLayoutBuilder.EditCategory(ICON_FA_CAMERA "  Camera");

		cameraCategoryBuilder.AddProperty<float>("Speed Multiplier",
			[pCameraController]() { return pCameraController->GetSpeedMultiplier(); },
			[pCameraController](const float& aValue) { pCameraController->SetSpeedMultiplier(aValue); },
			1.0f)
		.NameSlot().Label("Speed Multiplier")
		.ValueSlot().Slider().Range(pCameraController->GetMinSpeedMultiplierLimit(), pCameraController->GetMaxSpeedMultiplierLimit());
		
		cameraCategoryBuilder.AddProperty<float>("Field of View (H)",
			[pCameraController]() { return Math::RadToDeg(pCameraController->GetHorizontalFoV()); },
			[pCameraController](const float& aValue) { pCameraController->SetHorizontalFoV(Math::DegToRad(aValue)); },
			60.0f)
		.NameSlot().Label("Field of View (H)")
		.ValueSlot().Slider().Unit("\xC2\xB0").Range(5.0f, 170.0f);
		
		cameraCategoryBuilder.AddProperty<float>("Near View Plane",
			[pCameraController]() { return pCameraController->GetNearPlane(); },
			[pCameraController](const float& aValue) { pCameraController->SetNearPlane(aValue); },
			0.1f)
		.NameSlot().Label("Near View Plane")
		.ValueSlot().Slider().Unit("m").Range(0.01f, 100'000.0f).Logarithmic(true);
		
		cameraCategoryBuilder.AddProperty<float>("Far View Plane",
			[pCameraController]() { return pCameraController->GetFarPlane(); },
			[pCameraController](const float& aValue) { pCameraController->SetFarPlane(aValue); },
			1'000.0f)
		.NameSlot().Label("Far View Plane")
		.ValueSlot().Slider().Unit("m").Range(0.01f, 100'000.0f).Logarithmic(true);
	}
}