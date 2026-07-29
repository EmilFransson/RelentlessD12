#include "SpotLightComponentDetailCustomization.h"

#include <Relentless.h>

#include "UI/Views/Details/DetailHelpers.h"
#include "UI/Views/Details/IDetailsView.h"
#include "UI/Views/Details/LayoutBuilders/IDetailLayoutBuilder.h"
#include "UI/Views/Details/LayoutBuilders/IDetailCategoryBuilder.h"
#include "UI/Views/Details/Context/EntityDetailsContext.h"

#include "Property/EntityPropertyHandle.h"

namespace Relentless
{
	static Ref<HorizontalBox> OnBuildLightingChannelsRequested(EntityDetailsContext& aContext) noexcept
	{
		auto AllHaveChannelsSet = [&aContext](ELightChannel aLightChannel) -> bool
			{
				return std::ranges::all_of(aContext.Entities, [&aContext, aLightChannel](entity aEntity)
					{
						const SpotLightComponent& spotLightComponent = aContext.EntityManager->Get<SpotLightComponent>(aEntity);
						return spotLightComponent.HasAnyChannel(aLightChannel);
					});
			};

		Ref<HorizontalBox> pBox = RLS_NEW HorizontalBox();
		pBox->SetPadding({ 0.0f, 2.0f, 0.0f, 2.0f });
		pBox->SetSpacing(5.0f);

		for (uint32 i = 0; i <= 5; ++i)
		{
			const ELightChannel lightChannel = static_cast<ELightChannel>(1u << i);

			Button* pButton = pBox->AddWidget(RLS_NEW Button(std::format("{}", i)));
			pButton->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);
			pButton->SetHorizontalSizePolicy(ESizePolicy::Fixed);
			pButton->SetVerticalSizePolicy(ESizePolicy::Fixed);
			pButton->SetSize({ 20.0f, 20.0f });
			pButton->OnClicked([&aContext, lightChannel, AllHaveChannelsSet]()
				{
					const bool setChannel = !AllHaveChannelsSet(lightChannel);

					std::ranges::for_each(aContext.Entities, [&aContext, lightChannel, setChannel](entity aEntity)
						{
							SpotLightComponent& spotLightComponent = aContext.EntityManager->Get<SpotLightComponent>(aEntity);
							spotLightComponent.SetChannelEnabled(lightChannel, setChannel);
						});
				});

			pButton->OnMouseEnter([](Button* aButton) { aButton->SetTextColor(Colors::White); });
			pButton->OnMouseExit([lightChannel, AllHaveChannelsSet](Button* aButton)
				{
					if (AllHaveChannelsSet(lightChannel))
						return;

					aButton->SetTextColor(Colors::Gray);
				});

			const bool allSet = AllHaveChannelsSet(lightChannel);
			pButton->SetBackgroundColor(allSet ? Colors::Blue : Colors::Black);
			pButton->SetHoverColor(allSet ? Colors::Blue : Colors::Black);
			pButton->SetBorderColor(Colors::Normalize(50.0f, 50.0f, 50.0f, 255.0f));
			pButton->SetTextColor(allSet ? Colors::White : Colors::Gray);
		}

