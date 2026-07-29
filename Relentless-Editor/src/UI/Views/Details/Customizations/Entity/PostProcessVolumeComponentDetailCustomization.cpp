#include "PostProcessVolumeComponentDetailCustomization.h"

#include <Relentless.h>

#include "Core/Editor.h"

#include "UI/Views/Details/DetailHelpers.h"
#include "UI/Views/Details/IDetailsView.h"
#include "UI/Views/Details/LayoutBuilders/IDetailLayoutBuilder.h"
#include "UI/Views/Details/LayoutBuilders/IDetailCategoryBuilder.h"
#include "UI/Views/Details/Context/EntityDetailsContext.h"

#include "Property/EntityPropertyHandle.h"

#include "Subsystem/EngineContentSubsystem.h"

namespace Relentless
{
	using PPVC = PostProcessVolumeComponent;
	
	void PostProcessVolumeComponentDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& aDetailLayoutBuilder) noexcept
	{
		IDetailsView* pDetailsView = aDetailLayoutBuilder.GetDetailsView();
		EntityDetailsContext& context = pDetailsView->GetContext<EntityDetailsContext>();
		DetailHelpers::EntityHandleFactory<PPVC> handleFactory({ .Entities = context.Entities, .EntityManager = *context.EntityManager });

		IDetailCategoryBuilder& categoryBuilder = aDetailLayoutBuilder.EditCategory(ICON_FA_PAINTBRUSH "  Post Process");

		auto pInfiniteExtentHandle = handleFactory.Make(&PPVC::HasInfiniteExtent, &PPVC::SetHasInfiniteExtent, false);
		categoryBuilder.AddProperty<bool>("Infinite Extent", pInfiniteExtentHandle)
			.NameSlot().Label("Infinite Extent")
			.ValueSlot().CheckBox().Enabled(false);

		CustomizeAmbientOcclusionDetails(categoryBuilder, context, pDetailsView);
		CustomizeBloomDetails(categoryBuilder, context, pDetailsView);
		CustomizeExposureDetails(categoryBuilder, context);
	}

	void PostProcessVolumeComponentDetailCustomization::CustomizeAmbientOcclusionDetails(IDetailCategoryBuilder& aCategoryBuilder, EntityDetailsContext& aContext, IDetailsView* aDetailsView) noexcept
	{
		using AO = AmbientOcclusionSettings;

		DetailHelpers::EntityHandleFactory<PPVC> handleFactory{ .Entities = aContext.Entities, .EntityManager = *aContext.EntityManager };
		auto aoHandleFactory = handleFactory.MakeSubFactory([](auto& aComponent) -> auto& { return aComponent.GetAmbientOcclusion(); });
		
		IDetailGroupBuilder aoGroupBuilder = aCategoryBuilder.EditGroup("Ambient Occlusion");
		aoGroupBuilder.m_IsExpanded = false;

		auto pEnabledHandle = aoHandleFactory.Make(&AO::IsEnabled, &AO::SetEnabled, true);
		aoGroupBuilder.AddProperty<bool>("Enabled", pEnabledHandle)
			.NameSlot().Label("Enabled")
			.ValueSlot().CheckBox();

		auto pRadiusHandle = aoHandleFactory.Make(&AO::GetRadius, &AO::SetRadius, 1.0f);
		aoGroupBuilder.AddProperty<float>("Radius", pRadiusHandle)
			.NameSlot().Label("Radius")
			.ValueSlot().SpinBox().Range(0.0f, FLT_MAX).Delta(0.01f);

		auto pBiasHandle = aoHandleFactory.Make(&AO::GetBias, &AO::SetBias, 0.1f);
		aoGroupBuilder.AddProperty<float>("Bias", pBiasHandle)
			.NameSlot().Label("Bias")
			.ValueSlot().Slider().Range(0.0f, 0.5f);

		auto pPowerExponentHandle = aoHandleFactory.Make(&AO::GetPowerExponent, &AO::SetPowerExponent, 2.0f);
		aoGroupBuilder.AddProperty<float>("Power Exponent", pPowerExponentHandle)
			.NameSlot().Label("Power Exponent")
			.ValueSlot().Slider().Range(1.0f, 4.0f);

		auto pBlurEnabledHandle = handleFactory.MakeCustom(
			[](const PostProcessVolumeComponent& aPPVC) { return aPPVC.GetAmbientOcclusion().IsBlurEnabled(); },
			[aDetailsView](entity, PostProcessVolumeComponent& aPPVC, const bool& aEnabled) 
			{  
				aPPVC.GetAmbientOcclusion().SetBlurEnabled(aEnabled);
				aDetailsView->RequestRefresh(); 
			},
			true);
		
		aoGroupBuilder.AddProperty<bool>("Blur Enabled", pBlurEnabledHandle)
			.NameSlot().Label("Blur Enabled")
			.ValueSlot().CheckBox();

		auto pBlurSharpnessHandle = aoHandleFactory.Make(&AO::GetBlurSharpness, &AO::SetBlurSharpness, 16.0f);
		aoGroupBuilder.AddProperty<float>("Blur Sharpness", pBlurSharpnessHandle)
			.NameSlot().Label("Blur Sharpness")
			.ValueSlot().Slider().Range(0.0f, 16.0f).Enabled(std::ranges::all_of(aContext.Entities, [aContext](entity aEntity) { return aContext.EntityManager->Get<PostProcessVolumeComponent>(aEntity).GetAmbientOcclusion().IsBlurEnabled(); }));

		auto pBlurRadiusHandle = handleFactory.MakeCustom(
			[](const PostProcessVolumeComponent& aPPVC) { return  static_cast<int>(aPPVC.GetAmbientOcclusion().GetBlurRadius()) == 2 ? 0 : 1; },
			[](entity, PostProcessVolumeComponent& aPPVC, const int& aEnumValue) { aPPVC.GetAmbientOcclusion().SetBlurRadius(aEnumValue == 0 ? EAmbientOcclusionBlurRadius::_2 : EAmbientOcclusionBlurRadius::_4); },
			1);

		aoGroupBuilder.AddProperty<int>("Blur Radius", pBlurRadiusHandle)
			.NameSlot().Label("Blur Radius")
			.ValueSlot().ComboBox().Options({ "2", "4" }).Enabled(std::ranges::all_of(aContext.Entities, [aContext](entity aEntity) { return aContext.EntityManager->Get<PostProcessVolumeComponent>(aEntity).GetAmbientOcclusion().IsBlurEnabled(); }));

		auto pDepthPrecisionHandle = handleFactory.MakeCustom(
			[](const PostProcessVolumeComponent& aPPVC) { return  static_cast<int>(aPPVC.GetAmbientOcclusion().GetDepthPrecision()) == 16 ? 0 : 1; },
			[](entity, PostProcessVolumeComponent& aPPVC, const int& aEnumValue) { aPPVC.GetAmbientOcclusion().SetDepthPrecision(aEnumValue == 0 ? EAmbientOcclusionDepthPrecision::F16 : EAmbientOcclusionDepthPrecision::F32); },
			1);

		aoGroupBuilder.AddProperty<int>("Depth Precision", pDepthPrecisionHandle)
			.NameSlot().Label("Depth Precision")
			.ValueSlot().ComboBox().Options({ "16", "32" });

		auto pStepCountHandle = handleFactory.MakeCustom(
			[](const PostProcessVolumeComponent& aPPVC) { return  static_cast<int>(aPPVC.GetAmbientOcclusion().GetStepCount()) == 4 ? 0 : 1; },
			[](entity, PostProcessVolumeComponent& aPPVC, const int& aEnumValue) { aPPVC.GetAmbientOcclusion().SetStepCount(aEnumValue == 0 ? EAmbientOcclusionStepcount::_4 : EAmbientOcclusionStepcount::_8); },
			1);

		aoGroupBuilder.AddProperty<int>("Step Count", pStepCountHandle)
			.NameSlot().Label("Step Count")
			.ValueSlot().ComboBox().Options({ "4", "8" });
	}

	void PostProcessVolumeComponentDetailCustomization::CustomizeBloomDetails(IDetailCategoryBuilder& aCategoryBuilder, EntityDetailsContext& aContext, IDetailsView* aDetailsView) noexcept
	{
		using Bloom = BloomSettings;

		DetailHelpers::EntityHandleFactory<PPVC> handleFactory{ .Entities = aContext.Entities, .EntityManager = *aContext.EntityManager };
		auto bloomHandleFactory = handleFactory.MakeSubFactory([](auto& aComponent) -> auto& { return aComponent.GetBloom(); });

		IDetailGroupBuilder bloomGroupBuilder = aCategoryBuilder.EditGroup("Bloom");
		bloomGroupBuilder.m_IsExpanded = false;

		auto pIntensityHandle = bloomHandleFactory.Make(&Bloom::GetIntensity, &Bloom::SetIntensity, 1.0f);
		bloomGroupBuilder.AddProperty<float>("Intensity", pIntensityHandle)
			.NameSlot().Label("Intensity")
			.ValueSlot().Slider().Range(0.0f, 8.0f);

		//Dirt Mask:
		{
			AssetRegistryModule& assetRegistry = ModuleManager::LoadModuleChecked<AssetRegistryModule>();
			EngineContentSubsystem* pEngineContentSubsystem = Editor::Get()->GetSubsystem<EngineContentSubsystem>();

			PostProcessVolumeComponent& postProcessComponent = aContext.EntityManager->Get<PostProcessVolumeComponent>(aContext.Entities.front());
			const AssetHandle& assetHandle = postProcessComponent.GetBloom().GetDirtMaskHandle();
			AssetData* pAssetData = assetRegistry.FindAsset(assetHandle.Uuid);

			const bool isNone = pAssetData == nullptr;
			if (isNone)
				pAssetData = assetRegistry.FindAsset(pEngineContentSubsystem->GetNoneTexture2DHandle().Uuid);

			bloomGroupBuilder.AddAssetProperty("Dirt Mask", *pAssetData)
				.AcceptableAssetTypes({ Texture2D::StaticType() })
				.OnAssetsDropped([&aContext, aDetailsView](Span<const AssetData> someAssetDatas)
					{
						const AssetHandle assetHandle = AssetManager::LoadAsset(someAssetDatas[0]);
						std::ranges::for_each(aContext.Entities, [&aContext, &assetHandle](entity aEntity) { aContext.EntityManager->Get<PostProcessVolumeComponent>(aEntity).GetBloom().SetDirtMask(assetHandle); });
						Application::Get().SubmitToMainThread([aDetailsView]() { aDetailsView->RequestRefresh(); });
					})
				.NameSlot().Label("Dirt Mask")
				.ValueSlot().AssetThumbnail().Row()
				.RevertSlot().Widget([isNone, aDetailsView, &aContext]()
					{
						Ref<HorizontalBox> pRevertBox = RLS_NEW HorizontalBox();
						pRevertBox->SetPadding({ 0.0f, 2.0f, 0.0f, 2.0f });

						if (!isNone)
						{
							Button* pButton = pRevertBox->AddWidget( Button::CreateTransparent(ICON_FA_ARROW_ROTATE_LEFT));
							pButton->SetTextColor(Color(1.0f, 1.0f, 1.0f, 0.5f));
							pButton->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);

							pButton->OnMouseEnter([](Button* aButton) { aButton->SetTextColor(Color(1.0f, 1.0f, 1.0f, 1.0f)); });
							pButton->OnMouseExit([](Button* aButton) { aButton->SetTextColor(Color(1.0f, 1.0f, 1.0f, 0.5f)); });
							pButton->OnClicked([aDetailsView, &aContext]()
								{
									std::ranges::for_each(aContext.Entities, [&aContext](entity aEntity) { aContext.EntityManager->Get<PostProcessVolumeComponent>(aEntity).GetBloom().RemoveDirtMask(); });
									Application::Get().SubmitToMainThread([aDetailsView]() { aDetailsView->RequestRefresh(); });
								});
						}

						return pRevertBox;
					});
		}

		auto pDirtMaskIntensityHandle = bloomHandleFactory.Make(&Bloom::GetDirtMaskIntensity, &Bloom::SetDirtMaskIntensity, 0.0f);
		bloomGroupBuilder.AddProperty<float>("Dirt Mask Intensity", pDirtMaskIntensityHandle)
			.NameSlot().Label("Dirt Mask Intensity")
			.ValueSlot().Slider().Range(0.0f, 8.0f);

		auto pDirtMaskTintHandle = bloomHandleFactory.Make(&Bloom::GetDirtMaskTint, &Bloom::SetDirtMaskTint, Colors::Gray);
		bloomGroupBuilder.AddProperty<Color>("Dirt Mask Tint", pDirtMaskTintHandle)
			.NameSlot().Label("Dirt Mask Tint")
			.ValueSlot().ColorPicker();
	}

	void PostProcessVolumeComponentDetailCustomization::CustomizeExposureDetails(IDetailCategoryBuilder& aCategoryBuilder, EntityDetailsContext& aContext) noexcept
	{
		using Exposure = ExposureSettings;

		DetailHelpers::EntityHandleFactory<PPVC> handleFactory{ .Entities = aContext.Entities, .EntityManager = *aContext.EntityManager };
		auto exposureHandleFactory = handleFactory.MakeSubFactory([](auto& aComponent) -> auto& { return aComponent.GetExposure(); });

		IDetailGroupBuilder exposureGroupBuilder = aCategoryBuilder.EditGroup("Exposure");
		exposureGroupBuilder.m_IsExpanded = false;

		auto pCompensationHandle = exposureHandleFactory.Make(&Exposure::GetCompensation, &Exposure::SetCompensation, 1.0f);
		exposureGroupBuilder.AddProperty<float>("Exposure Compensation", pCompensationHandle)
			.NameSlot().Label("Exposure Compensation")
			.ValueSlot().Slider().Range(-15.0f, 15.0f);

		auto pMinEV100Handle = exposureHandleFactory.Make(&Exposure::GetMinEV100, &Exposure::SetMinEV100, -10.0f);
		exposureGroupBuilder.AddProperty<float>("Min EV100", pMinEV100Handle)
			.NameSlot().Label("Min EV100")
			.ValueSlot().Slider().Range(-10.0f, 20.0f);

		auto pMaxEV100Handle = exposureHandleFactory.Make(&Exposure::GetMaxEV100, &Exposure::SetMaxEV100, 20.0f);
		exposureGroupBuilder.AddProperty<float>("Max EV100", pMaxEV100Handle)
			.NameSlot().Label("Max EV100")
			.ValueSlot().Slider().Range(-10.0f, 20.0f);

		auto pSpeedUpHandle = exposureHandleFactory.Make(&Exposure::GetSpeedUp, &Exposure::SetSpeedUp, 3.0f);
		exposureGroupBuilder.AddProperty<float>("Speed Up", pSpeedUpHandle)
			.NameSlot().Label("Speed Up")
			.ValueSlot().Slider().Range(0.02f, 20.0f);

		auto pSpeedDownHandle = exposureHandleFactory.Make(&Exposure::GetSpeedDown, &Exposure::SetSpeedDown, 1.0f);
		exposureGroupBuilder.AddProperty<float>("Speed Down", pSpeedDownHandle)
			.NameSlot().Label("Speed Down")
			.ValueSlot().Slider().Range(0.02f, 20.0f);

		auto pLowPercentHandle = exposureHandleFactory.Make(&Exposure::GetLowPercent, &Exposure::SetLowPercent, 10.0f);
		exposureGroupBuilder.AddProperty<float>("Low Percent", pLowPercentHandle)
			.NameSlot().Label("Low percent")
			.ValueSlot().Slider().Range(0.0f, 100.0f);

		auto pHighPercentHandle = exposureHandleFactory.Make(&Exposure::GetHighPercent, &Exposure::SetHighPercent, 90.0f);
		exposureGroupBuilder.AddProperty<float>("High Percent", pHighPercentHandle)
			.NameSlot().Label("High percent")
			.ValueSlot().Slider().Range(0.0f, 100.0f);

		auto pHistogramMinEV100Handle = exposureHandleFactory.Make(&Exposure::GetHistogramMinEV100, &Exposure::SetHistogramMinEV100, -10.0f);
		exposureGroupBuilder.AddProperty<float>("Histogram Min EV100", pHistogramMinEV100Handle)
			.NameSlot().Label("Histogram Min EV100")
			.ValueSlot().Slider().Range(-16.0f, 0.0f);

		auto pHistogramMaxEV100Handle = exposureHandleFactory.Make(&Exposure::GetHistogramMaxEV100, &Exposure::SetHistogramMaxEV100, 20.0f);
		exposureGroupBuilder.AddProperty<float>("Histogram Max EV100", pHistogramMaxEV100Handle)
			.NameSlot().Label("Histogram Max EV100")
			.ValueSlot().Slider().Range(0.0f, 20.0f);
	}

}