#pragma once
#include "EntityDetailCustomization.h"

#include "../TableRows/EntityDetailRow.h"

namespace Relentless
{
	class IDetailLayoutBuilder;
	class EntityDetailLayoutBuilder;

	enum class EIntensityUnit : uint8 { Candelas = 0u, Lumens, Lux };

	template<typename ComponentType>
	class ILightComponentDetailCustomization : public EntityDetailCustomization
	{
	public:
		inline static constexpr ComboBox::SelectionInfo DEFAULT_INTENSITY_UNIT_SELECTION	= ComboBox::SelectionInfo{ "Candelas", 0 };
		inline static constexpr Color DEFAULT_LIGHT_COLOR									= Color(1.0f, 1.0f, 1.0f, 1.0f);
		inline static constexpr float DEFAULT_INTENSITY										= 8.0f;
		inline static constexpr float DEFAULT_ATTENUATION_RADIUS							= 10.0f;
		inline static constexpr float DEFAULT_INNER_CONE_ANGLES_DEGREES						= 0.0f;
		inline static constexpr float DEFAULT_OUTER_CONE_ANGLES_DEGREES						= 44.0f;
		inline static constexpr float DEFAULT_TEMPERATURE									= 6'500.0f;
		inline static constexpr bool DEFAULT_USE_TEMPERATURE								= false;

		NO_DISCARD Ref<ITableRow> OnRequestColorRow(const ItemInfo& aItemInfo) noexcept;
		NO_DISCARD Ref<ITableRow> OnRequestTypeRow(const ItemInfo& aItemInfo) noexcept;
		NO_DISCARD Ref<ITableRow> OnRequestIntensityRow(const ItemInfo& aItemInfo) noexcept;
		NO_DISCARD Ref<ITableRow> OnRequestIntensityUnitsRow(const ItemInfo& aItemInfo) noexcept;
		NO_DISCARD Ref<ITableRow> OnRequestTemperatureRow(const ItemInfo& aItemInfo) noexcept;
		NO_DISCARD Ref<ITableRow> OnRequestUseTemperatureRow(const ItemInfo& aItemInfo) noexcept;
	private:
		NO_DISCARD Color GetColor() const noexcept;
		NO_DISCARD float GetIntensity() const noexcept;
		NO_DISCARD float GetTemperature() const noexcept;
		NO_DISCARD bool GetUseTemperature() const noexcept;

		NO_DISCARD bool IsColorDefaultForInspected() const noexcept;
		NO_DISCARD bool IsIntensityDefaultForInspected() const noexcept;
		NO_DISCARD bool IsIntensityUnitDefaultForInspected() const noexcept;
		NO_DISCARD bool IsTemperatureDefaultForInspected() const noexcept;
		NO_DISCARD bool IsUseTemperatureDefaultForInspected() const noexcept;

		NO_DISCARD bool IsTemperatureEnabled() const noexcept;

		void OnColorChanged(const Color& aColor) noexcept;
		void OnIntensityChanged(float aIntensity) noexcept;
		void OnIntensityUnitChanged(const ComboBox::SelectionInfo& aSelectionInfo) noexcept;
		void OnTemperatureChanged(float aTemperature) noexcept;
		void OnTypeChanged(const ComboBox::SelectionInfo& aSelectionInfo) noexcept;
		void OnUseTemperatureCheckStateChanged(bool aState) noexcept;
	private:
		ComboBox* m_pIntensityUnitComboBox = nullptr;
		FloatSlider* m_pTemperatureSlider = nullptr;
		FloatSlider* m_pIntensitySlider = nullptr;
		EIntensityUnit m_IntensityUnit = EIntensityUnit::Candelas;

		Button* m_pRevertColorButton = nullptr;
		Button* m_pRevertIntensityButton = nullptr;
		Button* m_pRevertIntensityUnitButton = nullptr;
		Button* m_pRevertTemperatureButton = nullptr;
		Button* m_pRevertUseTemperatureButton = nullptr;
	};

