#include "SkyLightComponentDetailCustomization.h"

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
						const SkyLightComponent& skyLightComponent = aContext.EntityManager->Get<SkyLightComponent>(aEntity);
						return skyLightComponent.HasLightChannelsEnabled(aLightChannel);
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
			pButton->OnClicked([&aContext, lightChannel, pButton, AllHaveChannelsSet]()
				{
					const bool setChannel = !AllHaveChannelsSet(lightChannel);

					std::ranges::for_each(aContext.Entities, [&aContext, lightChannel, setChannel](entity aEntity)
						{
							SkyLightComponent& skyLightComponent = aContext.EntityManager->Get<SkyLightComponent>(aEntity);
							skyLightComponent.SetLightChannelEnabled(lightChannel, setChannel);
						});

					pButton->SetBackgroundColor(setChannel ? Colors::Blue : Colors::Black);
					pButton->SetHoverColor(setChannel ? Colors::Blue : Colors::Black);

					if (setChannel)
						pButton->SetTextColor(Colors::White);
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
						const SkyLightComponent& skyLightComponent = aContext.EntityManager->Get<SkyLightComponent>(aEntity);
						return skyLightComponent.GetLightChannels() == ELightChannel::Channel1;
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
						SkyLightComponent& skyLightComponent = aContext.EntityManager->Get<SkyLightComponent>(aEntity);
						skyLightComponent.SetLightChannelEnabled(ELightChannel::All, false);
						skyLightComponent.SetLightChannelEnabled(ELightChannel::Channel1, true);
					});
			});

		return pBox;
	}

	void SkyLightComponentDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& aDetailLayoutBuilder) noexcept
	{
		SetupConnections();

		using SLC = SkyLightComponent;
		EntityDetailsContext& context = aDetailLayoutBuilder.GetDetailsView()->GetContext<EntityDetailsContext>();
		DetailHelpers::EntityHandleFactory<SLC> handleFactory{ .Entities = context.Entities, .EntityManager = *context.EntityManager };
		const bool multiSelection = context.Entities.size() > 1u;

		IDetailCategoryBuilder& categoryBuilder = aDetailLayoutBuilder.EditCategory(ICON_FA_CLOUD_SUN "  Sky Light");

		Ref<PropertyHandle<bool>> pIsActiveSkyLightHandle = RLS_NEW PropertyHandle<bool>([&context]()
			{
				return context.Scene->GetActiveSkyLight() == context.Entities.front();
			}, 
			[&context](const bool& aIsChecked)
			{
				if (aIsChecked)
					context.Scene->SetActiveSkyLight(context.Entities.front());
				else
					context.Scene->RemoveActiveSkyLight();
			});

		categoryBuilder.AddProperty<bool>("Is Active", pIsActiveSkyLightHandle)
			.NameSlot().Label("Is Active")
			.ValueSlot().CheckBox().Enabled(!multiSelection);

		auto pCaptureModeHandle = handleFactory.MakeCustom(
			[](const SkyLightComponent& aSLC) { return static_cast<int>(aSLC.GetCaptureMode()); },
			[](entity, SkyLightComponent& aSLC, const int& aCaptureMode) { aSLC.SetCaptureMode(static_cast<ESkyLightCaptureMode>(aCaptureMode)); },
			static_cast<int>(ESkyLightCaptureMode::Static));

		SkyLightComponent& skyLightComponent = context.EntityManager->Get<SkyLightComponent>(context.Entities.front());
		const AssetHandle& assetHandle = skyLightComponent.GetPrimaryEnvironmentHandle();
		AssetData* pAssetData = ModuleManager::LoadModuleChecked<AssetRegistryModule>().FindAsset(assetHandle.Uuid);

		categoryBuilder.AddAssetProperty("Primary Environment", *pAssetData)
			.AcceptableAssetTypes({ Environment::StaticType() })
			.OnAssetsDropped([&context](Span<const AssetData> someAssetDatas)
				{
					const AssetHandle assetHandle = AssetManager::LoadAsset(someAssetDatas[0]);
					std::ranges::for_each(context.Entities, [&context, &assetHandle](entity aEntity) { context.EntityManager->Get<SkyLightComponent>(aEntity).SetPrimaryEnvironment(assetHandle); });
				})
			.NameSlot().Label("Primary Environment")
			.ValueSlot().AssetThumbnail();

		categoryBuilder.AddProperty<int>("Capture Mode", pCaptureModeHandle)
			.NameSlot().Label("Capture Mode")
			.ValueSlot().ComboBox().Options({"Static", "Realtime"});

		auto pRadianceSizeHandle = handleFactory.MakeCustom(
			[](const SkyLightComponent& aSLC) { return aSLC.GetRadianceMapSize(); },
			[](entity, SkyLightComponent& aSLC, const uint32& aRadianceSize) 
			{   
				const uint32 oldSize = aSLC.GetRadianceMapSize();
				uint32 newSize = aRadianceSize;

				if ((newSize % 2u) != 0u)
				{
					if (aRadianceSize < oldSize)
						newSize = oldSize / 2u;
					else if (aRadianceSize > oldSize)
						newSize = oldSize * 2u;
				
					newSize = Math::NearestPowerOfTwo(newSize);
				}
				aSLC.SetRadianceMapSize(newSize); },
			256u);

		categoryBuilder.AddProperty<uint32>("Cubemap Resolution", pRadianceSizeHandle)
			.NameSlot().Label("Cubemap Resolution")
			.ValueSlot().SpinBox().Range(SkyLightComponent::MIN_RADIANCE_MAP_SIZE, SkyLightComponent::MAX_RADIANCE_MAP_SIZE);

		auto pRealtimeMipsPerFrameHandle = handleFactory.Make(&SLC::GetRealtimeMipsPerFrame, &SLC::SetRealtimeMipsPerFrame, 1u);
		categoryBuilder.AddProperty<uint32>("Realtime Mips Per Frame", pRealtimeMipsPerFrameHandle)
			.NameSlot().Label("Realtime Mips Per Frame")
			.ValueSlot().SpinBox().Delta(1u).Range(0u, 1'000'000u);

		auto pIntensityHandle = handleFactory.Make(&SLC::GetIntensity, &SLC::SetIntensity, 1.0f);
		categoryBuilder.AddProperty<float>("Intensity", pIntensityHandle)
			.NameSlot().Label("Intensity")
			.ValueSlot().SpinBox().Range(0.0f, FLT_MAX).Delta(0.01f);

		auto pColorHandle = handleFactory.Make(&SLC::GetTintColor, &SLC::SetTintColor, Colors::White);
		categoryBuilder.AddProperty<Color>("Color", pColorHandle)
			.NameSlot().Label("Color")
			.ValueSlot().ColorPicker();

		categoryBuilder.AddProperty<bool>("Lighting Channels", nullptr)
			.NameSlot().Label("Lighting Channels")
			.ValueSlot().Widget([&context]() { return OnBuildLightingChannelsRequested(context); })
			.RevertSlot().Widget([&context]() { return OnBuildLightingChannelsRevertButtonRequested(context); });

		IDetailGroupBuilder groupBuilder = categoryBuilder.EditGroup("Advanced");

		groupBuilder.AddAssetProperty("Blend Environment", *pAssetData)
			.AcceptableAssetTypes({ Environment::StaticType() })
			.OnAssetsDropped([&context](Span<const AssetData> someAssetDatas)
				{
					const AssetHandle assetHandle = AssetManager::LoadAsset(someAssetDatas[0]);
					std::ranges::for_each(context.Entities, [&context, &assetHandle](entity aEntity) { context.EntityManager->Get<SkyLightComponent>(aEntity).SetBlendEnvironment(assetHandle); });
				})
			.NameSlot().Label("Blend Environment")
			.ValueSlot().AssetThumbnail();

		auto pBlendHandle = handleFactory.Make(&SLC::GetBlendFactor, &SLC::SetBlendFactor, 0.0f);
		groupBuilder.AddProperty<float>("Blend Factor", pBlendHandle)
			.NameSlot().Label("Blend Factor")
			.ValueSlot().Slider().Range(0.0f, 1.0f);

		auto pLowerHemisphereHandle = handleFactory.MakeCustom(
			[](const SkyLightComponent& aSLC){ return static_cast<int>(aSLC.GetLowerHemisphereMode()); },
			[](entity, SkyLightComponent& aSLC, const int& aLowerHemisphereMode){ aSLC.SetLowerHemisphereMode(static_cast<ESkyLightLowerHemisphereMode>(aLowerHemisphereMode)); },
			static_cast<int>(ESkyLightLowerHemisphereMode::Environment));

		groupBuilder.AddProperty<int>("Lower Hemisphere Mode", pLowerHemisphereHandle)
			.NameSlot().Label("Lower Hemisphere Mode")
			.ValueSlot().ComboBox().Options({ "Environment", "Solid Color" });

		auto pLowerHemisphereColorHandle = handleFactory.Make(&SLC::GetLowerHemisphereColor, &SLC::SetLowerHemisphereColor, Colors::Black);
		groupBuilder.AddProperty<Color>("Lower Hemisphere Color", pLowerHemisphereColorHandle)
			.NameSlot().Label("Lower Hemisphere Color")
			.ValueSlot().ColorPicker();
	}

	void SkyLightComponentDetailCustomization::SetupConnections() noexcept
	{
		m_OnSkyLightComponentPropertyChangedConnection = ScopedConnection(CoreObjectBroadcasters::OnEntityComponentPropertyChanged,
			[this](entity aEntity, TypeIndex aComponentType, MAYBE_UNUSED IComponent* aComponent, uint64 aProperty)
			{
				if (aComponentType != SkyLightComponent::StaticType())
					return;
				if (!IsEntityInspected(aEntity))
					return;
				if (aProperty == "m_PrimaryEnvironmentHandle"_h || aProperty == "m_BlendEnvironmentHandle"_h)
				{
					if (IDetailLayoutBuilder* pLayoutBuilder = GetDetailLayoutBuilder())
						pLayoutBuilder->ForceRefreshDetails();
				}
				else if (aProperty == "m_LightChannels"_h)
				{
					if (IDetailsView* pDetailsView = GetDetailsView())
						pDetailsView->RequestRefresh();
				}
			});
	}
}