#include "SkyLightComponentDetailCustomization.h"

#include <Relentless.h>

#include "UI/Views/Details/IDetailsView.h"
#include "UI/Views/Details/LayoutBuilders/IDetailLayoutBuilder.h"
#include "UI/Views/Details/LayoutBuilders/IDetailCategoryBuilder.h"
#include "UI/Views/Details/Context/EntityDetailsContext.h"

#include "Property/EntityPropertyHandle.h"

namespace Relentless
{
	void SkyLightComponentDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& aDetailLayoutBuilder) noexcept
	{
		EntityDetailsContext& context = aDetailLayoutBuilder.GetDetailsView()->GetContext<EntityDetailsContext>();
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

		Ref<EntityPropertyHandle<uint32, SkyLightComponent>> pRealtimeMipsPerFrameHandle = RLS_NEW EntityPropertyHandle<uint32, SkyLightComponent>(
			*context.EntityManager,
			context.Entities,
			[](const SkyLightComponent& aSLC) { return aSLC.GetRealtimeMipsPerFrame(); },
			[](entity, SkyLightComponent& aSLC, const uint32& aNumMips) { aSLC.SetRealtimeMipsPerFrame(aNumMips); },
			1u
		);

		categoryBuilder.AddProperty<uint32>("Realtime Mips Per Frame", pRealtimeMipsPerFrameHandle)
			.NameSlot().Label("Realtime Mips Per Frame")
			.ValueSlot().SpinBox().Delta(1u).Range(0u, 1'000'000u);

		Ref<EntityPropertyHandle<float, SkyLightComponent>> pIntensityHandle = RLS_NEW EntityPropertyHandle<float, SkyLightComponent>(
			*context.EntityManager,
			context.Entities,
			[](const SkyLightComponent& aSLC) { return aSLC.GetIntensity(); },
			[](entity, SkyLightComponent& aSLC, const float& aIntensity) { aSLC.SetIntensity(aIntensity); },
			1.0f
		);
		
		categoryBuilder.AddProperty<float>("Intensity", pIntensityHandle)
			.NameSlot().Label("Intensity")
			.ValueSlot().SpinBox().Range(0.0f, FLT_MAX).Delta(0.01f);

		Ref<EntityPropertyHandle<Color, SkyLightComponent>> pColorHandle = RLS_NEW EntityPropertyHandle<Color, SkyLightComponent>(
			*context.EntityManager,
			context.Entities,
			[](const SkyLightComponent& aSLC) { return aSLC.GetTintColor(); },
			[](entity, SkyLightComponent& aSLC, const Color& aColor){ aSLC.SetTintColor(aColor); },
			Colors::White
		);

		categoryBuilder.AddProperty<Color>("Color", pColorHandle)
			.NameSlot().Label("Color")
			.ValueSlot().ColorPicker();

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

		Ref<EntityPropertyHandle<float, SkyLightComponent>> pBlendHandle = RLS_NEW EntityPropertyHandle<float, SkyLightComponent>(
			*context.EntityManager,
			context.Entities,
			[](const SkyLightComponent& aSLC) { return aSLC.GetBlendFactor(); },
			[](entity, SkyLightComponent& aSLC, const float& aBlendFactor) { aSLC.SetBlendFactor(aBlendFactor); },
			0.0f
		);

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

		Ref<EntityPropertyHandle<Color, SkyLightComponent>> pLowerHemisphereColorHandle = RLS_NEW EntityPropertyHandle<Color, SkyLightComponent>(
			*context.EntityManager,
			context.Entities,
			[](const SkyLightComponent& aSLC) { return aSLC.GetLowerHemisphereColor(); },
			[](entity, SkyLightComponent& aSLC, const Color& aLowerHemisphereColor) { aSLC.SetLowerHemisphereColor(aLowerHemisphereColor); },
			Colors::Black
		);

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