	template<typename ComponentType>
	bool ILightComponentDetailCustomization<ComponentType>::IsColorDefaultForInspected() const noexcept
	{
		Scene& scene = m_pBuilder->GetScene();
		EntityManager& entityManager = scene.GetEntityManager();

		return std::ranges::all_of(GetInspectedEntities(), [this, &entityManager](entity aEntity)
			{
				return entityManager.Get<ComponentType>(aEntity).GetColor() == DEFAULT_LIGHT_COLOR;
			});
	}

	template<typename ComponentType>
	bool ILightComponentDetailCustomization<ComponentType>::IsIntensityDefaultForInspected() const noexcept
	{
		Scene& scene = m_pBuilder->GetScene();
		EntityManager& entityManager = scene.GetEntityManager();

		return std::ranges::all_of(GetInspectedEntities(), [this, &entityManager](entity aEntity)
			{
				ComponentType& component = entityManager.Get<ComponentType>(aEntity);

				if constexpr(std::is_same_v<ComponentType, DirectionalLightComponent>)
					return Math::AreValuesClose(component.GetIntensity(), 100'000.0f);
				else
					return Math::AreValuesClose(component.GetIntensity(), 8.0f);
			});
	}

	template<typename ComponentType>
	bool ILightComponentDetailCustomization<ComponentType>::IsIntensityUnitDefaultForInspected() const noexcept
	{
		return m_IntensityUnit == static_cast<EIntensityUnit>(DEFAULT_INTENSITY_UNIT_SELECTION.Index);
	}

	template<typename ComponentType>
	bool ILightComponentDetailCustomization<ComponentType>::IsTemperatureDefaultForInspected() const noexcept
	{
		Scene& scene = m_pBuilder->GetScene();
		EntityManager& entityManager = scene.GetEntityManager();

		return std::ranges::all_of(GetInspectedEntities(), [this, &entityManager](entity aEntity)
			{
				ComponentType& component = entityManager.Get<ComponentType>(aEntity);
				return Math::AreValuesClose(component.GetTemperature(), DEFAULT_TEMPERATURE);
			});
	}

	template<typename ComponentType>
	bool ILightComponentDetailCustomization<ComponentType>::IsTemperatureEnabled() const noexcept
	{
		EntityManager& entityManager = m_pBuilder->GetScene().GetEntityManager();
		return std::ranges::all_of(GetInspectedEntities(), [&entityManager](entity aEntity) { return entityManager.Get<ComponentType>(aEntity).IsUsingTemperature(); });
	}

	template<typename ComponentType>
	bool ILightComponentDetailCustomization<ComponentType>::IsUseTemperatureDefaultForInspected() const noexcept
	{
		Scene& scene = m_pBuilder->GetScene();
		EntityManager& entityManager = scene.GetEntityManager();

		return std::ranges::all_of(GetInspectedEntities(), [this, &entityManager](entity aEntity)
			{
				return entityManager.Get<ComponentType>(aEntity).IsUsingTemperature() == DEFAULT_USE_TEMPERATURE;
			});
	}

	template<typename ComponentType>
	Color ILightComponentDetailCustomization<ComponentType>::GetColor() const noexcept
	{
		return m_pBuilder->GetScene().GetEntityManager().Get<ComponentType>(GetPrimaryEntity()).GetColor();
	}

	template<typename ComponentType>
	float ILightComponentDetailCustomization<ComponentType>::GetIntensity() const noexcept
	{
		ComponentType& lightComponent = m_pBuilder->GetScene().GetEntityManager().Get<ComponentType>(GetPrimaryEntity());

		switch (m_IntensityUnit)
		{
		case EIntensityUnit::Candelas:
		case EIntensityUnit::Lux:
			return lightComponent.GetIntensity();
		case EIntensityUnit::Lumens:
		{
			if constexpr (std::is_same_v<ComponentType, PointLightComponent>)
				return Math::Photometry::CandelaToLumen_Point(lightComponent.GetIntensity());
			else if constexpr (std::is_same_v<ComponentType, SpotLightComponent>)
				return Math::Photometry::CandelaToLumen_Spot(lightComponent.GetIntensity(), lightComponent.GetOuterConeAngleRadians());
		}
		default:
			RLS_ASSERT(false, "[ILightComponentDetailCustomization::GetIntensity]: Unknown intensity unit encountered.");
		}

		return 0.0f;
	}

	template<typename ComponentType>
	float ILightComponentDetailCustomization<ComponentType>::GetTemperature() const noexcept
	{
		return m_pBuilder->GetScene().GetEntityManager().Get<ComponentType>(GetPrimaryEntity()).GetTemperature();
	}

	template<typename ComponentType>
	bool ILightComponentDetailCustomization<ComponentType>::GetUseTemperature() const noexcept
	{
		return m_pBuilder->GetScene().GetEntityManager().Get<ComponentType>(GetPrimaryEntity()).IsUsingTemperature();
	}

	template<typename ComponentType>
	void ILightComponentDetailCustomization<ComponentType>::OnColorChanged(const Color& aColor) noexcept
	{
		EntityManager& entityManager = m_pBuilder->GetScene().GetEntityManager();
		std::ranges::for_each(GetInspectedEntities(), [&](entity aEntity) {	entityManager.Get<ComponentType>(aEntity).SetColor(aColor); });

		m_pRevertColorButton->SetIsVisible(!IsColorDefaultForInspected());
	}

	template<typename ComponentType>
	void ILightComponentDetailCustomization<ComponentType>::OnIntensityChanged(float aIntensity) noexcept
	{
		EntityManager& entityManager = m_pBuilder->GetScene().GetEntityManager();

		std::ranges::for_each(GetInspectedEntities(), [&](entity aEntity)
			{
				ComponentType& component = entityManager.Get<ComponentType>(aEntity);

				if constexpr (std::is_same_v<ComponentType, DirectionalLightComponent>)
					component.SetIntensityLux(aIntensity);
				else
				{
					switch (m_IntensityUnit)
					{
					case EIntensityUnit::Candelas:
						component.SetIntensityCandela(aIntensity);
						break;
					case EIntensityUnit::Lumens:
						component.SetIntensityLumen(aIntensity);
						break;
					}
				}
			});

		m_pRevertIntensityButton->SetIsVisible(!IsIntensityDefaultForInspected());
	}

	template<typename ComponentType>
	void ILightComponentDetailCustomization<ComponentType>::OnIntensityUnitChanged(const ComboBox::SelectionInfo& aSelectionInfo) noexcept
	{
		m_IntensityUnit = static_cast<EIntensityUnit>(aSelectionInfo.Index);

		switch (m_IntensityUnit)
		{
		case EIntensityUnit::Candelas:
			m_pIntensitySlider->SetMinValue(0.0f);
			m_pIntensitySlider->SetMaxValue(160.0f);
			m_pIntensitySlider->SetFormat("%.3f cd");
			break;
		case EIntensityUnit::Lumens:
			m_pIntensitySlider->SetMinValue(0.0f);
			m_pIntensitySlider->SetMaxValue(2'010.619263f);
			m_pIntensitySlider->SetFormat("%.3f lm");
			break;
		case EIntensityUnit::Lux:
			m_pIntensitySlider->SetMinValue(0.0f);
			m_pIntensitySlider->SetMaxValue(120'000.0f);
			m_pIntensitySlider->SetFormat("%.1f lux");
			break;
		default:
			RLS_ASSERT(false, "[ILightComponentDetailCustomization::OnIntensityUnitChanged]: Unknown intensity unit encountered.");
			break;
		}

		m_pRevertIntensityUnitButton->SetIsVisible(!IsIntensityUnitDefaultForInspected());
	}

	template<typename ComponentType>
	void ILightComponentDetailCustomization<ComponentType>::OnTemperatureChanged(float aTemperature) noexcept
	{
		EntityManager& entityManager = m_pBuilder->GetScene().GetEntityManager();
		std::ranges::for_each(GetInspectedEntities(), [&](entity aEntity) {	entityManager.Get<ComponentType>(aEntity).SetTemperature(aTemperature); });

		m_pRevertTemperatureButton->SetIsVisible(!IsTemperatureDefaultForInspected());
	}

	template<typename ComponentType>
	void ILightComponentDetailCustomization<ComponentType>::OnTypeChanged(const ComboBox::SelectionInfo& aSelectionInfo) noexcept
	{
		Scene& scene = m_pBuilder->GetScene();
		EntityManager& entityManager = scene.GetEntityManager();

		const ELightType lightType = static_cast<const ELightType>(aSelectionInfo.Index);

		EditComponentData<ComponentType>(scene, [&](entity aEntity, ComponentType& aComponent)
			{
				switch (lightType)
				{
				case ELightType::Directional:
				{
					auto& dlc = entityManager.Add<DirectionalLightComponent>(aEntity);
					dlc.SetColor(aComponent.GetColor());
					dlc.SetIntensityLux(aComponent.GetIntensity());
					dlc.SetTemperature(aComponent.GetTemperature());
					dlc.SetUseTemperature(aComponent.IsUsingTemperature());
					break;
				}
				case ELightType::Point:
				{
					auto& plc = entityManager.Add<PointLightComponent>(aEntity);
					plc.SetColor(aComponent.GetColor());
					plc.SetIntensityCandela(aComponent.GetIntensity());
					plc.SetTemperature(aComponent.GetTemperature());
					plc.SetUseTemperature(aComponent.IsUsingTemperature());
					break;
				}
				case ELightType::Spot:
				{
					auto& slc = entityManager.Add<SpotLightComponent>(aEntity);
					slc.SetColor(aComponent.GetColor());
					slc.SetIntensityCandela(aComponent.GetIntensity());
					slc.SetTemperature(aComponent.GetTemperature());
					slc.SetUseTemperature(aComponent.IsUsingTemperature());
					break;
				}
				}

				Application::Get().SubmitToMainThread([aEntity, &entityManager]()
					{
						entityManager.Remove<ComponentType>(aEntity);
					});

			});

		m_pBuilder->RequestRefresh();
	}

	template<typename ComponentType>
	void ILightComponentDetailCustomization<ComponentType>::OnUseTemperatureCheckStateChanged(bool aState) noexcept
	{
		EntityManager& entityManager = m_pBuilder->GetScene().GetEntityManager();
		std::ranges::for_each(GetInspectedEntities(), [&](entity aEntity) {	entityManager.Get<ComponentType>(aEntity).SetUseTemperature(aState); });

		m_pTemperatureSlider->SetIsEnabled(!m_pTemperatureSlider->IsEnabled());
		m_pRevertUseTemperatureButton->SetIsVisible(!IsUseTemperatureDefaultForInspected());
	}

	template<typename ComponentType>
	Ref<ITableRow> ILightComponentDetailCustomization<ComponentType>::OnRequestColorRow(const ItemInfo& aItemInfo) noexcept
	{
		Ref<EntityDetailRow> pRow = new EntityDetailRow();

		{
			Ref<HorizontalBoxEx> pBox = new HorizontalBoxEx();

			pBox->AddWidget(new Label("Light Color"));
			pRow->SetColumnWidget(0, pBox);
		}
		{
			Ref<HorizontalBoxEx> pBox = new HorizontalBoxEx();

			Ref<HorizontalBoxEx> pInnerBox = new HorizontalBoxEx(Vector2(140.0f, 32.0f), true);
			pInnerBox->SetSpacing(0.0f);
			pInnerBox->SetSizePolicy(ESizePolicy::Fixed);
			pInnerBox->SetMargin(FloatRect(0.0f, 3.0f, 0.0f, 3.0f));

			pInnerBox->AddWidget(new ColorPicker(Vector2(180.0f, 30.0f)))
				->Value(this, &ILightComponentDetailCustomization::GetColor)
				->OnValueChanged(this, &ILightComponentDetailCustomization::OnColorChanged)
				->SetSizePolicy(ESizePolicy::Stretch);

			pBox->AddWidget(pInnerBox);
			pRow->SetColumnWidget(1, pBox);
		}
		{
			Ref<HorizontalBoxEx> pBox = new HorizontalBoxEx();
			m_pRevertColorButton = pBox->AddWidget(AddRevertButtonWidget([this](const Color& aColor) { OnColorChanged(aColor); }, DEFAULT_LIGHT_COLOR, !IsColorDefaultForInspected()));
			pRow->SetColumnWidget(2, pBox);
		}

		return pRow;
	}

	template<typename ComponentType>
	Ref<ITableRow> ILightComponentDetailCustomization<ComponentType>::OnRequestTypeRow(const ItemInfo& /*aItemInfo*/) noexcept
	{
		Ref<EntityDetailRow> pRow = new EntityDetailRow();

		{
			Ref<HorizontalBoxEx> pBox = new HorizontalBoxEx();

			pBox->AddWidget(new Label("Type"));
			pRow->SetColumnWidget(0, pBox);
		}
		{
			Ref<HorizontalBoxEx> pBox = new HorizontalBoxEx();

			Ref<HorizontalBoxEx> pInnerBox = new HorizontalBoxEx(Vector2(140.0f, 32.0f), true);
			pInnerBox->SetSpacing(0.0f);
			pInnerBox->SetSizePolicy(ESizePolicy::Fixed);
			pInnerBox->SetMargin(FloatRect(0.0f, 3.0f, 0.0f, 3.0f));

			int index = -1;

			if constexpr (std::is_same_v<ComponentType, DirectionalLightComponent>)
				index = 0;
			else if constexpr (std::is_same_v<ComponentType, PointLightComponent>)
				index = 1;
			else if constexpr (std::is_same_v<ComponentType, SpotLightComponent>)
				index = 2;

			pInnerBox->AddWidget(new ComboBox())
				->AddSelectables({ "Directional", "Point", "Spot" })
				->SetSelectedItem(index)
				->OnSelectionChanged(this, &ILightComponentDetailCustomization::OnTypeChanged)
				->SetSizePolicy(ESizePolicy::Stretch);

			pBox->AddWidget(pInnerBox);
			pRow->SetColumnWidget(1, pBox);
		}
		{
			Ref<HorizontalBoxEx> pBox = new HorizontalBoxEx();
			pBox->AddWidget(new Button(ICON_FA_ARROW_ROTATE_LEFT, Vector2(32.0f, 32.0f)))
				->SetBackgroundColor(Colors::Transparent)
				->SetActiveColor(Colors::Transparent)
				->SetHoverColor(Colors::Transparent)
				->SetBorderColor(Colors::Transparent)
				->SetFont(ImGui::GetIO().Fonts->Fonts[2])
				->SetIsVisible(false);

			pRow->SetColumnWidget(2, pBox);
		}

		return pRow;
	}

	template<typename ComponentType>
	Ref<ITableRow> ILightComponentDetailCustomization<ComponentType>::OnRequestIntensityRow(const ItemInfo& /*aItemInfo*/) noexcept
	{
		Ref<EntityDetailRow> pRow = new EntityDetailRow();

		{
			Ref<HorizontalBoxEx> pBox = new HorizontalBoxEx();

			pBox->AddWidget(new Label("Intensity"));
			pRow->SetColumnWidget(0, pBox);
		}
		{
			Ref<HorizontalBoxEx> pBox = new HorizontalBoxEx();

			Ref<HorizontalBoxEx> pInnerBox = new HorizontalBoxEx(Vector2(140.0f, 32.0f), true);
			pInnerBox->SetSpacing(0.0f);
			pInnerBox->SetSizePolicy(ESizePolicy::Fixed);
			pInnerBox->SetMargin(FloatRect(0.0f, 3.0f, 0.0f, 3.0f));

			const char* format = "?";
			constexpr float minRange = 0.0f;
			float maxRange = -1.0f;

			if constexpr (std::is_same_v<ComponentType, DirectionalLightComponent>)
				m_IntensityUnit = EIntensityUnit::Lux;

			switch (m_IntensityUnit)
			{
				case EIntensityUnit::Candelas:
					format = "%.3f cd";
					maxRange = 160.0f;
					break;
				case EIntensityUnit::Lumens:
					format = "%.3f lm";
					maxRange = 2'010.619263f;
					break;
				case EIntensityUnit::Lux:
					format = "%.1f lux";
					maxRange = 120'000.0f;
					break;
				default:
					RLS_ASSERT(false, "[ILightComponentDetailCustomization::OnRequestIntensityRow]: Unknown intensity unit encountered.");
					break;
			}

			m_pIntensitySlider = pInnerBox->AddWidget(new FloatSlider(minRange, maxRange, format))
				->Value(this, &ILightComponentDetailCustomization::GetIntensity)
				->OnValueChanged(this, &ILightComponentDetailCustomization::OnIntensityChanged);

			m_pIntensitySlider->SetSizePolicy(ESizePolicy::Stretch);

			pBox->AddWidget(pInnerBox);
			pRow->SetColumnWidget(1, pBox);
		}
		{
			Ref<HorizontalBoxEx> pBox = new HorizontalBoxEx();
			m_pRevertIntensityButton = pBox->AddWidget(AddRevertButtonWidget([this](float aValue) 
				{ 
					if constexpr (std::is_same_v<ComponentType, DirectionalLightComponent>)
						aValue = 100'000.0f;
					else if constexpr (std::is_same_v<ComponentType, PointLightComponent>)
					{
						if (m_IntensityUnit == EIntensityUnit::Lumens)
							aValue = Math::Photometry::CandelaToLumen_Point(DEFAULT_INTENSITY);
					}
					else if constexpr (std::is_same_v<ComponentType, SpotLightComponent>)
					{
						if (m_IntensityUnit == EIntensityUnit::Lumens)
							aValue = Math::Photometry::CandelaToLumen_Spot(DEFAULT_INTENSITY, m_pBuilder->GetScene().GetEntityManager().Get<SpotLightComponent>(GetPrimaryEntity()).GetOuterConeAngleRadians());
					}
					
					OnIntensityChanged(aValue);
				}, 
					DEFAULT_INTENSITY, !IsIntensityDefaultForInspected()));
			pRow->SetColumnWidget(2, pBox);
		}

		return pRow;
	}

	template<typename ComponentType>
	Ref<ITableRow> ILightComponentDetailCustomization<ComponentType>::OnRequestIntensityUnitsRow(const ItemInfo& aItemInfo) noexcept
	{
		Ref<EntityDetailRow> pRow = new EntityDetailRow();

		{
			Ref<HorizontalBoxEx> pBox = new HorizontalBoxEx();

			pBox->AddWidget(new Label("Intensity Units"));
			pRow->SetColumnWidget(0, pBox);
		}
		{
			Ref<HorizontalBoxEx> pBox = new HorizontalBoxEx();

			Ref<HorizontalBoxEx> pInnerBox = new HorizontalBoxEx(Vector2(140.0f, 32.0f), true);
			pInnerBox->SetSpacing(0.0f);
			pInnerBox->SetMargin(FloatRect(0.0f, 3.0f, 0.0f, 3.0f));
			pInnerBox->SetSizePolicy(ESizePolicy::Fixed);

			m_pIntensityUnitComboBox = pInnerBox->AddWidget(new ComboBox())
				->AddSelectables({ "Candelas", "Lumens" })
				->OnSelectionChanged(this, &ILightComponentDetailCustomization::OnIntensityUnitChanged);

			m_pIntensityUnitComboBox->SetSizePolicy(ESizePolicy::Stretch);

			pBox->AddWidget(pInnerBox);
			pRow->SetColumnWidget(1, pBox);
		}
		{
			Ref<HorizontalBoxEx> pBox = new HorizontalBoxEx();
			m_pRevertIntensityUnitButton = pBox->AddWidget(AddRevertButtonWidget([this](const ComboBox::SelectionInfo& aSelectionInfo)
				{
					OnIntensityUnitChanged(aSelectionInfo);
					m_pIntensityUnitComboBox->SetSelectedItem(static_cast<int>(EIntensityUnit::Candelas));
				}, DEFAULT_INTENSITY_UNIT_SELECTION, !IsIntensityUnitDefaultForInspected()));
			pRow->SetColumnWidget(2, pBox);
		}

		return pRow;
	}

	template<typename ComponentType>
	Ref<ITableRow> ILightComponentDetailCustomization<ComponentType>::OnRequestTemperatureRow(const ItemInfo& aItemInfo) noexcept
	{
		Ref<EntityDetailRow> pRow = new EntityDetailRow();

		{
			Ref<HorizontalBoxEx> pBox = new HorizontalBoxEx();

			pBox->AddWidget(new Label("Temperature"));
			pRow->SetColumnWidget(0, pBox);
		}
		{
			Ref<HorizontalBoxEx> pBox = new HorizontalBoxEx();

			Ref<HorizontalBoxEx> pInnerBox = new HorizontalBoxEx(Vector2(140.0f, 32.0f), true);
			pInnerBox->SetSpacing(0.0f);
			pInnerBox->SetMargin(FloatRect(0.0f, 3.0f, 0.0f, 3.0f));
			pInnerBox->SetSizePolicy(ESizePolicy::Fixed);

			m_pTemperatureSlider = pInnerBox->AddWidget(new FloatSlider(1'700.0f, 12'000.0f, "%.3f K"))
				->Value(this, &ILightComponentDetailCustomization::GetTemperature)
				->OnValueChanged(this, &ILightComponentDetailCustomization::OnTemperatureChanged);

			m_pTemperatureSlider->SetIsEnabled(IsTemperatureEnabled());
			m_pTemperatureSlider->SetSizePolicy(ESizePolicy::Stretch);

			pBox->AddWidget(pInnerBox);
			pRow->SetColumnWidget(1, pBox);
		}
		{
			Ref<HorizontalBoxEx> pBox = new HorizontalBoxEx();
			m_pRevertTemperatureButton = pBox->AddWidget(AddRevertButtonWidget([this](float aValue) { OnTemperatureChanged(aValue); }, DEFAULT_TEMPERATURE, !IsTemperatureDefaultForInspected()));
			pRow->SetColumnWidget(2, pBox);
		}

		return pRow;
	}

	template<typename ComponentType>
	Ref<ITableRow> ILightComponentDetailCustomization<ComponentType>::OnRequestUseTemperatureRow(const ItemInfo& aItemInfo) noexcept
	{
		Ref<EntityDetailRow> pRow = new EntityDetailRow();

		{
			Ref<HorizontalBoxEx> pBox = new HorizontalBoxEx();

			pBox->AddWidget(new Label("Use Temperature"));
			pRow->SetColumnWidget(0, pBox);
		}
		{
			Ref<HorizontalBoxEx> pBox = new HorizontalBoxEx();

			pBox->AddWidget(new CheckBox())
				->Value(this, &ILightComponentDetailCustomization::GetUseTemperature)
				->OnCheckStateChanged(this, &ILightComponentDetailCustomization::OnUseTemperatureCheckStateChanged)
				->SetMargin(IntRect(0.0f, 3.0f, 0.0f, 3.0f));

			pRow->SetColumnWidget(1, pBox);
		}
		{
			Ref<HorizontalBoxEx> pBox = new HorizontalBoxEx();
			m_pRevertUseTemperatureButton = pBox->AddWidget(AddRevertButtonWidget([this](bool aValue) { OnUseTemperatureCheckStateChanged(aValue); }, DEFAULT_USE_TEMPERATURE, !IsUseTemperatureDefaultForInspected()));
			pRow->SetColumnWidget(2, pBox);
		}

		return pRow;
	}
}
