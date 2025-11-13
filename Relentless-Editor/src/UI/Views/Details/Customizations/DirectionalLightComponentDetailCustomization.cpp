#include "DirectionalLightComponentDetailCustomization.h"

#include "../../../../Core/Selection.h"
#include "../LayoutBuilders/EntityDetailLayoutBuilder.h"
#include "../TableRows/EntityDetailRow.h"

namespace Relentless
{
	void DirectionalLightComponentDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& aDetailLayoutBuilder) noexcept
	{
		EntityDetailLayoutBuilder& builder = static_cast<EntityDetailLayoutBuilder&>(aDetailLayoutBuilder);
		IDetailCategoryBuilder& categoryBuilder = builder.EditCategory("Light");

		DetailNode& typeNode = categoryBuilder.AddProperty("Type");
		typeNode.OnRequestRow(this, &DirectionalLightComponentDetailCustomization::OnRequestTypeRow);

		DetailNode& intensityNode = categoryBuilder.AddProperty("Intensity");
		intensityNode.OnRequestRow(this, &DirectionalLightComponentDetailCustomization::OnRequestIntensityRow);

		DetailNode& colorNode = categoryBuilder.AddProperty("Light Color");
		colorNode.OnRequestRow(this, &DirectionalLightComponentDetailCustomization::OnRequestColorRow);

		DetailNode& useTempNode = categoryBuilder.AddProperty("Use Temperature");
		useTempNode.OnRequestRow(this, &DirectionalLightComponentDetailCustomization::OnRequestUseTemperatureRow);
		
		DetailNode& temperatureNode = categoryBuilder.AddProperty("Temperature");
		temperatureNode.OnRequestRow(this, &DirectionalLightComponentDetailCustomization::OnRequestTemperatureRow);

		m_pBuilder = &builder;
	}

	Color DirectionalLightComponentDetailCustomization::GetColor() const noexcept
	{
		return m_pBuilder->GetScene().GetEntityManager().Get<DirectionalLightComponent>(m_pBuilder->GetSelection().GetFirstSelected()).Color;
	}

	float DirectionalLightComponentDetailCustomization::GetIntensity() const noexcept
	{
		return Math::RadiantIrradianceToLux(m_pBuilder->GetScene().GetEntityManager().Get<DirectionalLightComponent>(m_pBuilder->GetSelection().GetFirstSelected()).Intensity);
	}

	float DirectionalLightComponentDetailCustomization::GetTemperature() const noexcept
	{
		return m_pBuilder->GetScene().GetEntityManager().Get<DirectionalLightComponent>(m_pBuilder->GetSelection().GetFirstSelected()).Temperature;
	}

	bool DirectionalLightComponentDetailCustomization::GetUseTemperature() const noexcept
	{
		return m_pBuilder->GetScene().GetEntityManager().Get<DirectionalLightComponent>(m_pBuilder->GetSelection().GetFirstSelected()).UseTemperature;
	}

	bool DirectionalLightComponentDetailCustomization::IsTemperatureEnabled() const noexcept
	{
		EntityManager& entityManager = m_pBuilder->GetScene().GetEntityManager();
		const std::vector<entity>& selectedEntities = m_pBuilder->GetSelection().GetSelectedEntities();

		return std::ranges::all_of(selectedEntities, [&entityManager](entity aEntity) { return entityManager.Get<DirectionalLightComponent>(aEntity).UseTemperature; });
	}

	void DirectionalLightComponentDetailCustomization::OnColorChanged(const Color& aColor) noexcept
	{
		EntityManager& entityManager = m_pBuilder->GetScene().GetEntityManager();

		const std::vector<entity>& selectedEntities = m_pBuilder->GetSelection().GetSelectedEntities();
		for (entity selectedEntity : selectedEntities)
			entityManager.Get<DirectionalLightComponent>(selectedEntity).Color = aColor;
	}

	void DirectionalLightComponentDetailCustomization::OnIntensityChanged(float aIntensity) noexcept
	{
		const float radiantIrradiance = Math::LuxToRadiantIrradiance(aIntensity);

		EntityManager& entityManager = m_pBuilder->GetScene().GetEntityManager();

		const std::vector<entity>& selectedEntities = m_pBuilder->GetSelection().GetSelectedEntities();
		for (entity selectedEntity : selectedEntities)
			entityManager.Get<DirectionalLightComponent>(selectedEntity).Intensity = radiantIrradiance;
	}

	void DirectionalLightComponentDetailCustomization::OnTemperatureChanged(float aTemperature) noexcept
	{
		EntityManager& entityManager = m_pBuilder->GetScene().GetEntityManager();

		const std::vector<entity>& selectedEntities = m_pBuilder->GetSelection().GetSelectedEntities();
		for (entity selectedEntity : selectedEntities)
			entityManager.Get<DirectionalLightComponent>(selectedEntity).Temperature = aTemperature;
	}

	void DirectionalLightComponentDetailCustomization::OnUseTemperatureCheckStateChanged(bool aState) noexcept
	{
		EntityManager& entityManager = m_pBuilder->GetScene().GetEntityManager();

		const std::vector<entity>& selectedEntities = m_pBuilder->GetSelection().GetSelectedEntities();
		for (entity selectedEntity : selectedEntities)
			entityManager.Get<DirectionalLightComponent>(selectedEntity).UseTemperature = aState;

		m_pTemperatureSlider->SetIsEnabled(!m_pTemperatureSlider->IsEnabled());
	}

	Ref<ITableRow> DirectionalLightComponentDetailCustomization::OnRequestColorRow(const ItemInfo& aItemInfo) noexcept
	{
		Ref<EntityDetailRow> pRow = new EntityDetailRow();

		{
			Ref<HorizontalBoxEx> pBox = new HorizontalBoxEx();

			pBox->AddWidget(new Label("Light Color"));
			pRow->SetColumnWidget(0, pBox);
		}
		{
			Ref<HorizontalBoxEx> pBox = new HorizontalBoxEx();

			pBox->AddWidget(new ColorPicker(Vector2(180.0f, 30.0f)))
				->Value(this, &DirectionalLightComponentDetailCustomization::GetColor)
				->OnValueChanged(this, &DirectionalLightComponentDetailCustomization::OnColorChanged)
				->SetMargin(IntRect(0.0f, 3.0f, 0.0f, 3.0f));

			pRow->SetColumnWidget(1, pBox);
		}

		return pRow;
	}

	Ref<ITableRow> DirectionalLightComponentDetailCustomization::OnRequestTypeRow(const ItemInfo& aItemInfo) noexcept
	{
		Ref<EntityDetailRow> pRow = new EntityDetailRow();

		{
			Ref<HorizontalBoxEx> pBox = new HorizontalBoxEx();

			pBox->AddWidget(new Label("Type"));
			pRow->SetColumnWidget(0, pBox);
		}
		{
			Ref<HorizontalBoxEx> pBox = new HorizontalBoxEx();

			pBox->AddWidget(new ComboBox())
				->AddSelectables({ "Directional", "Point", "Spot" })
				->SetMargin(IntRect(0.0f, 3.0f, 0.0f, 3.0f));

			pRow->SetColumnWidget(1, pBox);
		}

		return pRow;
	}

	Ref<ITableRow> DirectionalLightComponentDetailCustomization::OnRequestIntensityRow(const ItemInfo& aItemInfo) noexcept
	{
		Ref<EntityDetailRow> pRow = new EntityDetailRow();

		{
			Ref<HorizontalBoxEx> pBox = new HorizontalBoxEx();

			pBox->AddWidget(new Label("Intensity"));
			pRow->SetColumnWidget(0, pBox);
		}
		{
			Ref<HorizontalBoxEx> pBox = new HorizontalBoxEx();

			pBox->AddWidget(new FloatSlider(0.0f, 120'000.0f, "%.1f lux", ImGuiSliderFlags_Logarithmic))
				->Value(this, &DirectionalLightComponentDetailCustomization::GetIntensity)
				->OnValueChanged(this, &DirectionalLightComponentDetailCustomization::OnIntensityChanged)
				->SetMargin(IntRect(0.0f, 3.0f, 0.0f, 3.0f));

			pRow->SetColumnWidget(1, pBox);
		}

		return pRow;
	}

	Ref<ITableRow> DirectionalLightComponentDetailCustomization::OnRequestTemperatureRow(const ItemInfo& aItemInfo) noexcept
	{
		Ref<EntityDetailRow> pRow = new EntityDetailRow();

		{
			Ref<HorizontalBoxEx> pBox = new HorizontalBoxEx();

			pBox->AddWidget(new Label("Temperature"));
			pRow->SetColumnWidget(0, pBox);
		}
		{
			Ref<HorizontalBoxEx> pBox = new HorizontalBoxEx();

			m_pTemperatureSlider = pBox->AddWidget(new FloatSlider(1'700.0f, 12'000.0f, "%.3f K"))
				->Value(this, &DirectionalLightComponentDetailCustomization::GetTemperature)
				->OnValueChanged(this, &DirectionalLightComponentDetailCustomization::OnTemperatureChanged)
				->SetMargin(IntRect(0.0f, 3.0f, 0.0f, 3.0f));

			m_pTemperatureSlider->SetIsEnabled(IsTemperatureEnabled());

			pRow->SetColumnWidget(1, pBox);
		}

		return pRow;
	}
	
	Ref<ITableRow> DirectionalLightComponentDetailCustomization::OnRequestUseTemperatureRow(const ItemInfo& aItemInfo) noexcept
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
				->Value(this, &DirectionalLightComponentDetailCustomization::GetUseTemperature)
				->OnCheckStateChanged(this, &DirectionalLightComponentDetailCustomization::OnUseTemperatureCheckStateChanged)
				->SetMargin(IntRect(0.0f, 3.0f, 0.0f, 3.0f));

			pRow->SetColumnWidget(1, pBox);
		}

		return pRow;
	}
}
