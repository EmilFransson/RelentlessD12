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
	SkyLightComponentDetailCustomization::~SkyLightComponentDetailCustomization() noexcept
	{
		if (CoreObjectBroadcasters::OnEntityComponentPropertyChanged.IsConnected(m_OnSkyLightComponentPropertyChangedCallbackID))
			CoreObjectBroadcasters::OnEntityComponentPropertyChanged.Detach(m_OnSkyLightComponentPropertyChangedCallbackID);
	}

	void SkyLightComponentDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& aDetailLayoutBuilder) noexcept
	{
		using SLC = SkyLightComponent;
		EntityDetailsContext& context = aDetailLayoutBuilder.GetDetailsView()->GetContext<EntityDetailsContext>();
		DetailHelpers::EntityHandleFactory<SLC> handleFactory{ .Entities = context.Entities, .EntityManager = *context.EntityManager };
		const bool multiSelection = context.Entities.size() > 1u;

		if (CoreObjectBroadcasters::OnEntityComponentPropertyChanged.IsConnected(m_OnSkyLightComponentPropertyChangedCallbackID))
			CoreObjectBroadcasters::OnEntityComponentPropertyChanged.Detach(m_OnSkyLightComponentPropertyChangedCallbackID);

		m_OnSkyLightComponentPropertyChangedCallbackID = CoreObjectBroadcasters::OnEntityComponentPropertyChanged.Connect([&aDetailLayoutBuilder]
		(MAYBE_UNUSED entity aEntity, TypeIndex aComponentType, MAYBE_UNUSED IComponent* aComponent, uint64 aProperty)
			{
				if (aComponentType != SkyLightComponent::StaticType())
					return;

				if (aProperty == "m_PrimaryEnvironmentHandle"_h || aProperty == "m_BlendEnvironmentHandle"_h)
					aDetailLayoutBuilder.ForceRefreshDetails();
			});


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

		Ref<EntityPropertyHandle<int, SkyLightComponent>> pCaptureModeHandle = RLS_NEW EntityPropertyHandle<int, SkyLightComponent>(
			*context.EntityManager,
			context.Entities,
			[](const SkyLightComponent& aSLC) { return static_cast<int>(aSLC.GetCaptureMode());},
			[](entity, SkyLightComponent& aSLC, const int& aCaptureMode) { aSLC.SetCaptureMode(static_cast<ESkyLightCaptureMode>(aCaptureMode)); },
			0
		);

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

		Ref<EntityPropertyHandle<uint32, SkyLightComponent>> pRadianceSizeHandle = RLS_NEW EntityPropertyHandle<uint32, SkyLightComponent>(
			*context.EntityManager,
			context.Entities,
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

				aSLC.SetRadianceMapSize(newSize);
			},
			256u
		);

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
			.ValueSlot().Widget([&context]()
				{
					auto&& AllHaveChannelsSet = [&context](ELightChannel aLightChannel) -> bool
						{
							return std::ranges::all_of(context.Entities, [&context, aLightChannel](entity aEntity)
								{
									const SkyLightComponent& skyLightComponent = context.EntityManager->Get<SkyLightComponent>(aEntity);
									return skyLightComponent.HasLightChannelsEnabled(aLightChannel);
								});
						};

					Ref<HorizontalBox> pBox = RLS_NEW HorizontalBox();
					pBox->SetSpacing(5.0f);

					for (uint32 i = 0; i < 4; ++i)
					{
						const ELightChannel lightChannel = static_cast<ELightChannel>(1u << i);

						Button* pButton = pBox->AddWidget(RLS_NEW Button(std::format("{}", i)));
						pButton->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);
						pButton->OnClicked([&context, lightChannel, pButton, AllHaveChannelsSet]()
							{
								const bool setChannel = !AllHaveChannelsSet(lightChannel);

								std::ranges::for_each(context.Entities, [&context, lightChannel, setChannel](entity aEntity)
									{
										SkyLightComponent& skyLightComponent = context.EntityManager->Get<SkyLightComponent>(aEntity);
										skyLightComponent.SetLightChannelEnabled(lightChannel, setChannel);
									});

								pButton->SetBackgroundColor(setChannel ? Colors::Blue : Colors::Black);
							});

						pButton->SetBackgroundColor(AllHaveChannelsSet(lightChannel) ? Colors::Blue : Colors::Black);
					}

					return pBox;
				});

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

		Ref<EntityPropertyHandle<int, SkyLightComponent>> pLowerHemisphereHandle = RLS_NEW EntityPropertyHandle<int, SkyLightComponent>(
			*context.EntityManager,
			context.Entities,
			[](const SkyLightComponent& aSLC) { return static_cast<int>(aSLC.GetLowerHemisphereMode()); },
			[](entity, SkyLightComponent& aSLC, const int& aLowerHemisphereMode) { aSLC.SetLowerHemisphereMode(static_cast<ESkyLightLowerHemisphereMode>(aLowerHemisphereMode)); },
			0
		);

		groupBuilder.AddProperty<int>("Lower Hemisphere Mode", pLowerHemisphereHandle)
			.NameSlot().Label("Lower Hemisphere Mode")
			.ValueSlot().ComboBox().Options({ "Environment", "Solid Color" });

		auto pLowerHemisphereColorHandle = handleFactory.Make(&SLC::GetLowerHemisphereColor, &SLC::SetLowerHemisphereColor, Colors::Black);
		groupBuilder.AddProperty<Color>("Lower Hemisphere Color", pLowerHemisphereColorHandle)
			.NameSlot().Label("Lower Hemisphere Color")
			.ValueSlot().ColorPicker();
	}

	bool SkyLightComponentDetailCustomization::ShouldCustomize(IDetailLayoutBuilder& aDetailLayoutBuilder) const noexcept
	{
		const EntityDetailsContext& context = aDetailLayoutBuilder.GetDetailsView()->GetContext<EntityDetailsContext>();
		if (context.Entities.empty())
			return false;

		return std::ranges::all_of(context.Entities, [&context](entity aEntity) { return context.EntityManager->Has<SkyLightComponent>(aEntity); });
	}
}