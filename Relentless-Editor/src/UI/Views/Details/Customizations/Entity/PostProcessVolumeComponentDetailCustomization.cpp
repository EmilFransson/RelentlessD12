#include "PostProcessVolumeComponentDetailCustomization.h"

#include <Relentless.h>

#include "Core/Editor.h"

#include "UI/Views/Details/IDetailsView.h"
#include "UI/Views/Details/LayoutBuilders/IDetailLayoutBuilder.h"
#include "UI/Views/Details/LayoutBuilders/IDetailCategoryBuilder.h"
#include "UI/Views/Details/Context/EntityDetailsContext.h"

#include "Property/EntityPropertyHandle.h"

#include "Subsystem/EngineContentSubsystem.h"

namespace Relentless
{
	void PostProcessVolumeComponentDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& aDetailLayoutBuilder) noexcept
	{
		IDetailsView* pDetailsView = aDetailLayoutBuilder.GetDetailsView();
		EntityDetailsContext& context = pDetailsView->GetContext<EntityDetailsContext>();
		IDetailCategoryBuilder& categoryBuilder = aDetailLayoutBuilder.EditCategory(ICON_FA_PAINTBRUSH "  Post Process");

		Ref<EntityPropertyHandle<bool, PostProcessVolumeComponent>> pInfiniteExtentHandle = RLS_NEW EntityPropertyHandle<bool, PostProcessVolumeComponent>(
			*context.EntityManager,
			context.Entities,
			[](const PostProcessVolumeComponent& aPPVC) { return aPPVC.HasInfiniteExtent(); },
			[](entity, PostProcessVolumeComponent&, const bool&) { },
			false
		);

		categoryBuilder.AddProperty<bool>("Infinite Extent", pInfiniteExtentHandle)
			.NameSlot().Label("Infinite Extent")
			.ValueSlot().CheckBox().Enabled(false);

		CustomizeAmbientOcclusionDetails(categoryBuilder, context, pDetailsView);
		CustomizeBloomDetails(categoryBuilder, context, pDetailsView);
		CustomizeExposureDetails(categoryBuilder, context);
	}

	bool PostProcessVolumeComponentDetailCustomization::ShouldCustomize(IDetailLayoutBuilder& aDetailLayoutBuilder) const noexcept
	{
		const EntityDetailsContext& context = aDetailLayoutBuilder.GetDetailsView()->GetContext<EntityDetailsContext>();
		if (context.Entities.empty())
			return false;

		return std::ranges::all_of(context.Entities, [&context](entity aEntity) { return context.EntityManager->Has<PostProcessVolumeComponent>(aEntity); });
	}

	void PostProcessVolumeComponentDetailCustomization::CustomizeAmbientOcclusionDetails(IDetailCategoryBuilder& aCategoryBuilder, EntityDetailsContext& aContext, IDetailsView* aDetailsView) noexcept
	{
		IDetailGroupBuilder aoGroupBuilder = aCategoryBuilder.EditGroup("Ambient Occlusion");
		aoGroupBuilder.m_IsExpanded = false;

		//Enabled:
		{
			Ref<EntityPropertyHandle<bool, PostProcessVolumeComponent>> pHandle = RLS_NEW EntityPropertyHandle<bool, PostProcessVolumeComponent>(
				*aContext.EntityManager,
				aContext.Entities,
				[](const PostProcessVolumeComponent& aPPVC) { return aPPVC.GetAmbientOcclusion().IsEnabled(); },
				[](entity, PostProcessVolumeComponent& aPPVC, const bool& aEnabled) { aPPVC.GetAmbientOcclusion().SetEnabled(aEnabled); },
				true
			);

			aoGroupBuilder.AddProperty<bool>("Enabled", pHandle)
				.NameSlot().Label("Enabled")
				.ValueSlot().CheckBox();
		}

		//Radius:
		{
			Ref<EntityPropertyHandle<float, PostProcessVolumeComponent>> pHandle = RLS_NEW EntityPropertyHandle<float, PostProcessVolumeComponent>(
				*aContext.EntityManager,
				aContext.Entities,
				[](const PostProcessVolumeComponent& aPPVC) { return aPPVC.GetAmbientOcclusion().GetRadius(); },
				[](entity, PostProcessVolumeComponent& aPPVC, const float& aRadius) { aPPVC.GetAmbientOcclusion().SetRadius(aRadius); },
				1.0f
			);

			aoGroupBuilder.AddProperty<float>("Radius", pHandle)
				.NameSlot().Label("Radius")
				.ValueSlot().SpinBox().Range(0.0f, FLT_MAX).Delta(0.01f);
		}

		//Bias:
		{
			Ref<EntityPropertyHandle<float, PostProcessVolumeComponent>> pHandle = RLS_NEW EntityPropertyHandle<float, PostProcessVolumeComponent>(
				*aContext.EntityManager,
				aContext.Entities,
				[](const PostProcessVolumeComponent& aPPVC) { return aPPVC.GetAmbientOcclusion().GetBias(); },
				[](entity, PostProcessVolumeComponent& aPPVC, const float& aBias) { aPPVC.GetAmbientOcclusion().SetBias(aBias); },
				0.1f
			);

			aoGroupBuilder.AddProperty<float>("Bias", pHandle)
				.NameSlot().Label("Bias")
				.ValueSlot().Slider().Range(0.0f, 0.5f);
		}

		//Power Exponent:
		{
			Ref<EntityPropertyHandle<float, PostProcessVolumeComponent>> pHandle = RLS_NEW EntityPropertyHandle<float, PostProcessVolumeComponent>(
				*aContext.EntityManager,
				aContext.Entities,
				[](const PostProcessVolumeComponent& aPPVC) { return aPPVC.GetAmbientOcclusion().GetPowerExponent(); },
				[](entity, PostProcessVolumeComponent& aPPVC, const float& aExponent) { aPPVC.GetAmbientOcclusion().SetPowerExponent(aExponent); },
				2.0f
			);

			aoGroupBuilder.AddProperty<float>("Power Exponent", pHandle)
				.NameSlot().Label("Power Exponent")
				.ValueSlot().Slider().Range(1.0f, 4.0f);
		}

		//Blur Enabled:
		{
			Ref<EntityPropertyHandle<bool, PostProcessVolumeComponent>> pHandle = RLS_NEW EntityPropertyHandle<bool, PostProcessVolumeComponent>(
				*aContext.EntityManager,
				aContext.Entities,
				[](const PostProcessVolumeComponent& aPPVC) { return aPPVC.GetAmbientOcclusion().IsBlurEnabled(); },
				[aDetailsView](entity, PostProcessVolumeComponent& aPPVC, const bool& aEnabled)
				{ 
					aPPVC.GetAmbientOcclusion().SetBlurEnabled(aEnabled); 
					aDetailsView->RequestRefresh();
				},
				true
			);

			aoGroupBuilder.AddProperty<bool>("Blur Enabled", pHandle)
				.NameSlot().Label("Blur Enabled")
				.ValueSlot().CheckBox();
		}

		//Blur Sharpness:
		{
			Ref<EntityPropertyHandle<float, PostProcessVolumeComponent>> pHandle = RLS_NEW EntityPropertyHandle<float, PostProcessVolumeComponent>(
				*aContext.EntityManager,
				aContext.Entities,
				[](const PostProcessVolumeComponent& aPPVC) { return aPPVC.GetAmbientOcclusion().GetBlurSharpness(); },
				[](entity, PostProcessVolumeComponent& aPPVC, const float& aBlurSharpness) { aPPVC.GetAmbientOcclusion().SetBlurSharpness(aBlurSharpness); },
				16.0f
			);

			aoGroupBuilder.AddProperty<float>("Blur Sharpness", pHandle)
				.NameSlot().Label("Blur Sharpness")
				.ValueSlot().Slider().Range(0.0f, 16.0f).Enabled(std::ranges::all_of(aContext.Entities, [aContext](entity aEntity) { return aContext.EntityManager->Get<PostProcessVolumeComponent>(aEntity).GetAmbientOcclusion().IsBlurEnabled(); }));
		}

		//Blur Radius:
		{
			Ref<EntityPropertyHandle<int, PostProcessVolumeComponent>> pHandle = RLS_NEW EntityPropertyHandle<int, PostProcessVolumeComponent>(
				*aContext.EntityManager,
				aContext.Entities,
				[](const PostProcessVolumeComponent& aPPVC) { return static_cast<int>(aPPVC.GetAmbientOcclusion().GetBlurRadius()) == 2 ? 0 : 1; },
				[](entity, PostProcessVolumeComponent& aPPVC, const int& aEnumValue) { aPPVC.GetAmbientOcclusion().SetBlurRadius(aEnumValue == 0 ? EAmbientOcclusionBlurRadius::_2 : EAmbientOcclusionBlurRadius::_4); },
				1
			);

			aoGroupBuilder.AddProperty<int>("Blur Radius", pHandle)
				.NameSlot().Label("Blur Radius")
				.ValueSlot().ComboBox().Options({ "2", "4" }).Enabled(std::ranges::all_of(aContext.Entities, [aContext](entity aEntity) { return aContext.EntityManager->Get<PostProcessVolumeComponent>(aEntity).GetAmbientOcclusion().IsBlurEnabled(); }));
		}

		//Depth Precision:
		{
			Ref<EntityPropertyHandle<int, PostProcessVolumeComponent>> pHandle = RLS_NEW EntityPropertyHandle<int, PostProcessVolumeComponent>(
				*aContext.EntityManager,
				aContext.Entities,
				[](const PostProcessVolumeComponent& aPPVC) { return static_cast<int>(aPPVC.GetAmbientOcclusion().GetDepthPrecision()) == 16 ? 0 : 1; },
				[](entity, PostProcessVolumeComponent& aPPVC, const int& aEnumValue) { aPPVC.GetAmbientOcclusion().SetDepthPrecision(aEnumValue == 0 ? EAmbientOcclusionDepthPrecision::F16 : EAmbientOcclusionDepthPrecision::F32); },
				1
			);

			aoGroupBuilder.AddProperty<int>("Depth Precision", pHandle)
				.NameSlot().Label("Depth Precision")
				.ValueSlot().ComboBox().Options({ "16", "32" });
		}

		//Step Count:
		{
			Ref<EntityPropertyHandle<int, PostProcessVolumeComponent>> pHandle = RLS_NEW EntityPropertyHandle<int, PostProcessVolumeComponent>(
				*aContext.EntityManager,
				aContext.Entities,
				[](const PostProcessVolumeComponent& aPPVC) { return static_cast<int>(aPPVC.GetAmbientOcclusion().GetStepCount()) == 4 ? 0 : 1; },
				[](entity, PostProcessVolumeComponent& aPPVC, const int& aEnumValue) { aPPVC.GetAmbientOcclusion().SetStepCount(aEnumValue == 0 ? EAmbientOcclusionStepcount::_4 : EAmbientOcclusionStepcount::_8); },
				1
			);

			aoGroupBuilder.AddProperty<int>("Step Count", pHandle)
				.NameSlot().Label("Step Count")
				.ValueSlot().ComboBox().Options({ "4", "8" });
		}
	}

	void PostProcessVolumeComponentDetailCustomization::CustomizeBloomDetails(IDetailCategoryBuilder& aCategoryBuilder, EntityDetailsContext& aContext, IDetailsView* aDetailsView) noexcept
	{
		IDetailGroupBuilder bloomGroupBuilder = aCategoryBuilder.EditGroup("Bloom");
		bloomGroupBuilder.m_IsExpanded = false;

		//Intensity:
		{
			Ref<EntityPropertyHandle<float, PostProcessVolumeComponent>> pIntensityHandle = RLS_NEW EntityPropertyHandle<float, PostProcessVolumeComponent>(
				*aContext.EntityManager,
				aContext.Entities,
				[](const PostProcessVolumeComponent& aPPVC) { return aPPVC.GetBloom().GetIntensity(); },
				[](entity, PostProcessVolumeComponent& aPPVC, const float& aIntensity) { aPPVC.GetBloom().SetIntensity(aIntensity); },
				1.0f
			);

			bloomGroupBuilder.AddProperty<float>("Intensity", pIntensityHandle)
				.NameSlot().Label("Intensity")
				.ValueSlot().Slider().Range(0.0f, 8.0f);
		}

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

		//Dirt Mask Intensity:
		{
			Ref<EntityPropertyHandle<float, PostProcessVolumeComponent>> pDirtMaskIntensityHandle = RLS_NEW EntityPropertyHandle<float, PostProcessVolumeComponent>(
				*aContext.EntityManager,
				aContext.Entities,
				[](const PostProcessVolumeComponent& aPPVC) { return aPPVC.GetBloom().GetDirtMaskIntensity(); },
				[](entity, PostProcessVolumeComponent& aPPVC, const float& aIntensity) { aPPVC.GetBloom().SetDirtMaskIntensity(aIntensity); },
				0.0f
			);

			bloomGroupBuilder.AddProperty<float>("Dirt Mask Intensity", pDirtMaskIntensityHandle)
				.NameSlot().Label("Dirt Mask Intensity")
				.ValueSlot().Slider().Range(0.0f, 8.0f);
		}

		//Dirt Mask Tint
		{
			Ref<EntityPropertyHandle<Color, PostProcessVolumeComponent>> pDirtMaskTintHandle = RLS_NEW EntityPropertyHandle<Color, PostProcessVolumeComponent>(
				*aContext.EntityManager,
				aContext.Entities,
				[](const PostProcessVolumeComponent& aPPVC) { return aPPVC.GetBloom().GetDirtMaskTint(); },
				[](entity, PostProcessVolumeComponent& aPPVC, const Color& aColor) { aPPVC.GetBloom().SetDirtMaskTint(aColor); },
				Colors::Gray
			);

			bloomGroupBuilder.AddProperty<Color>("Dirt Mask Tint", pDirtMaskTintHandle)
				.NameSlot().Label("Dirt Mask Tint")
				.ValueSlot().ColorPicker();
		}
	}

	void PostProcessVolumeComponentDetailCustomization::CustomizeExposureDetails(IDetailCategoryBuilder& aCategoryBuilder, EntityDetailsContext& aContext) noexcept
	{
		IDetailGroupBuilder exposureGroupBuilder = aCategoryBuilder.EditGroup("Exposure");
		exposureGroupBuilder.m_IsExpanded = false;

		//Exposure Compensation:
		{
			Ref<EntityPropertyHandle<float, PostProcessVolumeComponent>> pExposureCompensationHandle = RLS_NEW EntityPropertyHandle<float, PostProcessVolumeComponent>(
				*aContext.EntityManager,
				aContext.Entities,
				[](const PostProcessVolumeComponent& aPPVC) { return aPPVC.GetExposure().GetCompensation(); },
				[](entity, PostProcessVolumeComponent& aPPVC, const float& aCompensation) { aPPVC.GetExposure().SetCompensation(aCompensation); },
				1.0f
			);

			exposureGroupBuilder.AddProperty<float>("Exposure Compensation", pExposureCompensationHandle)
				.NameSlot().Label("Exposure Compensation")
				.ValueSlot().Slider().Range(-15.0f, 15.0f);
		}

		//Min EV100:
		{
			Ref<EntityPropertyHandle<float, PostProcessVolumeComponent>> pMinEV100Handle = RLS_NEW EntityPropertyHandle<float, PostProcessVolumeComponent>(
				*aContext.EntityManager,
				aContext.Entities,
				[](const PostProcessVolumeComponent& aPPVC) { return aPPVC.GetExposure().GetMinEV100(); },
				[](entity, PostProcessVolumeComponent& aPPVC, const float& aMinEV100) { aPPVC.GetExposure().SetMinEV100(aMinEV100); },
				-10.0f
			);

			exposureGroupBuilder.AddProperty<float>("Min EV100", pMinEV100Handle)
				.NameSlot().Label("Min EV100")
				.ValueSlot().Slider().Range(-10.0f, 20.0f);
		}

		//Max EV100:
		{
			Ref<EntityPropertyHandle<float, PostProcessVolumeComponent>> pMaxEV100Handle = RLS_NEW EntityPropertyHandle<float, PostProcessVolumeComponent>(
				*aContext.EntityManager,
				aContext.Entities,
				[](const PostProcessVolumeComponent& aPPVC) { return aPPVC.GetExposure().GetMaxEV100(); },
				[](entity, PostProcessVolumeComponent& aPPVC, const float& aMaxEV100) { aPPVC.GetExposure().SetMaxEV100(aMaxEV100); },
				20.0f
			);

			exposureGroupBuilder.AddProperty<float>("Max EV100", pMaxEV100Handle)
				.NameSlot().Label("Max EV100")
				.ValueSlot().Slider().Range(-10.0f, 20.0f);
		}

		//Speed Up:
		{
			Ref<EntityPropertyHandle<float, PostProcessVolumeComponent>> pSpeedUpHandle = RLS_NEW EntityPropertyHandle<float, PostProcessVolumeComponent>(
				*aContext.EntityManager,
				aContext.Entities,
				[](const PostProcessVolumeComponent& aPPVC) { return aPPVC.GetExposure().GetSpeedUp(); },
				[](entity, PostProcessVolumeComponent& aPPVC, const float& aSpeedUp) { aPPVC.GetExposure().SetSpeedUp(aSpeedUp); },
				3.0f
			);

			exposureGroupBuilder.AddProperty<float>("Speed Up", pSpeedUpHandle)
				.NameSlot().Label("Speed Up")
				.ValueSlot().Slider().Range(0.02f, 20.0f);
		}

		//Speed Down:
		{
			Ref<EntityPropertyHandle<float, PostProcessVolumeComponent>> pSpeedUpHandle = RLS_NEW EntityPropertyHandle<float, PostProcessVolumeComponent>(
				*aContext.EntityManager,
				aContext.Entities,
				[](const PostProcessVolumeComponent& aPPVC) { return aPPVC.GetExposure().GetSpeedDown(); },
				[](entity, PostProcessVolumeComponent& aPPVC, const float& aSpeedDown) { aPPVC.GetExposure().SetSpeedDown(aSpeedDown); },
				1.0f
			);

			exposureGroupBuilder.AddProperty<float>("Speed Down", pSpeedUpHandle)
				.NameSlot().Label("Speed Down")
				.ValueSlot().Slider().Range(0.02f, 20.0f);
		}

		//Low Percent:
		{
			Ref<EntityPropertyHandle<float, PostProcessVolumeComponent>> pLowPercentHandle = RLS_NEW EntityPropertyHandle<float, PostProcessVolumeComponent>(
				*aContext.EntityManager,
				aContext.Entities,
				[](const PostProcessVolumeComponent& aPPVC) { return aPPVC.GetExposure().GetLowPercent(); },
				[](entity, PostProcessVolumeComponent& aPPVC, const float& aLowPercent) { aPPVC.GetExposure().SetLowPercent(aLowPercent); },
				10.0f
			);

			exposureGroupBuilder.AddProperty<float>("Low Percent", pLowPercentHandle)
				.NameSlot().Label("Low percent")
				.ValueSlot().Slider().Range(0.0f, 100.0f);
		}

		//High Percent:
		{
			Ref<EntityPropertyHandle<float, PostProcessVolumeComponent>> pHighPercentHandle = RLS_NEW EntityPropertyHandle<float, PostProcessVolumeComponent>(
				*aContext.EntityManager,
				aContext.Entities,
				[](const PostProcessVolumeComponent& aPPVC) { return aPPVC.GetExposure().GetHighPercent(); },
				[](entity, PostProcessVolumeComponent& aPPVC, const float& aHighPercent) { aPPVC.GetExposure().SetHighPercent(aHighPercent); },
				90.0f
			);

			exposureGroupBuilder.AddProperty<float>("High Percent", pHighPercentHandle)
				.NameSlot().Label("High percent")
				.ValueSlot().Slider().Range(0.0f, 100.0f);
		}

		//Histogram Min EV100:
		{
			Ref<EntityPropertyHandle<float, PostProcessVolumeComponent>> pHistogramMinEV100Handle = RLS_NEW EntityPropertyHandle<float, PostProcessVolumeComponent>(
				*aContext.EntityManager,
				aContext.Entities,
				[](const PostProcessVolumeComponent& aPPVC) { return aPPVC.GetExposure().GetHistogramMinEV100(); },
				[](entity, PostProcessVolumeComponent& aPPVC, const float& aHistogramMinEV100) { aPPVC.GetExposure().SetHistogramMinEV100(aHistogramMinEV100); },
				-10.0f
			);

			exposureGroupBuilder.AddProperty<float>("Histogram Min EV100", pHistogramMinEV100Handle)
				.NameSlot().Label("Histogram Min EV100")
				.ValueSlot().Slider().Range(-16.0f, 0.0f);
		}

		//Histogram Max EV100:
		{
			Ref<EntityPropertyHandle<float, PostProcessVolumeComponent>> pHistogramMaxEV100Handle = RLS_NEW EntityPropertyHandle<float, PostProcessVolumeComponent>(
				*aContext.EntityManager,
				aContext.Entities,
				[](const PostProcessVolumeComponent& aPPVC) { return aPPVC.GetExposure().GetHistogramMaxEV100(); },
				[](entity, PostProcessVolumeComponent& aPPVC, const float& aHistogramMaxEV100) { aPPVC.GetExposure().SetHistogramMaxEV100(aHistogramMaxEV100); },
				20.0f
			);

			exposureGroupBuilder.AddProperty<float>("Histogram Max EV100", pHistogramMaxEV100Handle)
				.NameSlot().Label("Histogram Max EV100")
				.ValueSlot().Slider().Range(0.0f, 20.0f);
		}
	}

}