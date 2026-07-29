#include "SkyBoxComponentDetailCustomization.h"

#include <Relentless.h>

#include "Property/EntityPropertyHandle.h"

#include "UI/Views/Details/DetailHelpers.h"
#include "UI/Views/Details/IDetailsView.h"
#include "UI/Views/Details/LayoutBuilders/IDetailLayoutBuilder.h"
#include "UI/Views/Details/LayoutBuilders/IDetailCategoryBuilder.h"
#include "UI/Views/Details/Context/EntityDetailsContext.h"

namespace Relentless
{
	void SkyBoxComponentDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& aDetailLayoutBuilder) noexcept
	{
		SetupConnections();

		using SBC = SkyBoxComponent;
		
		EntityDetailsContext& context = aDetailLayoutBuilder.GetDetailsView()->GetContext<EntityDetailsContext>();
		DetailHelpers::EntityHandleFactory<SBC> handleFactory{ .Entities = context.Entities, .EntityManager = *context.EntityManager };
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

		auto pIntensityHandle = handleFactory.Make(&SBC::GetIntensity, &SBC::SetIntensity, 1.0f);
		categoryBuilder.AddProperty<float>("Intensity", pIntensityHandle)
			.NameSlot().Label("Intensity")
			.ValueSlot().SpinBox().Range(0.0f, FLT_MAX).Delta(0.01f);

		auto pColorHandle = handleFactory.Make(&SBC::GetTintColor, &SBC::SetTintColor, Colors::White);
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

		auto pBlendFactorHandle = handleFactory.Make(&SBC::GetBlendFactor, &SBC::SetBlendFactor, 0.0f);
		groupBuilder.AddProperty<float>("Blend Factor", pBlendFactorHandle)
			.NameSlot().Label("Blend Factor")
			.ValueSlot().Slider().Range(0.0f, 1.0f);

		auto pLODBiasHandle = handleFactory.Make(&SBC::GetLODBias, &SBC::SetLODBias, 0.0f);
		groupBuilder.AddProperty<float>("LOD Bias", pLODBiasHandle)
			.NameSlot().Label("LOD Bias")
			.ValueSlot().SpinBox().Range(0.0f, FLT_MAX).Delta(0.01f);
	}

	void SkyBoxComponentDetailCustomization::SetupConnections() noexcept
	{
		m_OnSkyBoxComponentPropertyChangedConnection = ScopedConnection(CoreObjectBroadcasters::OnEntityComponentPropertyChanged,
			[this](entity aEntity, TypeIndex aComponentType, MAYBE_UNUSED IComponent* aComponent, uint64 aProperty)
			{
				if (aComponentType != SkyBoxComponent::StaticType())
					return;
				if (!IsEntityInspected(aEntity))
					return;
				if (aProperty == "m_PrimaryEnvironmentHandle"_h || aProperty == "m_BlendEnvironmentHandle"_h)
				{
					if (IDetailLayoutBuilder* pLayoutBuilder = GetDetailLayoutBuilder())
						pLayoutBuilder->ForceRefreshDetails();
				}
			});
	}
}