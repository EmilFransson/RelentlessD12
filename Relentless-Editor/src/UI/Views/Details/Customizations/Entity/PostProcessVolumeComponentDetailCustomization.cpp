#include "PostProcessVolumeComponentDetailCustomization.h"

#include <Relentless.h>

#include "UI/Views/Details/IDetailsView.h"
#include "UI/Views/Details/LayoutBuilders/IDetailLayoutBuilder.h"
#include "UI/Views/Details/LayoutBuilders/IDetailCategoryBuilder.h"
#include "UI/Views/Details/Context/EntityDetailsContext.h"

#include "Property/EntityPropertyHandle.h"

namespace Relentless
{
	void PostProcessVolumeComponentDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& aDetailLayoutBuilder) noexcept
	{
		EntityDetailsContext& context = aDetailLayoutBuilder.GetDetailsView()->GetContext<EntityDetailsContext>();
		const bool multiSelection = context.Entities.size() > 1u;

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

		//Exposure Compensation:
		{
			Ref<EntityPropertyHandle<float, PostProcessVolumeComponent>> pExposureCompensationHandle = RLS_NEW EntityPropertyHandle<float, PostProcessVolumeComponent>(
				*context.EntityManager,
				context.Entities,
				[](const PostProcessVolumeComponent& aPPVC) { return aPPVC.GetExposure().GetCompensation(); },
				[](entity, PostProcessVolumeComponent& aPPVC, const float& aCompensation) { aPPVC.GetExposure().SetCompensation(aCompensation); },
				1.0f
			);

			categoryBuilder.AddProperty<float>("Exposure Compensation", pExposureCompensationHandle)
				.NameSlot().Label("Exposure Compensation")
				.ValueSlot().Slider().Range(-15.0f, 15.0f);
		}

		//Min EV100:
		{
			Ref<EntityPropertyHandle<float, PostProcessVolumeComponent>> pMinEV100Handle = RLS_NEW EntityPropertyHandle<float, PostProcessVolumeComponent>(
				*context.EntityManager,
				context.Entities,
				[](const PostProcessVolumeComponent& aPPVC) { return aPPVC.GetExposure().GetMinEV100(); },
				[](entity, PostProcessVolumeComponent& aPPVC, const float& aMinEV100) { aPPVC.GetExposure().SetMinEV100(aMinEV100); },
				-10.0f
			);

			categoryBuilder.AddProperty<float>("Min EV100", pMinEV100Handle)
				.NameSlot().Label("Min EV100")
				.ValueSlot().Slider().Range(-10.0f, 20.0f);
		}

		//Max EV100:
		{
			Ref<EntityPropertyHandle<float, PostProcessVolumeComponent>> pMaxEV100Handle = RLS_NEW EntityPropertyHandle<float, PostProcessVolumeComponent>(
				*context.EntityManager,
				context.Entities,
				[](const PostProcessVolumeComponent& aPPVC) { return aPPVC.GetExposure().GetMaxEV100(); },
				[](entity, PostProcessVolumeComponent& aPPVC, const float& aMaxEV100) { aPPVC.GetExposure().SetMaxEV100(aMaxEV100); },
				20.0f
			);

			categoryBuilder.AddProperty<float>("Max EV100", pMaxEV100Handle)
				.NameSlot().Label("Max EV100")
				.ValueSlot().Slider().Range(-10.0f, 20.0f);
		}
		
		//Speed Up:
		{
			Ref<EntityPropertyHandle<float, PostProcessVolumeComponent>> pSpeedUpHandle = RLS_NEW EntityPropertyHandle<float, PostProcessVolumeComponent>(
				*context.EntityManager,
				context.Entities,
				[](const PostProcessVolumeComponent& aPPVC) { return aPPVC.GetExposure().GetSpeedUp(); },
				[](entity, PostProcessVolumeComponent& aPPVC, const float& aSpeedUp) { aPPVC.GetExposure().SetSpeedUp(aSpeedUp); },
				3.0f
			);

			categoryBuilder.AddProperty<float>("Speed Up", pSpeedUpHandle)
				.NameSlot().Label("Speed Up")
				.ValueSlot().Slider().Range(0.02f, 20.0f);
		}

		//Speed Down:
		{
			Ref<EntityPropertyHandle<float, PostProcessVolumeComponent>> pSpeedUpHandle = RLS_NEW EntityPropertyHandle<float, PostProcessVolumeComponent>(
				*context.EntityManager,
				context.Entities,
				[](const PostProcessVolumeComponent& aPPVC) { return aPPVC.GetExposure().GetSpeedDown(); },
				[](entity, PostProcessVolumeComponent& aPPVC, const float& aSpeedDown) { aPPVC.GetExposure().SetSpeedDown(aSpeedDown); },
				1.0f
			);

			categoryBuilder.AddProperty<float>("Speed Down", pSpeedUpHandle)
				.NameSlot().Label("Speed Down")
				.ValueSlot().Slider().Range(0.02f, 20.0f);
		}

		//Low Percent:
		{
			Ref<EntityPropertyHandle<float, PostProcessVolumeComponent>> pLowPercentHandle = RLS_NEW EntityPropertyHandle<float, PostProcessVolumeComponent>(
				*context.EntityManager,
				context.Entities,
				[](const PostProcessVolumeComponent& aPPVC) { return aPPVC.GetExposure().GetLowPercent(); },
				[](entity, PostProcessVolumeComponent& aPPVC, const float& aLowPercent) { aPPVC.GetExposure().SetLowPercent(aLowPercent); },
				10.0f
			);

			categoryBuilder.AddProperty<float>("Low Percent", pLowPercentHandle)
				.NameSlot().Label("Low percent")
				.ValueSlot().Slider().Range(0.0f, 100.0f);
		}

		//High Percent:
		{
			Ref<EntityPropertyHandle<float, PostProcessVolumeComponent>> pHighPercentHandle = RLS_NEW EntityPropertyHandle<float, PostProcessVolumeComponent>(
				*context.EntityManager,
				context.Entities,
				[](const PostProcessVolumeComponent& aPPVC) { return aPPVC.GetExposure().GetHighPercent(); },
				[](entity, PostProcessVolumeComponent& aPPVC, const float& aHighPercent) { aPPVC.GetExposure().SetHighPercent(aHighPercent); },
				90.0f
			);

			categoryBuilder.AddProperty<float>("High Percent", pHighPercentHandle)
				.NameSlot().Label("High percent")
				.ValueSlot().Slider().Range(0.0f, 100.0f);
		}

		//Histogram Min EV100:
		{
			Ref<EntityPropertyHandle<float, PostProcessVolumeComponent>> pHistogramMinEV100Handle = RLS_NEW EntityPropertyHandle<float, PostProcessVolumeComponent>(
				*context.EntityManager,
				context.Entities,
				[](const PostProcessVolumeComponent& aPPVC) { return aPPVC.GetExposure().GetHistogramMinEV100(); },
				[](entity, PostProcessVolumeComponent& aPPVC, const float& aHistogramMinEV100) { aPPVC.GetExposure().SetHistogramMinEV100(aHistogramMinEV100); },
				-10.0f
			);

			categoryBuilder.AddProperty<float>("Histogram Min EV100", pHistogramMinEV100Handle)
				.NameSlot().Label("Histogram Min EV100")
				.ValueSlot().Slider().Range(-16.0f, 0.0f);
		}

		//Histogram Max EV100:
		{
			Ref<EntityPropertyHandle<float, PostProcessVolumeComponent>> pHistogramMaxEV100Handle = RLS_NEW EntityPropertyHandle<float, PostProcessVolumeComponent>(
				*context.EntityManager,
				context.Entities,
				[](const PostProcessVolumeComponent& aPPVC) { return aPPVC.GetExposure().GetHistogramMaxEV100(); },
				[](entity, PostProcessVolumeComponent& aPPVC, const float& aHistogramMaxEV100) { aPPVC.GetExposure().SetHistogramMaxEV100(aHistogramMaxEV100); },
				20.0f
			);

			categoryBuilder.AddProperty<float>("Histogram Max EV100", pHistogramMaxEV100Handle)
				.NameSlot().Label("Histogram Max EV100")
				.ValueSlot().Slider().Range(0.0f, 20.0f);
		}
	}

	bool PostProcessVolumeComponentDetailCustomization::ShouldCustomize(IDetailLayoutBuilder& aDetailLayoutBuilder) const noexcept
	{
		const EntityDetailsContext& context = aDetailLayoutBuilder.GetDetailsView()->GetContext<EntityDetailsContext>();
		if (context.Entities.empty())
			return false;

		return std::ranges::all_of(context.Entities, [&context](entity aEntity) { return context.EntityManager->Has<PostProcessVolumeComponent>(aEntity); });
	}
}