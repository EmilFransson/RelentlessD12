#include "ExponentialHeightFogComponentDetailCustomization.h"

#include "Core/Editor.h"

#include "Subsystem/EngineContentSubsystem.h"
#include "Subsystem/EntityComponentDefinitionRegistry.h"

#include "UI/Views/Details/DetailHelpers.h"

namespace Relentless
{ 
	void ExponentialHeightFogComponentDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& aDetailLayoutBuilder) noexcept
	{
		using EHFC = ExponentialHeightFogComponent;

		IDetailsView* pDetailsView = aDetailLayoutBuilder.GetDetailsView();
		EntityDetailsContext& context = pDetailsView->GetContext<EntityDetailsContext>();
		DetailHelpers::EntityHandleFactory<EHFC> handleFactory{ .Entities = context.Entities, .EntityManager = *context.EntityManager };

		EntityComponentDefinitionRegistry* pEntityComponentDefinitionRegistry = Editor::Get()->GetSubsystem<EntityComponentDefinitionRegistry>();
		Ref<IEntityComponentDefinition> pEntityComponentDefinition = pEntityComponentDefinitionRegistry->GetDefinition<EHFC>();
		IDetailCategoryBuilder& categoryBuilder = aDetailLayoutBuilder.EditCategory(std::format("{}  {}", pEntityComponentDefinition->GetIcon(), pEntityComponentDefinition->GetDisplayName()).c_str());

		//Layer 0:
		{
			auto pDensityHandle = handleFactory.MakeCustom(
				[](const EHFC& aComponent) { return aComponent.GetFogLayer(0u).Density; },
				[](entity, EHFC& aComponent, const float& aDensity) { aComponent.SetLayerDensity(0u, aDensity); },
				EHFC::DEFAULT_DENSITY);
			categoryBuilder.AddProperty<float>("Fog Density", pDensityHandle)
				.NameSlot().Label("Fog Density")
				.ValueSlot().Slider().Range(0.0f, 0.05f);

			auto pHeightFalloffHandle = handleFactory.MakeCustom(
				[](const EHFC& aComponent) { return aComponent.GetFogLayer(0u).HeightFalloff; },
				[](entity, EHFC& aComponent, const float& aHeightFallOff) { aComponent.SetLayerHeightFalloff(0u, aHeightFallOff); },
				EHFC::DEFAULT_HEIGHT_FALLOF);
			categoryBuilder.AddProperty<float>("Fog Height Falloff", pHeightFalloffHandle)
				.NameSlot().Label("Fog Height Falloff")
				.ValueSlot().Slider().Range(0.001f, 2.0f).Logarithmic(true);

			auto pStartDistanceHandle = handleFactory.MakeCustom(
				[](const EHFC& aComponent) { return aComponent.GetFogLayer(0u).StartDistance; },
				[](entity, EHFC& aComponent, const float& aStartDistance) { aComponent.SetLayerStartDistance(0u, aStartDistance); },
				EHFC::DEFAULT_START_DISTANCE);
			categoryBuilder.AddProperty<float>("Fog Start Distance", pStartDistanceHandle)
				.NameSlot().Label("Fog Start Distance")
				.ValueSlot().SpinBox().Range(0.0f, FLT_MAX);

			auto pEndDistanceHandle = handleFactory.MakeCustom(
				[](const EHFC& aComponent) { return aComponent.GetFogLayer(0u).EndDistance; },
				[](entity, EHFC& aComponent, const float& aEndDistance) { aComponent.SetLayerEndDistance(0u, aEndDistance); },
				EHFC::DEFAULT_END_DISTANCE);
			categoryBuilder.AddProperty<float>("Fog End Distance", pEndDistanceHandle)
				.NameSlot().Label("Fog End Distance")
				.ValueSlot().SpinBox().Range(0.0f, FLT_MAX);
		}
		
		//Layer 1:
		{
			IDetailGroupBuilder secondFogDataGroup = categoryBuilder.EditGroup("Second Fog Data");
		
			auto pDensityHandle = handleFactory.MakeCustom(
				[](const EHFC& aComponent) { return aComponent.GetFogLayer(1u).Density; },
				[](entity, EHFC& aComponent, const float& aDensity) { aComponent.SetLayerDensity(1u, aDensity); },
				0.0f);
			secondFogDataGroup.AddProperty<float>("Fog Density", pDensityHandle)
				.NameSlot().Label("Fog Density")
				.ValueSlot().Slider().Range(0.0f, 0.05f);

			auto pHeightFalloffHandle = handleFactory.MakeCustom(
				[](const EHFC& aComponent) { return aComponent.GetFogLayer(1u).HeightFalloff; },
				[](entity, EHFC& aComponent, const float& aHeightFallOff) { aComponent.SetLayerHeightFalloff(1u, aHeightFallOff); },
				EHFC::DEFAULT_HEIGHT_FALLOF);
			secondFogDataGroup.AddProperty<float>("Fog Height Falloff", pHeightFalloffHandle)
				.NameSlot().Label("Fog Height Falloff")
				.ValueSlot().Slider().Range(0.001f, 2.0f).Logarithmic(true);

			auto pStartDistanceHandle = handleFactory.MakeCustom(
				[](const EHFC& aComponent) { return aComponent.GetFogLayer(1u).StartDistance; },
				[](entity, EHFC& aComponent, const float& aStartDistance) { aComponent.SetLayerStartDistance(1u, aStartDistance); },
				EHFC::DEFAULT_START_DISTANCE);
			secondFogDataGroup.AddProperty<float>("Fog Start Distance", pStartDistanceHandle)
				.NameSlot().Label("Fog Start Distance")
				.ValueSlot().SpinBox().Range(0.0f, FLT_MAX);

			auto pEndDistanceHandle = handleFactory.MakeCustom(
				[](const EHFC& aComponent) { return aComponent.GetFogLayer(1u).EndDistance; },
				[](entity, EHFC& aComponent, const float& aEndDistance) { aComponent.SetLayerEndDistance(1u, aEndDistance); },
				EHFC::DEFAULT_END_DISTANCE);
			secondFogDataGroup.AddProperty<float>("Fog End Distance", pEndDistanceHandle)
				.NameSlot().Label("Fog End Distance")
				.ValueSlot().SpinBox().Range(0.0f, FLT_MAX);
		}

		auto pInScatterColorHandle = handleFactory.Make(&EHFC::GetInscatteringColor, &EHFC::SetInscatteringColor, EHFC::DEFAULT_INSCATTERING_COLOR);
		categoryBuilder.AddProperty<Color>("Fog Inscatter Color", pInScatterColorHandle)
			.NameSlot().Label("Fog Inscatter Color")
			.ValueSlot().ColorPicker();

		auto pMaxOpacityHandle = handleFactory.Make(&EHFC::GetMaxOpacity, &EHFC::SetMaxOpacity, EHFC::DEFAULT_MAX_OPACITY);
		categoryBuilder.AddProperty<float>("Fog Max Opacity", pMaxOpacityHandle)
			.NameSlot().Label("Fog Max Opacity")
			.ValueSlot().Slider().Range(0.0f, 1.0f);

		auto pInscatterFogModeHandle = handleFactory.MakeCustom(
			[](const EHFC& aComponent) { return static_cast<int>(aComponent.GetInscatterMode()); },
			[](entity, EHFC& aComponent, const int& aInScatterMode) { aComponent.SetInscatterMode(static_cast<EFogInscatterMode>(aInScatterMode)); },
			static_cast<int>(EFogInscatterMode::Uniform));
		categoryBuilder.AddProperty<int>("Fog Inscatter Mode", pInscatterFogModeHandle)
			.NameSlot().Label("Fog Inscatter Mode")
			.ValueSlot().ComboBox().Options({ "Uniform", "Cubemap" });

		IDetailGroupBuilder inscatteringTextureGroup = categoryBuilder.EditGroup("Inscattering Texture");

		AssetRegistryModule& assetRegistry = ModuleManager::LoadModuleChecked<AssetRegistryModule>();
		EngineContentSubsystem* pEngineContentSubsystem = Editor::Get()->GetSubsystem<EngineContentSubsystem>();
		const AssetHandle heuristicHandle = context.EntityManager->Get<EHFC>(context.Entities.front()).GetInscatterTextureHandle();

		const bool allSameAndValid = heuristicHandle.IsValid() && std::ranges::all_of(context.Entities, [&context, &heuristicHandle](const entity aEntity)
			{
				return context.EntityManager->Get<EHFC>(aEntity).GetInscatterTextureHandle() == heuristicHandle;
			});

		AssetData* pAssetData = nullptr;

		if (allSameAndValid)
			pAssetData = assetRegistry.FindAsset(heuristicHandle.Uuid);
		if (!pAssetData)
			pAssetData = assetRegistry.FindAsset(pEngineContentSubsystem->GetNoneTexture2DHandle().Uuid);

		RLS_ASSERT(pAssetData, "[ExponentialHeightFogComponentDetailCustomization::CustomizeDetails]: Asset data is invalid.");

		inscatteringTextureGroup.AddAssetProperty("Inscattering Color Cubemap", *pAssetData)
			.AcceptableAssetTypes({ TextureCube::StaticType() })
			.OnAssetsDropped([&context](Span<const AssetData> someAssetDatas)
				{
					const AssetHandle assetHandle = AssetManager::LoadAsset(someAssetDatas[0]);
					std::ranges::for_each(context.Entities, [&context, &assetHandle](entity aEntity) { context.EntityManager->Get<EHFC>(aEntity).SetInscatterTexture(assetHandle); });
				})
			.NameSlot().Label("Inscattering Color Cubemap")
			.ValueSlot().AssetThumbnail();

		auto pInScatterTextureTintColorHandle = handleFactory.Make(&EHFC::GetInscatterTextureTintColor, &EHFC::SetInscatterTextureTintColor, EHFC::DEFAULT_INSCATTERING_TEXTURE_TINT);
		inscatteringTextureGroup.AddProperty<Color>("Inscattering Texture Tint", pInScatterTextureTintColorHandle)
			.NameSlot().Label("Inscattering Texture Tint")
			.ValueSlot().ColorPicker();
	}
}