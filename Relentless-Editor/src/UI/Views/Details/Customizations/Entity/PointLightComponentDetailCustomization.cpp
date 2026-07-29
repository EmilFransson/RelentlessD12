#include "PointLightComponentDetailCustomization.h"

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
						const PointLightComponent& pointLightComponent = aContext.EntityManager->Get<PointLightComponent>(aEntity);
						return pointLightComponent.HasAnyChannel(aLightChannel);
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
							PointLightComponent& pointLightComponent = aContext.EntityManager->Get<PointLightComponent>(aEntity);
							pointLightComponent.SetChannelEnabled(lightChannel, setChannel);
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
						const PointLightComponent& pointLightComponent = aContext.EntityManager->Get<PointLightComponent>(aEntity);
						return pointLightComponent.GetChannel() == ELightChannel::Channel1;
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
						PointLightComponent& pointLightComponent = aContext.EntityManager->Get<PointLightComponent>(aEntity);
						pointLightComponent.SetChannelEnabled(ELightChannel::All, false);
						pointLightComponent.SetChannelEnabled(ELightChannel::Channel1, true);
					});
			});

		return pBox;
	}

	void PointLightComponentDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& aDetailLayoutBuilder) noexcept
	{
		SetupConnections();

		using PLC = PointLightComponent;

		EntityDetailsContext& context = aDetailLayoutBuilder.GetDetailsView()->GetContext<EntityDetailsContext>();
		DetailHelpers::EntityHandleFactory<PLC> handleFactory({ .Entities = context.Entities, .EntityManager = *context.EntityManager });
		const bool multiSelection = context.Entities.size() > 1u;

		auto pTypeHandle = handleFactory.MakeCustom<int>(
			[](MAYBE_UNUSED const PointLightComponent& aPLC) { return static_cast<int>(ELightType::Point); },
			[&context, pDetailsView = aDetailLayoutBuilder.GetDetailsView()](entity aEntity, MAYBE_UNUSED PointLightComponent& aPLC, const int& aSelection)
			{
				Application::Get().SubmitToMainThread([&context, pDetailsView, aEntity, aSelection]()
					{
						const ELightType lightType = static_cast<ELightType>(aSelection);

						if (lightType == ELightType::Spot)
							EntityUtils::ConvertLightType(aEntity, *context.EntityManager, ELightType::Point, ELightType::Spot);
						else
							EntityUtils::ConvertLightType(aEntity, *context.EntityManager, ELightType::Point, ELightType::Directional);

						pDetailsView->RequestRefresh();
					});
			});

		IDetailCategoryBuilder& categoryBuilder = aDetailLayoutBuilder.EditCategory(ICON_FA_LIGHTBULB "  Light");
		
		categoryBuilder.AddProperty<int>("Type", pTypeHandle)
			.NameSlot().Label("Type")
			.ValueSlot().ComboBox().Options({ "Directional", "Point", "Spot" }).Selected(1);

		Ref<EntityPropertyHandle<float, PointLightComponent>> pIntensityHandle = RLS_NEW EntityPropertyHandle<float, PointLightComponent>(
			*context.EntityManager,
			context.Entities,
			[&context](const PointLightComponent& aPLC) 
			{ 
				if (context.LightIntensityType == ELightIntensityType::Candelas)
					return aPLC.GetIntensity();
				else
					return Math::Photometry::CandelaToLumen_Point(aPLC.GetIntensity());
			},
			[&context](entity, PointLightComponent& aPLC, const float& aIntensity)
			{
				if (context.LightIntensityType == ELightIntensityType::Candelas)
					aPLC.SetIntensityCandela(aIntensity);
				else
					aPLC.SetIntensityLumen(aIntensity);
			},
			context.LightIntensityType == ELightIntensityType::Candelas ? 8.0f : Math::Photometry::CandelaToLumen_Point(8.0f)
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
			.ValueSlot().ComboBox().Options({"Candelas", "Lumens"}).Selected(static_cast<int>(context.LightIntensityType));

		auto pColorHandle = handleFactory.Make(&PLC::GetColor, &PLC::SetColor, Colors::White);
		categoryBuilder.AddProperty<Color>("Light Color", pColorHandle)
			.NameSlot().Label("Light Color")
			.ValueSlot().ColorPicker();

		auto pAttenuationRadiusHandle = handleFactory.Make(&PLC::GetAttenuationRadius, &PLC::SetAttenuationRadius, 10.0f);
		auto attenuationBuilder = categoryBuilder.AddProperty<float>("Attenuation Radius", pAttenuationRadiusHandle);
		attenuationBuilder.NameSlot().Label("Attenuation Radius");
		if (multiSelection)
			attenuationBuilder.ValueSlot().NumericEntryBox().Range(0.08f, 163.48f).Unit(" m");
		else
			attenuationBuilder.ValueSlot().Slider().Range(0.08f, 163.48f).Unit(" m").Logarithmic(true);

		auto pUseTemperatureHandle = handleFactory.Make(&PLC::IsUsingTemperature, &PLC::SetUseTemperature, false);
		categoryBuilder.AddProperty<bool>("Use Temperature", pUseTemperatureHandle)
			.NameSlot().Label("Use Temperature")
			.ValueSlot().CheckBox();

		auto pTemperatureHandle = handleFactory.Make(&PLC::GetTemperature, &PLC::SetTemperature, 6'500.0f);
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

		auto pCastShadowsHandle = handleFactory.Make(&PLC::IsCastingShadows, &PLC::SetCastShadows, true);
		shadowsGroupBuilder.AddProperty<bool>("Cast Shadows", pCastShadowsHandle)
			.NameSlot().Label("Cast Shadows")
			.ValueSlot().CheckBox();

		auto pShadowAmountHandle = handleFactory.Make(&PLC::GetShadowAmount, &PLC::SetShadowAmount, 1.0f);
		shadowsGroupBuilder.AddProperty<float>("Shadow Amount", pShadowAmountHandle)
			.NameSlot().Label("Shadow Amount")
			.ValueSlot().Slider().Range(0.0f, 1.0f);

		auto pShadowResolutionScaleHandle = handleFactory.Make(&PLC::GetShadowResolutionScale, &PLC::SetShadowResolutionScale, 1.0f);
		shadowsGroupBuilder.AddProperty<float>("Shadow Resolution Scale", pShadowResolutionScaleHandle)
			.NameSlot().Label("Shadow Resolution Scale")
			.ValueSlot().Slider().Range(0.125f, 8.0f);

		auto pShadowBiasHandle = handleFactory.Make(&PLC::GetShadowBias, &PLC::SetShadowBias, 0.0001f);
		shadowsGroupBuilder.AddProperty<float>("Shadow Bias", pShadowBiasHandle)
			.NameSlot().Label("Shadow Bias")
			.ValueSlot().Slider().Logarithmic(true).Range(0.0f, 0.5f);

		auto pShadowSlopeBiasHandle = handleFactory.Make(&PLC::GetShadowSlopeBias, &PLC::SetShadowSlopeBias, 0.0005f);
		shadowsGroupBuilder.AddProperty<float>("Shadow Slope Bias", pShadowSlopeBiasHandle)
			.NameSlot().Label("Shadow Slope Bias")
			.ValueSlot().Slider().Logarithmic(true).Range(0.0f, 0.5f);
	}

	void PointLightComponentDetailCustomization::SetupConnections() noexcept
	{
		m_OnPointLightComponentPropertyChangedConnection = ScopedConnection(CoreObjectBroadcasters::OnEntityComponentPropertyChanged,
			[this](entity aEntity, TypeIndex aComponentType, MAYBE_UNUSED IComponent* aComponent, uint64 aProperty)
			{
				if (aComponentType != PointLightComponent::StaticType())
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