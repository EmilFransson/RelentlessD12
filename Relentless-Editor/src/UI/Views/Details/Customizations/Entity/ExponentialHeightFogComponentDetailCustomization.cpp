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
		categoryBuilder.AddHeaderAction("Remove", [this]() { RemoveFromInspected(); });
		
		categoryBuilder.AddProperty<bool>(
			"Is Active",
			[&context](){ return context.Scene->GetActiveExponentialHeightFog() == context.Entities.front(); },
			[&context](const bool& aIsChecked)
			{  
				if (aIsChecked)
					context.Scene->SetActiveExponentialHeightFog(context.Entities.front());
				else
					context.Scene->RemoveActiveExponentialHeightFog();
			})
			.NameSlot().Label("Is Active")
			.ValueSlot().CheckBox().Enabled(context.Entities.size() == 1u);

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

			auto pHeightOffsetHandle = handleFactory.MakeCustom(
				[](const EHFC& aComponent) { return aComponent.GetFogLayer(0u).HeightOffset; },
				[](entity, EHFC& aComponent, const float& aHeightOffset) { aComponent.SetLayerHeightOffset(0u, aHeightOffset); },
				EHFC::DEFAULT_HEIGHT_OFFSET);
			categoryBuilder.AddProperty<float>("Fog Height Offset", pHeightOffsetHandle)
				.NameSlot().Label("Fog Height Offset")
				.ValueSlot().SpinBox().Range(-FLT_MAX, FLT_MAX);

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

			auto pHeightOffsetHandle = handleFactory.MakeCustom(
				[](const EHFC& aComponent) { return aComponent.GetFogLayer(1u).HeightOffset; },
				[](entity, EHFC& aComponent, const float& aHeightOffset) { aComponent.SetLayerHeightOffset(1u, aHeightOffset); },
				EHFC::DEFAULT_HEIGHT_OFFSET);
			secondFogDataGroup.AddProperty<float>("Fog Height Offset", pHeightOffsetHandle)
				.NameSlot().Label("Fog Height Offset")
				.ValueSlot().SpinBox().Range(-FLT_MAX, FLT_MAX);

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

		auto pInscatteringColorIntensityHandle = handleFactory.Make(&EHFC::GetInscatteringColorIntensity, &EHFC::SetInscatteringColorIntensity, EHFC::DEFAULT_INSCATTERING_COLOR_INTENSITY);
		categoryBuilder.AddProperty<float>("Fog Inscattering Color Intensity", pInscatteringColorIntensityHandle)
			.NameSlot().Label("Fog Inscattering Color Intensity")
			.ValueSlot().SpinBox().Range(0.0f, FLT_MAX);

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

		handleFactory.MakeAssetTarget(categoryBuilder, "Inscattering Color Cubemap", { TextureCube::StaticType() }, &EHFC::GetInscatterTextureHandle, &EHFC::SetInscatterTexture, &EHFC::RemoveInscatterTexture);

		auto pInScatterTextureTintColorHandle = handleFactory.Make(&EHFC::GetInscatterTextureTintColor, &EHFC::SetInscatterTextureTintColor, EHFC::DEFAULT_INSCATTERING_TEXTURE_TINT);
		inscatteringTextureGroup.AddProperty<Color>("Inscattering Texture Tint", pInScatterTextureTintColorHandle)
			.NameSlot().Label("Inscattering Texture Tint")
			.ValueSlot().ColorPicker();

		auto pFullyDirectionalInScatteringColorDistanceHandle = handleFactory.Make(&EHFC::GetFullyDirectionalInScatteringColorDistance, &EHFC::SetFullyDirectionalInScatteringColorDistance, EHFC::DEFAULT_FULLY_DIRECTIONAL_INSCATTERING_COLOR_DISTANCE);
		categoryBuilder.AddProperty<float>("Fully Directional Inscattering Color Distance", pFullyDirectionalInScatteringColorDistanceHandle)
			.NameSlot().Label("Fully Directional Inscattering Color Distance")
			.ValueSlot().SpinBox().Range(EHFC::DEFAULT_NON_DIRECTIONAL_INSCATTERING_COLOR_DISTANCE, 10'000.0f);

		auto pNonDirectionalInScatteringColorDistanceHandle = handleFactory.Make(&EHFC::GetNonDirectionalInScatteringColorDistance, &EHFC::SetNonDirectionalInScatteringColorDistance, EHFC::DEFAULT_NON_DIRECTIONAL_INSCATTERING_COLOR_DISTANCE);
		categoryBuilder.AddProperty<float>("Non-Directional Inscattering Color Distance", pNonDirectionalInScatteringColorDistanceHandle)
			.NameSlot().Label("Non-Directional Inscattering Color Distance")
			.ValueSlot().SpinBox().Range(EHFC::DEFAULT_NON_DIRECTIONAL_INSCATTERING_COLOR_DISTANCE, 10'000.0f);
	}
}