		return pBox;
	}

	static Ref<HorizontalBox> OnBuildLightingChannelsRevertButtonRequested(EntityDetailsContext& aContext) noexcept
	{
		auto AllHaveOnlyChannel1Set = [&aContext]() -> bool
			{
				return std::ranges::all_of(aContext.Entities, [&aContext](entity aEntity)
					{
						const SpotLightComponent& spotLightComponent = aContext.EntityManager->Get<SpotLightComponent>(aEntity);
						return spotLightComponent.GetChannel() == ELightChannel::Channel1;
					});
			};

		Ref<HorizontalBox> pBox = RLS_NEW HorizontalBox();
		pBox->SetPadding({ 0.0f, 2.0f, 0.0f, 2.0f });
		pBox->SetSpacing(5.0f);

		Button* pButton = pBox->AddWidget(Button::CreateTransparent(ICON_FA_ARROW_ROTATE_LEFT));
		pButton->SetTextColor(Color(1.0f, 1.0f, 1.0f, 0.5f));
		pButton->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);
		pButton->SetIsVisible(!AllHaveOnlyChannel1Set());

		pButton->OnMouseEnter([](Button* aButton) { aButton->SetTextColor(Color(1.0f, 1.0f, 1.0f, 1.0f)); });
		pButton->OnMouseExit([](Button* aButton) { aButton->SetTextColor(Color(1.0f, 1.0f, 1.0f, 0.5f)); });
		pButton->OnClicked([&aContext]()
			{
				std::ranges::for_each(aContext.Entities, [&aContext](const entity aEntity)
					{
						SpotLightComponent& spotLightComponent = aContext.EntityManager->Get<SpotLightComponent>(aEntity);
						spotLightComponent.SetChannelEnabled(ELightChannel::All, false);
						spotLightComponent.SetChannelEnabled(ELightChannel::Channel1, true);
					});
			});

		return pBox;
	}

	void SpotLightComponentDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& aDetailLayoutBuilder) noexcept
	{
		SetupConnections();

		using SLC = SpotLightComponent;

		EntityDetailsContext& context = aDetailLayoutBuilder.GetDetailsView()->GetContext<EntityDetailsContext>();
		DetailHelpers::EntityHandleFactory<SLC> handleFactory({ .Entities = context.Entities, .EntityManager = *context.EntityManager });
		const bool multiSelection = context.Entities.size() > 1u;

		auto pTypeHandle = handleFactory.MakeCustom<int>(
			[](MAYBE_UNUSED const SpotLightComponent& aSLC) { return static_cast<int>(ELightType::Spot); },
			[&context, pDetailsView = aDetailLayoutBuilder.GetDetailsView()](entity aEntity, MAYBE_UNUSED SpotLightComponent& aSLC, const int& aSelection)
			{
				Application::Get().SubmitToMainThread([&context, pDetailsView, aEntity, aSelection]()
					{
						const ELightType lightType = static_cast<ELightType>(aSelection);

						if (lightType == ELightType::Point)
							EntityUtils::ConvertLightType(aEntity, *context.EntityManager, ELightType::Spot, ELightType::Point);
						else
							EntityUtils::ConvertLightType(aEntity, *context.EntityManager, ELightType::Spot, ELightType::Directional);

						pDetailsView->RequestRefresh();
					});
			});

		IDetailCategoryBuilder& categoryBuilder = aDetailLayoutBuilder.EditCategory(ICON_FA_LIGHTBULB "  Light");

		categoryBuilder.AddProperty<int>("Type", pTypeHandle)
			.NameSlot().Label("Type")
			.ValueSlot().ComboBox().Options({ "Directional", "Point", "Spot" }).Selected(2);

		Ref<EntityPropertyHandle<float, SpotLightComponent>> pIntensityHandle = RLS_NEW EntityPropertyHandle<float, SpotLightComponent>(
			*context.EntityManager,
			context.Entities,
			[&context](const SpotLightComponent& aSLC)
			{
				if (context.LightIntensityType == ELightIntensityType::Candelas)
					return aSLC.GetIntensity();
				else
					return Math::Photometry::CandelaToLumen_Spot(aSLC.GetIntensity(), aSLC.GetOuterConeAngleRadians());
			},
			[&context](entity, SpotLightComponent& aSLC, const float& aIntensity)
			{
				if (context.LightIntensityType == ELightIntensityType::Candelas)
					aSLC.SetIntensityCandela(aIntensity);
				else
					aSLC.SetIntensityLumen(aIntensity);
			},
			context.LightIntensityType == ELightIntensityType::Candelas ? 8.0f : Math::Photometry::CandelaToLumen_Spot(8.0f, Math::DegToRad(44.0f)) 
		);

		const char* unit = context.LightIntensityType == ELightIntensityType::Candelas ? " cd" : " lm";
		const Vector2 range = context.LightIntensityType == ELightIntensityType::Candelas ? Vector2(0.0f, 160.0f) : Vector2(0.0f, 2'010.619263f);

		auto intensityBuilder = categoryBuilder.AddProperty<float>("Intensity", pIntensityHandle);
		intensityBuilder.NameSlot().Label("Intensity");
		if (multiSelection)
			intensityBuilder.ValueSlot().NumericEntryBox().Range(range.x, range.y).Unit(unit);
		else
			intensityBuilder.ValueSlot().Slider().Range(range.x, range.y).Unit(unit);

		Ref<PropertyHandle<int>> pIntensityUnitsHandle = RLS_NEW PropertyHandle<int>(
			[&context]() { return static_cast<int>(context.LightIntensityType); },
			[&context, pDetailsView = aDetailLayoutBuilder.GetDetailsView()](const int& aValue)
			{
				context.LightIntensityType = static_cast<ELightIntensityType>(aValue);
				pDetailsView->RequestRefresh();
			},
			0
		);

		categoryBuilder.AddProperty<int>("Intensity Units", pIntensityUnitsHandle)
			.NameSlot().Label("Intensity Units")
			.ValueSlot().ComboBox().Options({ "Candelas", "Lumens" }).Selected(static_cast<int>(context.LightIntensityType));

		auto pColorHandle = handleFactory.Make(&SLC::GetColor, &SLC::SetColor, Colors::White);
		categoryBuilder.AddProperty<Color>("Light Color", pColorHandle)
			.NameSlot().Label("Light Color")
			.ValueSlot().ColorPicker();

		auto pAttenuationRadiusHandle = handleFactory.Make(&SLC::GetAttenuationRadius, &SLC::SetAttenuationRadius, 10.0f);
		auto attenuationBuilder = categoryBuilder.AddProperty<float>("Attenuation Radius", pAttenuationRadiusHandle);
		attenuationBuilder.NameSlot().Label("Attenuation Radius");
		if (multiSelection)
			attenuationBuilder.ValueSlot().NumericEntryBox().Range(0.08f, 163.48f).Unit(" m");
		else
			attenuationBuilder.ValueSlot().Slider().Range(0.08f, 163.48f).Unit(" m").Logarithmic(true);

		auto pInnerConeAngleHandle = handleFactory.Make(&SLC::GetInnerConeAngleDegrees, &SLC::SetInnerConeAngleDegrees, 0.0f);
		auto innerConeAngleBuilder = categoryBuilder.AddProperty<float>("Inner Cone Angle", pInnerConeAngleHandle);
		innerConeAngleBuilder.NameSlot().Label("Inner Cone Angle");
		if (multiSelection)
			innerConeAngleBuilder.ValueSlot().NumericEntryBox().Range(0.0f, 80.0f).Unit("\xC2\xB0");
		else
			innerConeAngleBuilder.ValueSlot().Slider().Range(0.0f, 80.0f).Unit("\xC2\xB0");

		auto pOuterConeAngleHandle = handleFactory.Make(&SLC::GetOuterConeAngleDegrees, &SLC::SetOuterConeAngleDegrees, 44.0f);
		auto builder = categoryBuilder.AddProperty<float>("Outer Cone Angle", pOuterConeAngleHandle);
		builder.NameSlot().Label("Outer Cone Angle");
		if (multiSelection)
			builder.ValueSlot().NumericEntryBox().Range(0.0f, 80.0f).Unit("\xC2\xB0");
		else
			builder.ValueSlot().Slider().Range(0.0f, 80.0f).Unit("\xC2\xB0");

		auto pUseTemperatureHandle = handleFactory.Make(&SLC::IsUsingTemperature, &SLC::SetUseTemperature, false);
		categoryBuilder.AddProperty<bool>("Use Temperature", pUseTemperatureHandle)
			.NameSlot().Label("Use Temperature")
			.ValueSlot().CheckBox();

		auto pTemperatureHandle = handleFactory.Make(&SLC::GetTemperature, &SLC::SetTemperature, 6'500.0f);
		auto temperatureBuilder = categoryBuilder.AddProperty<float>("Temperature", pTemperatureHandle);
		temperatureBuilder.NameSlot().Label("Temperature");
		if (multiSelection)
			temperatureBuilder.ValueSlot().NumericEntryBox().Range(1'700.0f, 12'000.0f).Unit(" K").Enabled(pUseTemperatureHandle->AllEqualTo(true));
		else
			temperatureBuilder.ValueSlot().Slider().Range(1'700.0f, 12'000.0f).Unit(" K").Enabled(pUseTemperatureHandle->AllEqualTo(true));

		categoryBuilder.AddProperty<bool>("Lighting Channels", nullptr)
			.NameSlot().Label("Lighting Channels")
			.ValueSlot().Widget([&context]() { return OnBuildLightingChannelsRequested(context); })
			.RevertSlot().Widget([&context]() { return OnBuildLightingChannelsRevertButtonRequested(context); });

		IDetailGroupBuilder shadowsGroupBuilder = categoryBuilder.EditGroup("Shadows");

		auto pCastShadowsHandle = handleFactory.Make(&SLC::IsCastingShadows, &SLC::SetCastShadows, true);
		shadowsGroupBuilder.AddProperty<bool>("Cast Shadows", pCastShadowsHandle)
			.NameSlot().Label("Cast Shadows")
			.ValueSlot().CheckBox();

		auto pShadowAmountHandle = handleFactory.Make(&SLC::GetShadowAmount, &SLC::SetShadowAmount, 1.0f);
		shadowsGroupBuilder.AddProperty<float>("Shadow Amount", pShadowAmountHandle)
			.NameSlot().Label("Shadow Amount")
			.ValueSlot().Slider().Range(0.0f, 1.0f);

		auto pShadowResolutionScaleHandle = handleFactory.Make(&SLC::GetShadowResolutionScale, &SLC::SetShadowResolutionScale, 1.0f);
		shadowsGroupBuilder.AddProperty<float>("Shadow Resolution Scale", pShadowResolutionScaleHandle)
			.NameSlot().Label("Shadow Resolution Scale")
			.ValueSlot().Slider().Range(0.125f, 8.0f);

		auto pShadowBiasHandle = handleFactory.Make(&SLC::GetShadowBias, &SLC::SetShadowBias, 0.0001f);
		shadowsGroupBuilder.AddProperty<float>("Shadow Bias", pShadowBiasHandle)
			.NameSlot().Label("Shadow Bias")
			.ValueSlot().Slider().Logarithmic(true).Range(0.0f, 5.0f);

		auto pShadowSlopeBiasHandle = handleFactory.Make(&SLC::GetShadowSlopeBias, &SLC::SetShadowSlopeBias, 0.0005f);
		shadowsGroupBuilder.AddProperty<float>("Shadow Slope Bias", pShadowSlopeBiasHandle)
			.NameSlot().Label("Shadow Slope Bias")
			.ValueSlot().Slider().Logarithmic(true).Range(0.0f, 5.0f);
	}

	void SpotLightComponentDetailCustomization::SetupConnections() noexcept
	{
		m_OnSpotLightComponentPropertyChangedConnection = ScopedConnection(CoreObjectBroadcasters::OnEntityComponentPropertyChanged,
			[this](entity aEntity, TypeIndex aComponentType, MAYBE_UNUSED IComponent* aComponent, uint64 aProperty)
			{
				if (aComponentType != SpotLightComponent::StaticType())
					return;
				if (!IsEntityInspected(aEntity))
					return;
				if (aProperty == "m_LightChannel"_h || aProperty == "m_UseTemperature"_h)
				{
					if (IDetailsView* pDetailsView = GetDetailsView())
						pDetailsView->RequestRefresh();
				}
			});
	}
}