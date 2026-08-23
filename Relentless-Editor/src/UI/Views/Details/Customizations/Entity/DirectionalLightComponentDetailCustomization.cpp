#include "DirectionalLightComponentDetailCustomization.h"

#include "UI/Views/Details/DetailHelpers.h"
#include "UI/Views/Details/IDetailsView.h"
#include "UI/Views/Details/LayoutBuilders/IDetailLayoutBuilder.h"
#include "UI/Views/Details/LayoutBuilders/IDetailCategoryBuilder.h"
#include "UI/Views/Details/Context/EntityDetailsContext.h"

#include "Property/EntityPropertyHandle.h"

namespace Relentless
{
	//TODO: Go through property handle
	static Ref<HorizontalBox> OnBuildLightingChannelsRequested(EntityDetailsContext& aContext) noexcept
	{
		auto AllHaveChannelsSet = [&aContext](ELightChannel aLightChannel) -> bool
			{
				return std::ranges::all_of(aContext.Entities, [&aContext, aLightChannel](entity aEntity)
					{
						const DirectionalLightComponent& directionalLightComponent = aContext.EntityManager->Get<DirectionalLightComponent>(aEntity);
						return directionalLightComponent.HasAnyChannel(aLightChannel);
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
							DirectionalLightComponent& directionalLightComponent = aContext.EntityManager->Get<DirectionalLightComponent>(aEntity);
							directionalLightComponent.SetChannelEnabled(lightChannel, setChannel);
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
						const DirectionalLightComponent& directionalLightComponent = aContext.EntityManager->Get<DirectionalLightComponent>(aEntity);
						return directionalLightComponent.GetChannel() == ELightChannel::Channel1;
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
						DirectionalLightComponent& directionalLightComponent = aContext.EntityManager->Get<DirectionalLightComponent>(aEntity);
						directionalLightComponent.SetChannelEnabled(ELightChannel::All, false);
						directionalLightComponent.SetChannelEnabled(ELightChannel::Channel1, true);
					});
			});
		
