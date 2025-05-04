#include "DetailsPanel.h"
#include "../Core/Editor.h"

namespace Relentless
{
	DetailsPanel::DetailsPanel(const char* pName, ImGuiWindowFlags flags, Editor* pEditor) noexcept
		: PanelBase{ pName, flags}
		, m_pEditor{ pEditor }
	{
		m_pEditor->GetSelection()->OnSelectionChanged.Connect(this, &DetailsPanel::OnSelectionChanged);
	}

	DetailsPanel::~DetailsPanel() noexcept
	{
		if (m_pEditor)
		{
			if (const UniquePtr<Selection>& pSelection = m_pEditor->GetSelection())
				pSelection->OnSelectionChanged.Detach(this);
		}
	}

	Vector3 DetailsPanel::GetLocation(ComboBox* pTransformSpaceComboBox) const noexcept
	{
		const ETransformSpace transformSpace = static_cast<ETransformSpace>(pTransformSpaceComboBox->GetSelectedIndex());
		auto& selectedEntities = m_pEditor->GetSelection()->GetSelectedEntities();

		switch (transformSpace)
		{
		case ETransformSpace::Relative:
			return m_pEditor->GetActiveScene()->GetLocalLocation(selectedEntities[0]);
		case ETransformSpace::Absolute:
			return m_pEditor->GetActiveScene()->GetWorldLocation(selectedEntities[0]);
		default:
			RLS_ASSERT(false, "Unreachable.");
			return Vector3::Zero;
		}
	}

	Vector3 DetailsPanel::GetRotation(ComboBox* pTransformSpaceComboBox) const noexcept
	{
		const ETransformSpace transformSpace = static_cast<ETransformSpace>(pTransformSpaceComboBox->GetSelectedIndex());
		auto& selectedEntities = m_pEditor->GetSelection()->GetSelectedEntities();

		switch (transformSpace)
		{
		case ETransformSpace::Relative:
			return m_pEditor->GetActiveScene()->GetLocalRotation(selectedEntities[0]).ToEuler();
		case ETransformSpace::Absolute:
			return m_pEditor->GetActiveScene()->GetWorldRotation(selectedEntities[0]).ToEuler();
		default:
			RLS_ASSERT(false, "Unreachable.");
			return Vector3::Zero;
		}
	}

	Vector3 DetailsPanel::GetScale(ComboBox* pTransformSpaceComboBox) const noexcept
	{
		const ETransformSpace transformSpace = static_cast<ETransformSpace>(pTransformSpaceComboBox->GetSelectedIndex());
		auto& selectedEntities = m_pEditor->GetSelection()->GetSelectedEntities();

		switch (transformSpace)
		{
		case ETransformSpace::Relative:
			return m_pEditor->GetActiveScene()->GetLocalScale(selectedEntities[0]);
		case ETransformSpace::Absolute:
			return m_pEditor->GetActiveScene()->GetWorldScale(selectedEntities[0]);
		default:
			RLS_ASSERT(false, "Unreachable.");
			return Vector3::Zero;
		}
	}

	void DetailsPanel::OnLocationChanged(float value, EAxis axis, ETransformSpace space) noexcept
	{
		auto& selectedEntities = m_pEditor->GetSelection()->GetSelectedEntities();
		Scene* pScene = m_pEditor->GetActiveScene();

		std::for_each(selectedEntities.begin(), selectedEntities.end(), [&](entity e)
			{
				Vector3 location = space == ETransformSpace::Absolute ? pScene->GetWorldLocation(e) : pScene->GetLocalLocation(e);
				switch (axis)
				{
				case EAxis::X: location.x = value; break;
				case EAxis::Y: location.y = value; break;
				case EAxis::Z: location.z = value; break;
				}

				switch (space)
				{
				case ETransformSpace::Absolute: pScene->SetWorldLocation(e, location); break;
				case ETransformSpace::Relative: pScene->SetLocalLocation(e, location); break;
				}
			});
	}

	void DetailsPanel::OnRotationChanged(float valueDeg, EAxis axis, ETransformSpace space) noexcept
	{
		auto& selectedEntities = m_pEditor->GetSelection()->GetSelectedEntities();
		Scene* pScene = m_pEditor->GetActiveScene();

		std::for_each(selectedEntities.begin(), selectedEntities.end(), [&](entity e)
			{
				const Quaternion currentRotation = space == ETransformSpace::Absolute ? pScene->GetWorldRotation(e) : pScene->GetLocalRotation(e);

				const Vector3 eulerDeg = Math::RadToDeg(currentRotation.ToEuler());

				Vector3 axisVec = Vector3::Zero;
				float currentAngle = 0.0f;
				switch (axis)
				{
				case EAxis::X: axisVec = Vector3::Right;	currentAngle = eulerDeg.x; break;
				case EAxis::Y: axisVec = Vector3::Up;		currentAngle = eulerDeg.y; break;
				case EAxis::Z: axisVec = Vector3::Forward;	currentAngle = eulerDeg.z; break;
				}

				const float deltaAngleDeg = valueDeg - currentAngle;
				const float deltaAngleRad = Math::DegToRad(deltaAngleDeg);

				const Quaternion deltaRot = Quaternion::CreateFromAxisAngle(axisVec, deltaAngleRad);

				switch (space)
				{
					case ETransformSpace::Absolute: pScene->AddWorldRotation(e, Math::RadToDeg(deltaRot.ToEuler())); break;
					case ETransformSpace::Relative: pScene->AddLocalRotation(e, Math::RadToDeg(deltaRot.ToEuler())); break;
				}
			});
	}

