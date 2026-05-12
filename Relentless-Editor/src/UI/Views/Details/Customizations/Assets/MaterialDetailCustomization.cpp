#include "MaterialDetailCustomization.h"

#include "Core/Editor.h"

#include "Subsystem/EngineContentSubsystem.h"

#include "UI/Views/Details/Context/MaterialDetailsContext.h"
#include "UI/Views/Details/IDetailsView.h"
#include "UI/Views/Details/LayoutBuilders/DetailPropertyRowBuilder.h"
#include "UI/Views/Details/LayoutBuilders/IDetailLayoutBuilder.h"

namespace Relentless
{
	void MaterialDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& aDetailLayoutBuilder) noexcept
	{
		MaterialDetailsContext& detailsContext = aDetailLayoutBuilder.GetDetailsView()->GetContext<MaterialDetailsContext>();
		AssetRegistryModule& assetRegistry = ModuleManager::LoadModuleChecked<AssetRegistryModule>();
		EngineContentSubsystem* pEngineContentSubsystem = Editor::Get()->GetSubsystem<EngineContentSubsystem>();

		RLS_ASSERT(detailsContext.Material, "[MaterialDetailCustomization::CustomizeDetails]: Material is invalid.");
		RLS_ASSERT(pEngineContentSubsystem, "[MaterialDetailCustomization::CustomizeDetails]: Engine content subsystem is invalid.");

		if (detailsContext.Material->OnPropertyChanged.IsConnected(m_OnAssetPropertyChangedCallbackID))
		{
			detailsContext.Material->OnPropertyChanged.Detach(m_OnAssetPropertyChangedCallbackID);
			m_OnAssetPropertyChangedCallbackID = INVALID_CALLBACK_ID;
		}

		m_OnAssetPropertyChangedCallbackID = detailsContext.Material->OnPropertyChanged.Connect([this, &aDetailLayoutBuilder, &detailsContext]
		(MAYBE_UNUSED IAsset* aAsset, MAYBE_UNUSED uint64 aProperty)
			{
				if (m_SuspendRefresh)
					return;

				aDetailLayoutBuilder.ForceRefreshDetails();
			});

		IDetailCategoryBuilder& categoryBuilder = aDetailLayoutBuilder.EditCategory(ICON_FA_PALETTE "  Material");

		//Blend Mode:
		{
			categoryBuilder.AddProperty<int>("Blend Mode", 
				[&detailsContext]() { return static_cast<int>(detailsContext.Material->GetBlendMode()); },
				[this, &detailsContext](const int& aValue)
				{
					ScopedSuspend scopedSuspend(m_SuspendRefresh);
					detailsContext.Material->SetBlendMode(static_cast<EBlendMode>(aValue));
				},
				0
				)
				.NameSlot().Label("Blend Mode")
				.ValueSlot().ComboBox().Options({ "Opaque", "Alpha Mask", "Alpha Blend" });
		}

		//Two Sided:
		{
			categoryBuilder.AddProperty<bool>("Two Sided", 
				[&detailsContext]() { return detailsContext.Material->IsTwoSided(); },
				[this, &detailsContext](const bool& aValue)
				{
					ScopedSuspend scopedSuspend(m_SuspendRefresh);
					detailsContext.Material->SetIsTwoSided(aValue);
				},
				false)
				.NameSlot().Label("Two Sided")
				.ValueSlot().CheckBox();
		}
		
		auto AddTextureMapProperty = [&assetRegistry, &detailsContext, pEngineContentSubsystem, &aDetailLayoutBuilder](ETextureType aTextureType, const char* aPropertyName, IDetailGroupBuilder& aGroupBuilder)
			{
				AssetData* pAssetData = assetRegistry.FindAsset(detailsContext.Material->GetTextureHandle(aTextureType).Uuid);
				bool isNone = false;
				if (!pAssetData)
				{
					isNone = true;
					pAssetData = assetRegistry.FindAsset(pEngineContentSubsystem->GetNoneTexture2DHandle().Uuid);
				}
				aGroupBuilder.AddAssetProperty(aPropertyName, *pAssetData)
					.AcceptableAssetTypes({ Texture2D::StaticType() })
					.OnAssetsDropped([&detailsContext, aTextureType, &aDetailLayoutBuilder](Span<const AssetData> someAssetDatas)
						{
							const AssetHandle assetHandle = AssetManager::LoadAsset(someAssetDatas[0]);
							detailsContext.Material->SetTexture(aTextureType, assetHandle);

							Application::Get().SubmitToMainThread([&aDetailLayoutBuilder]()
								{
									aDetailLayoutBuilder.GetDetailsView()->Rebuild<MaterialDetailsContext>();
								});
						})
					.NameSlot().Label("Map")
					.ValueSlot().AssetThumbnail().Row()
					.RevertSlot().Widget([&detailsContext, aTextureType, &aDetailLayoutBuilder, isNone]()
						{
							Ref<HorizontalBox> pRevertBox = RLS_NEW HorizontalBox();
							pRevertBox->SetPadding({ 0.0f, 2.0f, 0.0f, 2.0f });

							if (!isNone)
							{
								Button* pButton = pRevertBox->AddWidget(RLS_NEW Button(ICON_FA_ARROW_ROTATE_LEFT));
								pButton->SetBackgroundColor(Colors::Transparent);
								pButton->SetBorderColor(Colors::Transparent);
								pButton->SetHoverColor(Colors::Transparent);
								pButton->SetActiveColor(Colors::Transparent);
								pButton->SetTextColor(Color(1.0f, 1.0f, 1.0f, 0.5f));
								pButton->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);

								pButton->OnMouseEnter([](Button* aButton) { aButton->SetTextColor(Color(1.0f, 1.0f, 1.0f, 1.0f)); });
								pButton->OnMouseEnter([](Button* aButton) { aButton->SetTextColor(Color(1.0f, 1.0f, 1.0f, 1.0f)); });
								pButton->OnMouseExit([](Button* aButton) { aButton->SetTextColor(Color(1.0f, 1.0f, 1.0f, 0.5f)); });
								pButton->OnClicked([&detailsContext, aTextureType, &aDetailLayoutBuilder]()
									{
										detailsContext.Material->RemoveTexture(aTextureType);

										Application::Get().SubmitToMainThread([&aDetailLayoutBuilder]()
											{
												aDetailLayoutBuilder.GetDetailsView()->Rebuild<MaterialDetailsContext>();
											});
									});
							}

							return pRevertBox;
						});
			};

		//Albedo
		{
			auto albedoGroupBuilder = categoryBuilder.EditGroup("Albedo");
			
			//Map:
			AddTextureMapProperty(ETextureType::Albedo, "Albedo Map", albedoGroupBuilder);

			//Color:
			albedoGroupBuilder.AddProperty<Color>("Color",
				[&detailsContext](){ return detailsContext.Material->GetAlbedoColor(); },
				[this, &detailsContext](const Color& aColor) { ScopedSuspend scopedSuspend(m_SuspendRefresh); detailsContext.Material->SetAlbedoColor(aColor); },
				Colors::White)
				.NameSlot().Label("Color")
				.ValueSlot().ColorPicker();
		}

		//Metallic:
		{
			auto metallicGroupBuilder = categoryBuilder.EditGroup("Metallic");
			
			//Map:
			AddTextureMapProperty(ETextureType::Metallic, "Metallic Map", metallicGroupBuilder);

			//Factor:
			metallicGroupBuilder.AddProperty<float>("Metallic",
				[&detailsContext]() { return detailsContext.Material->GetMetalness(); },
				[this, &detailsContext](const float& aValue) { ScopedSuspend scopedSuspend(m_SuspendRefresh); detailsContext.Material->SetMetalness(aValue); },
				0.0f)
				.NameSlot().Label("Metallic")
				.ValueSlot().Slider().Range(0.0f, 1.0f);
		}

		//Roughness:
		{
			auto roughnessGroupBuilder = categoryBuilder.EditGroup("Roughness");
			
			//Map:
			AddTextureMapProperty(ETextureType::Roughness, "Roughness Map", roughnessGroupBuilder);

			//Factor:
			roughnessGroupBuilder.AddProperty<float>("Roughness",
				[&detailsContext]() { return detailsContext.Material->GetRoughness(); },
				[this, &detailsContext](const float& aValue) { ScopedSuspend scopedSuspend(m_SuspendRefresh); detailsContext.Material->SetRoughness(aValue); },
				0.5f)
				.NameSlot().Label("Roughness")
				.ValueSlot().Slider().Range(0.0f, 1.0f);
		}

		//Normal:
		{
			auto normalMapGroupBuilder = categoryBuilder.EditGroup("Normal Map");

			//Map:
			AddTextureMapProperty(ETextureType::NormalMap, "Normal Map", normalMapGroupBuilder);
		}

		//Displacement:
		{
			auto displacementGroupBuilder = categoryBuilder.EditGroup("Displacement");

			//Map:
			AddTextureMapProperty(ETextureType::DisplacementMap, "Displacement Map", displacementGroupBuilder);

			//Strength:
			displacementGroupBuilder.AddProperty<float>("Displacement Strength",
				[&detailsContext]() { return detailsContext.Material->GetDisplacementIntensity(); },
				[this, &detailsContext](const float& aValue) { ScopedSuspend scopedSuspend(m_SuspendRefresh); detailsContext.Material->SetDisplacementIntensity(aValue); },
				0.0f)
				.NameSlot().Label("Strength")
				.ValueSlot().Slider().Range(0.0f, 1.0f);
		}

		//Ambient Occlusion:
		{
			auto ambientOcclusionGroupBuilder = categoryBuilder.EditGroup("Ambient Occlusion");

			//Map:
			AddTextureMapProperty(ETextureType::AmbientOcclusion, "AO Map", ambientOcclusionGroupBuilder);

			//Strength:
			ambientOcclusionGroupBuilder.AddProperty<float>("Ambient Occlusion Strength",
				[&detailsContext]() { return detailsContext.Material->GetAmbientOcclusionIntensity(); },
				[this, &detailsContext](const float& aValue) { ScopedSuspend scopedSuspend(m_SuspendRefresh); detailsContext.Material->SetAmbientOcclusionIntensity(aValue); },
				1.0f)
				.NameSlot().Label("Strength")
				.ValueSlot().Slider().Range(0.0f, 1.0f);
		}

		//Emission:
		{
			auto emissionGroupBuilder = categoryBuilder.EditGroup("Emission");

			//Map:
			AddTextureMapProperty(ETextureType::Emission, "Emission Map", emissionGroupBuilder);

			//Color:
			emissionGroupBuilder.AddProperty<Color>("Emission Color",
				[&detailsContext]() { return detailsContext.Material->GetEmissiveColor(); },
				[this, &detailsContext](const Color& aColor) { ScopedSuspend scopedSuspend(m_SuspendRefresh); detailsContext.Material->SetEmissiveColor(aColor); },
				Colors::Black)
				.NameSlot().Label("Color")
				.ValueSlot().ColorPicker();

			//Strength:
			emissionGroupBuilder.AddProperty<float>("Emissive Strength",
				[&detailsContext]() { return detailsContext.Material->GetEmissiveIntensity(); },
				[this, &detailsContext](const float& aValue) { ScopedSuspend scopedSuspend(m_SuspendRefresh); detailsContext.Material->SetEmissiveIntensity(aValue); },
				0.0f)
				.NameSlot().Label("Strength")
				.ValueSlot().SpinBox().Range(0.0f, FLT_MAX).Delta(0.01f);
		}
		
		//Opacity:
		{
			auto opacityGroupBuilder = categoryBuilder.EditGroup("Opacity");

			//Map:
			AddTextureMapProperty(ETextureType::Opacity, "Opacity Map", opacityGroupBuilder);

			//Alpha CutOff:
			opacityGroupBuilder.AddProperty<float>("Alpha CutOff",
				[&detailsContext]() { return detailsContext.Material->GetAlphaCutOff(); },
				[this, &detailsContext](const float& aValue) { ScopedSuspend scopedSuspend(m_SuspendRefresh); detailsContext.Material->SetAlphaCutOff(aValue); },
				0.5f)
				.NameSlot().Label("Alpha Cutoff")
				.ValueSlot().Slider().Range(0.0f, 1.0f);

			//IOR:
			opacityGroupBuilder.AddProperty<float>("IOR",
				[&detailsContext]() { return detailsContext.Material->GetIOR(); },
				[this, &detailsContext](const float& aValue) { ScopedSuspend scopedSuspend(m_SuspendRefresh); detailsContext.Material->SetIOR(aValue); },
				1.5f)
				.NameSlot().Label("IOR")
				.ValueSlot().Slider().Range(1.0f, 3.0f);

			//Refraction Strength:
			opacityGroupBuilder.AddProperty<float>("Refraction Strength",
				[&detailsContext]() { return detailsContext.Material->GetRefractionStrength(); },
				[this, &detailsContext](const float& aValue) { ScopedSuspend scopedSuspend(m_SuspendRefresh); detailsContext.Material->SetRefractionStrength(aValue); },
				0.0f)
				.NameSlot().Label("Refraction Strength")
				.ValueSlot().Slider().Range(0.0f, 1.0f);
		}

		//Tiling:
		{
			categoryBuilder.AddProperty<Vector2>("Global Tiling", [&detailsContext]() { return detailsContext.Material->GetGlobalTilingFactor(); }, 
				[this, &detailsContext](const Vector2& aValue)
				{
					ScopedSuspend scopedSuspend(m_SuspendRefresh);
					detailsContext.Material->SetGlobalTilingFactor(aValue);
				},
				Vector2::One)
				.NameSlot().Label("Global Tiling")
				.ValueSlot().SpinBox().Range(1.0f, FLT_MAX).Delta(0.01f);
		}

		//Offset:
		{
			categoryBuilder.AddProperty<Vector2>("Global Offset", 
				[&detailsContext]() { return detailsContext.Material->GetGlobalOffset(); },
				[this, &detailsContext](const Vector2& aValue)
				{
					ScopedSuspend scopedSuspend(m_SuspendRefresh);
					detailsContext.Material->SetGlobalOffset(aValue);
				},
				Vector2::Zero)
				.NameSlot().Label("Global Offset")
				.ValueSlot().SpinBox().Range(-FLT_MAX, FLT_MAX).Delta(0.01f);
		}
	}

	void MaterialDetailCustomization::OnDestroy(IDetailLayoutBuilder& aDetailLayoutBuilder) noexcept
	{
		MaterialDetailsContext& detailsContext = aDetailLayoutBuilder.GetDetailsView()->GetContext<MaterialDetailsContext>();

		if (detailsContext.Material->OnPropertyChanged.IsConnected(m_OnAssetPropertyChangedCallbackID))
			detailsContext.Material->OnPropertyChanged.Detach(m_OnAssetPropertyChangedCallbackID);

		m_OnAssetPropertyChangedCallbackID = INVALID_CALLBACK_ID;
	}

	bool MaterialDetailCustomization::ShouldCustomize(IDetailLayoutBuilder& aDetailLayoutBuilder) const noexcept
	{
		return aDetailLayoutBuilder.GetDetailsView()->GetContext<MaterialDetailsContext>().MaterialHandle != AssetHandle::INVALID;
	}
}