		return pBox;
	}

	void DirectionalLightComponentDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& aDetailLayoutBuilder) noexcept
	{
		SetupConnections();

		using DLC = DirectionalLightComponent;

		IDetailsView* pDetailsView = aDetailLayoutBuilder.GetDetailsView();
		EntityDetailsContext& context = pDetailsView->GetContext<EntityDetailsContext>();
		DetailHelpers::EntityHandleFactory<DLC> handleFactory{ .Entities = context.Entities, .EntityManager = *context.EntityManager };
		const bool multiSelection = context.Entities.size() > 1u;

		auto pTypeHandle = handleFactory.MakeCustom<int>(
			[](MAYBE_UNUSED const DirectionalLightComponent& aDLC) { return static_cast<int>(ELightType::Directional); },
			[this, &context](entity aEntity, MAYBE_UNUSED DirectionalLightComponent& aDLC, const int& aSelection)
			{
				Application::Get().SubmitToMainThread([this, &context, aEntity, aSelection]()
					{
						const ELightType lightType = static_cast<ELightType>(aSelection);

						if (lightType == ELightType::Point)
							EntityUtils::ConvertLightType(aEntity, *context.EntityManager, ELightType::Directional, ELightType::Point);
						else 
							EntityUtils::ConvertLightType(aEntity, *context.EntityManager, ELightType::Directional, ELightType::Spot);

						if (IDetailsView* pDetailsView = GetDetailsView())
							pDetailsView->RequestRefresh();
					});
			});

		IDetailCategoryBuilder& categoryBuilder = aDetailLayoutBuilder.EditCategory(ICON_FA_LIGHTBULB "  Light");
		categoryBuilder.AddHeaderAction("Remove", [this]() { RemoveFromInspected(); });

		categoryBuilder.AddProperty<int>("Type", pTypeHandle)
			.NameSlot().Label("Type")
			.ValueSlot().ComboBox().Options({ "Directional", "Point", "Spot" }).Selected(0);

		auto pIntensityHandle = handleFactory.Make(&DLC::GetIntensity, &DLC::SetIntensityLux, 100'000.0f);
		auto intensityBuilder = categoryBuilder.AddProperty<float>("Intensity", pIntensityHandle);
		intensityBuilder.NameSlot().Label("Intensity");
		if (multiSelection)
			intensityBuilder.ValueSlot().NumericEntryBox().Range(0.0f, 120'000.0f).Unit(" lux");
		else 
			intensityBuilder.ValueSlot().Slider().Range(0.0f, 120'000.0f).Unit(" lux");

		auto pColorHandle = handleFactory.Make(&DLC::GetColor, &DLC::SetColor, Colors::White);
		categoryBuilder.AddProperty<Color>("Light Color", pColorHandle)
			.NameSlot().Label("Light Color")
			.ValueSlot().ColorPicker();

		auto pUseTemperatureHandle = handleFactory.Make(&DLC::IsUsingTemperature, &DLC::SetUseTemperature, false);
		categoryBuilder.AddProperty<bool>("Use Temperature", pUseTemperatureHandle)
			.NameSlot().Label("Use Temperature")
			.ValueSlot().CheckBox();

		auto pTemperatureHandle = handleFactory.Make(&DLC::GetTemperature, &DLC::SetTemperature, 6'500.0f);
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

		auto pCastShadowsHandle = handleFactory.Make(&DLC::IsCastingShadows, &DLC::SetCastShadows, true);
		shadowsGroupBuilder.AddProperty<bool>("Cast Shadows", pCastShadowsHandle)
			.NameSlot().Label("Cast Shadows")
			.ValueSlot().CheckBox();

		auto pShadowAmountHandle = handleFactory.Make(&DLC::GetShadowAmount, &DLC::SetShadowAmount, 1.0f);
		shadowsGroupBuilder.AddProperty<float>("Shadow Amount", pShadowAmountHandle)
			.NameSlot().Label("Shadow Amount")
			.ValueSlot().Slider().Range(0.0f, 1.0f);

		auto pShadowResolutionScaleHandle = handleFactory.Make(&DLC::GetShadowResolutionScale, &DLC::SetShadowResolutionScale, 1.0f);
		shadowsGroupBuilder.AddProperty<float>("Shadow Resolution Scale", pShadowResolutionScaleHandle)
			.NameSlot().Label("Shadow Resolution Scale")
			.ValueSlot().Slider().Range(0.125f, 8.0f);

		auto pShadowBiasHandle = handleFactory.Make(&DLC::GetShadowBias, &DLC::SetShadowBias, 0.0001f);
		shadowsGroupBuilder.AddProperty<float>("Shadow Bias", pShadowBiasHandle)
			.NameSlot().Label("Shadow Bias")
			.ValueSlot().Slider().Logarithmic(true).Range(0.0f, 0.1f);

		auto pShadowSlopeBiasHandle = handleFactory.Make(&DLC::GetShadowSlopeBias, &DLC::SetShadowSlopeBias, 0.0005f);
		shadowsGroupBuilder.AddProperty<float>("Shadow Slope Bias", pShadowSlopeBiasHandle)
			.NameSlot().Label("Shadow Slope Bias")
			.ValueSlot().Slider().Logarithmic(true).Range(0.0f, 0.1f);

		auto pNumCascadesHandle = handleFactory.Make(&DLC::GetNumCascades, &DLC::SetNumCascades, 4u);
		shadowsGroupBuilder.AddProperty<uint32>("Number of Cascades", pNumCascadesHandle)
			.NameSlot().Label("Number of Cascades")
			.ValueSlot().Slider().Range(1u, 4u);

		auto pCascadeDistributionHandle = handleFactory.Make(&DLC::GetCascadeDistribution, &DLC::SetCascadeDistribution, 0.85f);
		shadowsGroupBuilder.AddProperty<float>("Cascade Distribution", pCascadeDistributionHandle)
			.NameSlot().Label("Cascade Distribution")
			.ValueSlot().Slider().Range(0.0f, 1.0f);
	}

	void DirectionalLightComponentDetailCustomization::SetupConnections() noexcept
	{
		m_OnDirectionalLightComponentPropertyChangedConnection = ScopedConnection(CoreObjectBroadcasters::OnEntityComponentPropertyChanged,
			[this](entity aEntity, TypeIndex aComponentType, MAYBE_UNUSED IComponent* aComponent, uint64 aProperty)
			{
				if (aComponentType != DirectionalLightComponent::StaticType())
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