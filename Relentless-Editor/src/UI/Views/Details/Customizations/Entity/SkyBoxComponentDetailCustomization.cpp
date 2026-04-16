#include "SkyBoxComponentDetailCustomization.h"

#include <Relentless.h>

#include "Property/EntityPropertyHandle.h"

#include "UI/Views/Details/IDetailsView.h"
#include "UI/Views/Details/LayoutBuilders/IDetailLayoutBuilder.h"
#include "UI/Views/Details/LayoutBuilders/IDetailCategoryBuilder.h"
#include "UI/Views/Details/Context/EntityDetailsContext.h"

namespace Relentless
{
	SkyBoxComponentDetailCustomization::~SkyBoxComponentDetailCustomization() noexcept
	{
		if (CoreObjectBroadcasters::OnComponentPropertyChanged.IsConnected(m_OnSkyBoxComponentPropertyChangedCallbackID))
			CoreObjectBroadcasters::OnComponentPropertyChanged.Detach(m_OnSkyBoxComponentPropertyChangedCallbackID);
	}

	void SkyBoxComponentDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& aDetailLayoutBuilder) noexcept
	{
		if (CoreObjectBroadcasters::OnComponentPropertyChanged.IsConnected(m_OnSkyBoxComponentPropertyChangedCallbackID))
			CoreObjectBroadcasters::OnComponentPropertyChanged.Detach(m_OnSkyBoxComponentPropertyChangedCallbackID);

		m_OnSkyBoxComponentPropertyChangedCallbackID = CoreObjectBroadcasters::OnComponentPropertyChanged.Connect([&aDetailLayoutBuilder]
		(MAYBE_UNUSED entity aEntity, TypeIndex aComponentType, MAYBE_UNUSED IComponent* aComponent, uint64 aProperty)
			{
				if (aComponentType != SkyBoxComponent::StaticType())
					return;

				if (aProperty == "m_PrimaryEnvironmentHandle"_h || aProperty == "m_BlendEnvironmentHandle"_h)
					aDetailLayoutBuilder.ForceRefreshDetails();
			});

		EntityDetailsContext& context = aDetailLayoutBuilder.GetDetailsView()->GetContext<EntityDetailsContext>();
		const bool multiSelection = context.Entities.size() > 1u;

		IDetailCategoryBuilder& categoryBuilder = aDetailLayoutBuilder.EditCategory(ICON_FA_EARTH_AMERICAS "  Sky Box");

		Ref<PropertyHandle<bool>> pIsActiveSkyBoxHandle = RLS_NEW PropertyHandle<bool>([&context]()
			{
				return context.Scene->GetActiveSkyBox() == context.Entities.front();
			},
			[&context](const bool& aIsChecked)
			{
				if (aIsChecked)
					context.Scene->SetActiveSkyBox(context.Entities.front());
				else
					context.Scene->RemoveActiveSkyBox();
			});

		categoryBuilder.AddProperty<bool>("Is Active", pIsActiveSkyBoxHandle)
			.NameSlot().Label("Is Active")
			.ValueSlot().CheckBox().Enabled(!multiSelection);

		SkyBoxComponent& skyBoxComponent = context.EntityManager->Get<SkyBoxComponent>(context.Entities.front());
		const AssetHandle& assetHandle = skyBoxComponent.GetPrimaryEnvironmentHandle();
		AssetData* pAssetData = ModuleManager::LoadModuleChecked<AssetRegistryModule>().FindAsset(assetHandle.Uuid);

		categoryBuilder.AddAssetProperty("Primary Environment", *pAssetData)
			.AcceptableAssetTypes({ Environment::StaticType() })
			.OnAssetsDropped([&context](Span<const AssetData> someAssetDatas)
				{
					const AssetHandle assetHandle = AssetManager::LoadAsset(someAssetDatas[0]);
					std::ranges::for_each(context.Entities, [&context, &assetHandle](entity aEntity) { context.EntityManager->Get<SkyBoxComponent>(aEntity).SetPrimaryEnvironment(assetHandle); });
				})
			.NameSlot().Label("Primary Environment")
			.ValueSlot().AssetThumbnail();

		Ref<EntityPropertyHandle<float, SkyBoxComponent>> pIntensityHandle = RLS_NEW EntityPropertyHandle<float, SkyBoxComponent>(
			*context.EntityManager,
			context.Entities,
			[](const SkyBoxComponent& aSBC) { return aSBC.GetIntensity(); },
			[](entity, SkyBoxComponent& aSBC, const float& aIntensity) { aSBC.SetIntensity(aIntensity); },
			1.0f
		);

		categoryBuilder.AddProperty<float>("Intensity", pIntensityHandle)
			.NameSlot().Label("Intensity")
			.ValueSlot().SpinBox().Range(0.0f, FLT_MAX).Delta(0.01f);

		Ref<EntityPropertyHandle<Color, SkyBoxComponent>> pColorHandle = RLS_NEW EntityPropertyHandle<Color, SkyBoxComponent>(
			*context.EntityManager,
			context.Entities,
			[](const SkyBoxComponent& aSBC) { return aSBC.GetTintColor(); },
			[](entity, SkyBoxComponent& aSBC, const Color& aColor) { aSBC.SetTintColor(aColor); },
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
					std::ranges::for_each(context.Entities, [&context, &assetHandle](entity aEntity) { context.EntityManager->Get<SkyBoxComponent>(aEntity).SetBlendEnvironment(assetHandle); });
				})
			.NameSlot().Label("Blend Environment")
			.ValueSlot().AssetThumbnail();

		Ref<EntityPropertyHandle<float, SkyBoxComponent>> pLODBiasHandle = RLS_NEW EntityPropertyHandle<float, SkyBoxComponent>(
			*context.EntityManager,
			context.Entities,
			[](const SkyBoxComponent& aSBC) { return aSBC.GetLODBias(); },
			[](entity, SkyBoxComponent& aSBC, const float& aLODBias) { aSBC.SetLODBias(aLODBias); },
			0.0f
		);

		Ref<EntityPropertyHandle<float, SkyBoxComponent>> pBlendFactorHandle = RLS_NEW EntityPropertyHandle<float, SkyBoxComponent>(
			*context.EntityManager,
			context.Entities,
			[](const SkyBoxComponent& aSBC) { return aSBC.GetBlendFactor(); },
			[](entity, SkyBoxComponent& aSBC, const float& aBlendFactor) { aSBC.SetBlendFactor(aBlendFactor); },
			0.0f
		);

		groupBuilder.AddProperty<float>("Blend Factor", pBlendFactorHandle)
			.NameSlot().Label("Blend Factor")
			.ValueSlot().Slider().Range(0.0f, 1.0f);

		groupBuilder.AddProperty<float>("LOD Bias", pLODBiasHandle)
			.NameSlot().Label("LOD Bias")
			.ValueSlot().SpinBox().Range(0.0f, FLT_MAX).Delta(0.01f);
	}

	bool SkyBoxComponentDetailCustomization::ShouldCustomize(IDetailLayoutBuilder& aDetailLayoutBuilder) const noexcept
	{
		const EntityDetailsContext& context = aDetailLayoutBuilder.GetDetailsView()->GetContext<EntityDetailsContext>();
		if (context.Entities.empty())
			return false;

		return std::ranges::all_of(context.Entities, [&context](entity aEntity) { return context.EntityManager->Has<SkyBoxComponent>(aEntity); });
	}
}