	void DetailsPanel::OnScaleChanged(float value, EAxis axis, ETransformSpace space) noexcept
	{
		auto& selectedEntities = m_pEditor->GetSelection()->GetSelectedEntities();
		Scene* pScene = m_pEditor->GetActiveScene();

		std::for_each(selectedEntities.begin(), selectedEntities.end(), [&](entity e)
			{
				Vector3 scale = space == ETransformSpace::Absolute ? pScene->GetWorldScale(e) : pScene->GetLocalScale(e);
				switch (axis)
				{
				case EAxis::X: scale.x = value; break;
				case EAxis::Y: scale.y = value; break;
				case EAxis::Z: scale.z = value; break;
				}

				switch (space)
				{
				case ETransformSpace::Absolute: pScene->SetWorldScale(e, scale); break;
				case ETransformSpace::Relative: pScene->SetLocalScale(e, scale); break;
				}
			});
	}

	void DetailsPanel::OnSelectionChanged(entity, ESelectionState) noexcept
	{
		const std::vector<entity>& selectedEntities = m_pEditor->GetSelection()->GetSelectedEntities();
		if (selectedEntities.empty())
		{
			SetRoot(nullptr); //TODO label with "Select X to..."
			return;
		}

		EntityManager& entityManager = m_pEditor->GetActiveScene()->GetEntityManager();

		Ref<VerticalBox> pRoot = new VerticalBox("##DetailsPanel Root");
		ConditionallyCreateSection<TransformComponent>(pRoot, entityManager, selectedEntities);
		ConditionallyCreateSection<DirectionalLightComponent>(pRoot, entityManager, selectedEntities);

		SetRoot(pRoot);
	}

	template<>
	Ref<IWidget> DetailsPanel::CreateComponentSection<DirectionalLightComponent>(const std::vector<entity>& selectedEntities) noexcept
	{
		Ref<Table> pTable = new Table("##PropertyTable");

		Ref<FloatSlider> pIntensitySlider = new FloatSlider("##Intensity", 8.0f, 0.0f, 160.0f, "%.3f cd");
		pIntensitySlider->OnChanged.Connect([](float value) { RLS_CORE_INFO("{0}", value) });

		Ref<Label> pIntensityLabel = new Label("Intensity");

		Ref<Label> pUseTemperatureLabel = new Label("Use Temperature");
		Ref<Label> pTemperatureLabel = new Label("Temperature");

		Ref<FloatSlider> pTemperatureSlider = new FloatSlider("##TemperatureSlider", 6'500.0f, 1'700.0f, 12'000.0f, "%.3f K");
		pTemperatureSlider->OnChanged.Connect([](float value) { RLS_CORE_INFO("{0}", value) });
		pTemperatureSlider->SetIsEnabled(false);

		Ref<CheckBox> pUseTemperatureCheckbox = new CheckBox("##UseTemperatureCheckBox");
		pUseTemperatureCheckbox->OnCheckStateChanged.Connect([cb = pTemperatureSlider.Get()](bool state)
			{
				if (cb)
					cb->SetIsEnabled(state);
			});

		Ref<Label> pIntensityUnitsLabel = new Label("Intensity Units");
		Ref<ComboBox> pIntensityUnitsComboBox = new ComboBox("##IntensityUnitsCombo");
		pIntensityUnitsComboBox->AddSelectables({ "Candelas", "Lumens" });
		pIntensityUnitsComboBox->OnChanged.Connect([](int selected) {RLS_CORE_INFO("{0}", selected)});

		uint32 currentRow = 0;

		Ref<ComboBox> pTypeComboBox = new ComboBox("##LightTypeComboBox");
		pTypeComboBox->AddSelectables({ "Point", "Spot", "Directional" });

		pTable->Add(new Label("Type"), 0, currentRow);
		pTable->Add(pTypeComboBox, 1, currentRow);
		currentRow++;

		pTable->Add(pIntensityLabel, 0, currentRow);
		pTable->Add(pIntensitySlider, 1, currentRow);
		currentRow++;

		pTable->Add(pIntensityUnitsLabel, 0, currentRow);
		pTable->Add(pIntensityUnitsComboBox, 1, currentRow);
		currentRow++;

		Ref<Label> pColorLabel = new Label("Light Color");
		Ref<ColorPicker> pColorPicker = new ColorPicker("##LightColorPicker", Colors::White, Vector2(180.0f, 30.0f));

		pTable->Add(pColorLabel, 0, currentRow);
		pTable->Add(pColorPicker, 1, currentRow);
		currentRow++;

		Ref<Label> pAttenuationRadiusLabel = new Label("Attenuation Radius");
		Ref<FloatSlider> pAttenuationRadiusSlider = new FloatSlider("##AttenuationRadiusSlider", 10.0f, 0.08, 163.48f, "%.3f m", ImGuiSliderFlags_Logarithmic);

		pTable->Add(pAttenuationRadiusLabel, 0, currentRow);
		pTable->Add(pAttenuationRadiusSlider, 1, currentRow);
		currentRow++;

		pTable->Add(pUseTemperatureLabel, 0, currentRow);
		pTable->Add(pUseTemperatureCheckbox, 1, currentRow);
		currentRow++;

		pTable->Add(pTemperatureLabel, 0, currentRow);
		pTable->Add(pTemperatureSlider, 1, currentRow);
		currentRow++;

		Ref<CollapsibleSection> pLightSection = new CollapsibleSection("Light");
		pLightSection->Add(pTable);

		return pLightSection;
	}

	template<>
	Ref<IWidget> DetailsPanel::CreateComponentSection<TransformComponent>(const std::vector<entity>& selectedEntities) noexcept
	{
		Ref<Table> pTable = new Table("##PropertyTable");

		uint32 currentRow = 0u;

		{
			Ref<ComboBox> pLocationComboBox = new ComboBox("##TransformLocationComboBox");
			pLocationComboBox->AddSelectables({ "Location", "Absolute Location" });

			Ref<FloatDrag> pLocationDragFloatX = new FloatDrag("##LocationDragFloatX", 0.01f, -FLT_MAX, FLT_MAX, "%.2f");
			pLocationDragFloatX
				->Value([this, pComboBox = pLocationComboBox.Get()]() { return GetLocation(pComboBox).x; })
				->OnValueChanged([this, pComboBox = pLocationComboBox.Get()](float value) {  OnLocationChanged(value, EAxis::X, static_cast<ETransformSpace>(pComboBox->GetSelectedIndex())); })
				->SetIndicatorColor(Colors::Red);

			Ref<FloatDrag> pLocationDragFloatY = new FloatDrag("##LocationDragFloatY", 0.01f, -FLT_MAX, FLT_MAX, "%.2f");
			pLocationDragFloatY
				->Value([this, pComboBox = pLocationComboBox.Get()]() { return GetLocation(pComboBox).y; })
				->OnValueChanged([this, pComboBox = pLocationComboBox.Get()](float value) {  OnLocationChanged(value, EAxis::Y, static_cast<ETransformSpace>(pComboBox->GetSelectedIndex())); })
				->SetIndicatorColor(Colors::Green);

			Ref<FloatDrag> pLocationDragFloatZ = new FloatDrag("##LocationDragFloatZ", 0.01f, -FLT_MAX, FLT_MAX, "%.2f");
			pLocationDragFloatZ
				->Value([this, pComboBox = pLocationComboBox.Get()]() { return GetLocation(pComboBox).z; })
				->OnValueChanged([this, pComboBox = pLocationComboBox.Get()](float value) {  OnLocationChanged(value, EAxis::Z, static_cast<ETransformSpace>(pComboBox->GetSelectedIndex())); })
				->SetIndicatorColor(Colors::Blue);

			Ref<HorizontalBox> pLocationHorizontalBox = new HorizontalBox("##LocationHorizontalBox");
			pLocationHorizontalBox->Add(pLocationDragFloatX);
			pLocationHorizontalBox->Add(pLocationDragFloatY);
			pLocationHorizontalBox->Add(pLocationDragFloatZ);

			pTable->Add(pLocationComboBox, 0, currentRow);
			pTable->Add(pLocationHorizontalBox, 1, currentRow);
			currentRow++;
		}

		{
			Ref<ComboBox> pRotationComboBox = new ComboBox("##RotationComboBox");
			pRotationComboBox->AddSelectables({ "Rotation", "Absolute Rotation" });

			Ref<FloatDrag> pRotationDragFloatX = new FloatDrag("##RotationDragFloatX", 1.0f, -FLT_MAX, FLT_MAX, "%.2f\xC2\xB0");
			pRotationDragFloatX
				->Value([this, pComboBox = pRotationComboBox.Get()]() { return Math::RadToDeg360(GetRotation(pComboBox).x); })
				->OnValueChanged([this, pComboBox = pRotationComboBox.Get()](float value) { OnRotationChanged(value, EAxis::X, static_cast<ETransformSpace>(pComboBox->GetSelectedIndex())); })
				->SetIndicatorColor(Colors::Red);

			Ref<FloatDrag> pRotationDragFloatY = new FloatDrag("##RotationDragFloatY", 1.0f, -FLT_MAX, FLT_MAX, "%.2f\xC2\xB0");
			pRotationDragFloatY
				->Value([this, pComboBox = pRotationComboBox.Get()]() { return Math::RadToDeg360(GetRotation(pComboBox).y); })
				->OnValueChanged([this, pComboBox = pRotationComboBox.Get()](float value) { OnRotationChanged(value, EAxis::Y, static_cast<ETransformSpace>(pComboBox->GetSelectedIndex())); })
				->SetIndicatorColor(Colors::Green);

			Ref<FloatDrag> pRotationDragFloatZ = new FloatDrag("##RotationDragFloatZ", 1.0f, -FLT_MAX, FLT_MAX, "%.2f\xC2\xB0");
			pRotationDragFloatZ
				->Value([this, pComboBox = pRotationComboBox.Get()]() { return Math::RadToDeg360(GetRotation(pComboBox).z); })
				->OnValueChanged([this, pComboBox = pRotationComboBox.Get()](float value) { OnRotationChanged(value, EAxis::Z, static_cast<ETransformSpace>(pComboBox->GetSelectedIndex())); })
				->SetIndicatorColor(Colors::Blue);

			Ref<HorizontalBox> pRotationHorizontalBox = new HorizontalBox("##RotationHorizontalBox");
			pRotationHorizontalBox->Add(pRotationDragFloatX);
			pRotationHorizontalBox->Add(pRotationDragFloatY);
			pRotationHorizontalBox->Add(pRotationDragFloatZ);

			pTable->Add(pRotationComboBox, 0, currentRow);
			pTable->Add(pRotationHorizontalBox, 1, currentRow);
			currentRow++;
		}

		{
			Ref<ComboBox> pScaleComboBox = new ComboBox("##ScaleComboBox");
			pScaleComboBox->AddSelectables({ "Scale", "Absolute Scale" });

			Ref<FloatDrag> pScaleDragFloatX = new FloatDrag("##ScaleDragFloatX", 0.01f, 0.01, FLT_MAX, "%.2f");
			pScaleDragFloatX
				->Value([this, pComboBox = pScaleComboBox.Get()]() { return GetScale(pComboBox).x; })
				->OnValueChanged([this, pComboBox = pScaleComboBox.Get()](float value) {  OnScaleChanged(value, EAxis::X, static_cast<ETransformSpace>(pComboBox->GetSelectedIndex())); })
				->SetIndicatorColor(Colors::Red);

			Ref<FloatDrag> pScaleDragFloatY = new FloatDrag("##ScaleDragFloatY", 0.01f, 0.01, FLT_MAX, "%.2f");
			pScaleDragFloatY
				->Value([this, pComboBox = pScaleComboBox.Get()]() { return GetScale(pComboBox).y; })
				->OnValueChanged([this, pComboBox = pScaleComboBox.Get()](float value) {  OnScaleChanged(value, EAxis::Y, static_cast<ETransformSpace>(pComboBox->GetSelectedIndex())); })
				->SetIndicatorColor(Colors::Green);

			Ref<FloatDrag> pScaleDragFloatZ = new FloatDrag("##ScaleDragFloatZ", 0.01f, 0.01, FLT_MAX, "%.2f");
			pScaleDragFloatZ
				->Value([this, pComboBox = pScaleComboBox.Get()]() { return GetScale(pComboBox).z; })
				->OnValueChanged([this, pComboBox = pScaleComboBox.Get()](float value) {  OnScaleChanged(value, EAxis::Z, static_cast<ETransformSpace>(pComboBox->GetSelectedIndex())); })
				->SetIndicatorColor(Colors::Blue);

			Ref<HorizontalBox> pScaleHorizontalBox = new HorizontalBox("##ScaleHorizontalBox");
			pScaleHorizontalBox->Add(pScaleDragFloatX);
			pScaleHorizontalBox->Add(pScaleDragFloatY);
			pScaleHorizontalBox->Add(pScaleDragFloatZ);

			pTable->Add(pScaleComboBox, 0, currentRow);
			pTable->Add(pScaleHorizontalBox, 1, currentRow);
			currentRow++;
		}

		Ref<CollapsibleSection> pLightSection = new CollapsibleSection("Transform");
		pLightSection->Add(pTable);

		return pLightSection;
	}
}
