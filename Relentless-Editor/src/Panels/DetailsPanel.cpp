#include "DetailsPanel.h"
#include "../Core/Editor.h"

#include "../UI/Views/Details/EntityDetailsView.h"

namespace Relentless
{
	DetailsPanel::DetailsPanel(std::weak_ptr<Editor> aEditor) noexcept
		: IEditorPanel{ ICON_FA_LINES_LEANING " Details", ImGuiWindowFlags_NoScrollbar, aEditor}
	{
		m_pEntityDetailsView = new EntityDetailsView(aEditor);
		SetRoot(m_pEntityDetailsView);
	}

// 	DetailsPanel::~DetailsPanel() noexcept
// 	{
// 		if (m_pEditor)
// 		{
// 			if (const UniquePtr<Selection>& pSelection = m_pEditor->GetSelection())
// 				pSelection->OnSelectionChanged.Detach(this);
// 
// 			m_pEditor->OnSceneChanged.Detach(this);
// 		}
// 	}

	//Ref<IBaseWidget> DetailsPanel::CreateBaseSection() noexcept
	//{
	//	return nullptr;
	//
	//	EntityManager& entityManager = m_pEditor->GetActiveScene()->GetEntityManager();
	//	
	//	Ref<HorizontalBox> topBox = new HorizontalBox();
	//	topBox->SetMargin(FloatRect(0.0f, 5.0f, 0.0f, 0.0f));
	//
	//	Ref<HorizontalBox> topLeftBox = new HorizontalBox();
	//	topLeftBox->SetMargin(FloatRect(10.0f, 0.0f, 0.0f, 0.0f));
	//	
	//	Ref<HorizontalBox> topRightBox = new HorizontalBox();
	//	topRightBox->SetAlignmentPolicy(EAlignmentPolicy::Right);
	//	topRightBox->SetMargin(FloatRect(0.0f, 0.0f, 10.0f, 0.0f));
	//
	//	Ref<Button> pAddButton = new Button(ICON_FA_PLUS " Add", Vector2(80, 30));
	//	pAddButton->SetFont(ImGui::GetIO().Fonts->Fonts[2]);
	//
	//	Ref<Button> pLockButton = new Button(m_SelectionLocked ? ICON_FA_LOCK : ICON_FA_LOCK_OPEN, Vector2(40, 30));
	//	pLockButton->SetFont(ImGui::GetIO().Fonts->Fonts[2]);
	//	pLockButton->SetTextColor(m_SelectionLocked ? Colors::White : Colors::Gray);
	//	pLockButton->SetBackgroundColor(Colors::Transparent);
	//	pLockButton->SetBorderColor(Colors::Transparent);
	//	pLockButton->OnClicked([this, lockButton = pLockButton.Get()]()
	//		{
	//			if (lockButton->GetText() == ICON_FA_LOCK_OPEN)
	//			{
	//				lockButton->SetText(ICON_FA_LOCK);
	//				lockButton->SetTextColor(Colors::White);
	//			}
	//			else
	//			{
	//				lockButton->SetText(ICON_FA_LOCK_OPEN);
	//				lockButton->SetTextColor(Colors::Gray);
	//			}
	//
	//			m_SelectionLocked = !m_SelectionLocked;
	//		});
	//
	//	topRightBox->Add(pAddButton);
	//	topRightBox->Add(pLockButton);
	//
	//	const String text = m_SelectedEntities.size() > 1 ? std::format("{0} entities", m_SelectedEntities.size()) : entityManager.Get<NameComponent>(m_SelectedEntities[0]).Name;
	//	topLeftBox->Add(new Label(std::format("{}  {}", ICON_FA_TAG, text), ImGui::GetIO().Fonts->Fonts[2]));
	//
	//	topBox->Add(topLeftBox);
	//	topBox->Add(topRightBox);
	//
	//	Ref<SearchBar> pSearchBar = new SearchBar("Search...", true);
	//	pSearchBar->SetSizePolicy(ESizePolicy::Stretch);
	//
	//	Ref<HorizontalBox> pHBox = new HorizontalBox(false);
	//	pHBox->SetMargin(FloatRect(10.0f, 0.0f, 10.0f, 0.0f));
	//	pHBox->Add(pSearchBar);
	//
	//	Ref<VerticalBox> pSection = new VerticalBox(Vector2(0, 80.0f), true);
	//	
	//	pSection->Add(topBox);
	//	pSection->Add(pHBox);
	//
	//	return pSection;
	//}

// 	void DetailsPanel::OnDirectionalLightColorChanged(const Color& color) noexcept
// 	{
// 		EntityManager& entityManager = m_pEditor->GetActiveScene()->GetEntityManager();
// 		std::for_each(m_SelectedEntities.begin(), m_SelectedEntities.end(), [&](entity e) { entityManager.Get<DirectionalLightComponent>(e).Color = color; });
// 	}
// 
// 	void DetailsPanel::OnDirectionalLightIntensityChanged(float intensity) noexcept
// 	{
// 		EntityManager& entityManager = m_pEditor->GetActiveScene()->GetEntityManager();
// 		const float radiantIrradiance = Math::LuxToRadiantIrradiance(intensity);
// 		std::for_each(m_SelectedEntities.begin(), m_SelectedEntities.end(), [&](entity e) { entityManager.Get<DirectionalLightComponent>(e).Intensity = radiantIrradiance; });
// 	}
// 
// 	void DetailsPanel::OnDirectionalLightTemperatureChanged(float temperature) noexcept
// 	{
// 		EntityManager& entityManager = m_pEditor->GetActiveScene()->GetEntityManager();
// 		std::for_each(m_SelectedEntities.begin(), m_SelectedEntities.end(), [&](entity e) { entityManager.Get<DirectionalLightComponent>(e).Temperature = temperature; });
// 	}
// 
// 	void DetailsPanel::OnDirectionalLightUseTemperatureChanged(bool useTemperature) noexcept
// 	{
// 		EntityManager& entityManager = m_pEditor->GetActiveScene()->GetEntityManager();
// 		std::for_each(m_SelectedEntities.begin(), m_SelectedEntities.end(), [&](entity e) { entityManager.Get<DirectionalLightComponent>(e).UseTemperature = useTemperature; });
// 	}
// 
// 	Color DetailsPanel::OnDirectionalLightColorRequested() const noexcept
// 	{
// 		EntityManager& entityManager = m_pEditor->GetActiveScene()->GetEntityManager();
// 		return entityManager.Get<DirectionalLightComponent>(m_SelectedEntities.back()).Color;
// 	}
// 
// 	float DetailsPanel::OnDirectionalLightIntensityRequested() const noexcept
// 	{
// 		EntityManager& entityManager = m_pEditor->GetActiveScene()->GetEntityManager();
// 		return Math::RadiantIrradianceToLux(entityManager.Get<DirectionalLightComponent>(m_SelectedEntities.back()).Intensity);
// 	}
// 
// 	float DetailsPanel::OnDirectionalLightTemperatureRequested() const noexcept
// 	{
// 		EntityManager& entityManager = m_pEditor->GetActiveScene()->GetEntityManager();
// 		return entityManager.Get<DirectionalLightComponent>(m_SelectedEntities.back()).Temperature;
// 	}
// 
// 	bool DetailsPanel::OnDirectionalLightUseTemperatureRequested() const noexcept
// 	{
// 		EntityManager& entityManager = m_pEditor->GetActiveScene()->GetEntityManager();
// 		return entityManager.Get<DirectionalLightComponent>(m_SelectedEntities.back()).UseTemperature;
// 	}
// 
// 	void DetailsPanel::OnPointLightAttenuationRadiusChanged(float radius) noexcept
// 	{
// 		EntityManager& entityManager = m_pEditor->GetActiveScene()->GetEntityManager();
// 		std::for_each(m_SelectedEntities.begin(), m_SelectedEntities.end(), [&](entity e) { entityManager.Get<PointLightComponent>(e).AttenuationRadius = radius; });
// 	}

// 	void DetailsPanel::OnPointLightColorChanged(const Color& color) noexcept
// 	{
// 		EntityManager& entityManager = m_pEditor->GetActiveScene()->GetEntityManager();
// 		std::for_each(m_SelectedEntities.begin(), m_SelectedEntities.end(), [&](entity e) { entityManager.Get<PointLightComponent>(e).Color = color; });
// 	}
// 
// 	void DetailsPanel::OnPointLightIntensityChanged(float intensity, const char* pIntensityUnit) noexcept
// 	{
// 		EntityManager& entityManager = m_pEditor->GetActiveScene()->GetEntityManager();
// 
// 		if (strcmp(pIntensityUnit, "Lumens") == 0)
// 			std::for_each(m_SelectedEntities.begin(), m_SelectedEntities.end(), [&](entity e) { entityManager.Get<PointLightComponent>(e).Intensity = Math::LumenToRadiantIntensity(intensity); });
// 		else if (strcmp(pIntensityUnit, "Candelas") == 0)
// 			std::for_each(m_SelectedEntities.begin(), m_SelectedEntities.end(), [&](entity e) { entityManager.Get<PointLightComponent>(e).Intensity = Math::CandelaToRadiantIntensity(intensity); });
// 	}
// 
// 	void DetailsPanel::OnPointLightTemperatureChanged(float temperature) noexcept
// 	{
// 		EntityManager& entityManager = m_pEditor->GetActiveScene()->GetEntityManager();
// 		std::for_each(m_SelectedEntities.begin(), m_SelectedEntities.end(), [&](entity e) { entityManager.Get<PointLightComponent>(e).Temperature = temperature; });
// 	}
// 
// 	void DetailsPanel::OnPointLightUseTemperatureChanged(bool useTemperature) noexcept
// 	{
// 		EntityManager& entityManager = m_pEditor->GetActiveScene()->GetEntityManager();
// 		std::for_each(m_SelectedEntities.begin(), m_SelectedEntities.end(), [&](entity e) { entityManager.Get<PointLightComponent>(e).UseTemperature = useTemperature; });
// 	}
// 
// 	float DetailsPanel::OnPointLightAttenuationRadiusRequested() const noexcept
// 	{
// 		EntityManager& entityManager = m_pEditor->GetActiveScene()->GetEntityManager();
// 		return entityManager.Get<PointLightComponent>(m_SelectedEntities.back()).AttenuationRadius;
// 	}
// 
// 	Color DetailsPanel::OnPointLightColorRequested() const noexcept
// 	{
// 		EntityManager& entityManager = m_pEditor->GetActiveScene()->GetEntityManager();
// 		return entityManager.Get<PointLightComponent>(m_SelectedEntities.back()).Color;
// 	}
// 
// 	float DetailsPanel::OnPointLightIntensityRequested(const char* pIntensityUnit) const noexcept
// 	{
// 		EntityManager& entityManager = m_pEditor->GetActiveScene()->GetEntityManager();
// 		
// 		if (strcmp(pIntensityUnit, "Lumens") == 0)
// 			return Math::RadiantIntensityToLumen(entityManager.Get<PointLightComponent>(m_SelectedEntities.back()).Intensity);
// 		else
// 			return Math::RadiantIntensityToCandela(entityManager.Get<PointLightComponent>(m_SelectedEntities.back()).Intensity);
// 	}
// 
// 	float DetailsPanel::OnPointLightTemperatureRequested() const noexcept
// 	{
// 		EntityManager& entityManager = m_pEditor->GetActiveScene()->GetEntityManager();
// 		return entityManager.Get<PointLightComponent>(m_SelectedEntities.back()).Temperature;
// 	}
// 
// 	bool DetailsPanel::OnPointLightUseTemperatureRequested() const noexcept
// 	{
// 		EntityManager& entityManager = m_pEditor->GetActiveScene()->GetEntityManager();
// 		return entityManager.Get<PointLightComponent>(m_SelectedEntities.back()).UseTemperature;
// 	}
// 
// 	void DetailsPanel::OnSpotLightAttenuationRadiusChanged(float radius) noexcept
// 	{
// 		EntityManager& entityManager = m_pEditor->GetActiveScene()->GetEntityManager();
// 		std::for_each(m_SelectedEntities.begin(), m_SelectedEntities.end(), [&](entity e) { entityManager.Get<SpotLightComponent>(e).AttenuationRadius = radius; });
// 	}
// 
// 	void DetailsPanel::OnSpotLightColorChanged(const Color& color) noexcept
// 	{
// 		EntityManager& entityManager = m_pEditor->GetActiveScene()->GetEntityManager();
// 		std::for_each(m_SelectedEntities.begin(), m_SelectedEntities.end(), [&](entity e) { entityManager.Get<SpotLightComponent>(e).Color = color; });
// 	}
// 
// 	void DetailsPanel::OnSpotLightInnerConeAngleChanged(float angleDeg) noexcept
// 	{
// 		EntityManager& entityManager = m_pEditor->GetActiveScene()->GetEntityManager();
// 		std::for_each(m_SelectedEntities.begin(), m_SelectedEntities.end(), [&](entity e) 
// 			{ 
// 				entityManager.Get<SpotLightComponent>(e).InnerConeAngle = Math::DegToRad(angleDeg); 
// 				entityManager.Get<SpotLightComponent>(e).OuterConeAngle = Math::Max(entityManager.Get<SpotLightComponent>(e).InnerConeAngle, entityManager.Get<SpotLightComponent>(e).OuterConeAngle);
// 			});
// 	}
// 
// 	void DetailsPanel::OnSpotLightIntensityChanged(float intensity, const char* pIntensityUnit) noexcept
// 	{
// 		EntityManager& entityManager = m_pEditor->GetActiveScene()->GetEntityManager();
// 
// 		if (strcmp(pIntensityUnit, "Lumens") == 0)
// 			std::for_each(m_SelectedEntities.begin(), m_SelectedEntities.end(), [&](entity e) { entityManager.Get<SpotLightComponent>(e).Intensity = Math::LumenToRadiantIntensity(intensity, Math::SpotLightHalfAngleToSolidAngle(entityManager.Get<SpotLightComponent>(e).OuterConeAngle)); });
// 		else if (strcmp(pIntensityUnit, "Candelas") == 0)
// 			std::for_each(m_SelectedEntities.begin(), m_SelectedEntities.end(), [&](entity e) { entityManager.Get<SpotLightComponent>(e).Intensity = Math::CandelaToRadiantIntensity(intensity); });
// 	}
// 
// 	void DetailsPanel::OnSpotLightOuterConeAngleChanged(float angleDeg) noexcept
// 	{
// 		EntityManager& entityManager = m_pEditor->GetActiveScene()->GetEntityManager();
// 		std::for_each(m_SelectedEntities.begin(), m_SelectedEntities.end(), [&](entity e)
// 			{
// 				entityManager.Get<SpotLightComponent>(e).OuterConeAngle = Math::DegToRad(angleDeg);
// 				entityManager.Get<SpotLightComponent>(e).InnerConeAngle = Math::Min(entityManager.Get<SpotLightComponent>(e).InnerConeAngle, entityManager.Get<SpotLightComponent>(e).OuterConeAngle);
// 			});
// 	}
// 
// 	void DetailsPanel::OnSpotLightTemperatureChanged(float temperature) noexcept
// 	{
// 		EntityManager& entityManager = m_pEditor->GetActiveScene()->GetEntityManager();
// 		std::for_each(m_SelectedEntities.begin(), m_SelectedEntities.end(), [&](entity e) { entityManager.Get<SpotLightComponent>(e).Temperature = temperature; });
// 	}
// 
// 	void DetailsPanel::OnSpotLightUseTemperatureChanged(bool useTemperature) noexcept
// 	{
// 		EntityManager& entityManager = m_pEditor->GetActiveScene()->GetEntityManager();
// 		std::for_each(m_SelectedEntities.begin(), m_SelectedEntities.end(), [&](entity e) { entityManager.Get<SpotLightComponent>(e).UseTemperature = useTemperature; });
// 	}
// 
// 	float DetailsPanel::OnSpotLightAttenuationRadiusRequested() const noexcept
// 	{
// 		EntityManager& entityManager = m_pEditor->GetActiveScene()->GetEntityManager();
// 		return entityManager.Get<SpotLightComponent>(m_SelectedEntities.back()).AttenuationRadius;
// 	}
// 
// 	Color DetailsPanel::OnSpotLightColorRequested() const noexcept
// 	{
// 		EntityManager& entityManager = m_pEditor->GetActiveScene()->GetEntityManager();
// 		return entityManager.Get<SpotLightComponent>(m_SelectedEntities.back()).Color;
// 	}
// 
// 	float DetailsPanel::OnSpotLightInnerConeAngleRequested() const noexcept
// 	{
// 		EntityManager& entityManager = m_pEditor->GetActiveScene()->GetEntityManager();
// 		return Math::RadToDeg(entityManager.Get<SpotLightComponent>(m_SelectedEntities.back()).InnerConeAngle);
// 	}
// 
// 	float DetailsPanel::OnSpotLightIntensityRequested(const char* pIntensityUnit) const noexcept
// 	{
// 		EntityManager& entityManager = m_pEditor->GetActiveScene()->GetEntityManager();
// 
// 		if (strcmp(pIntensityUnit, "Lumens") == 0)
// 			return Math::RadiantIntensityToLumen(entityManager.Get<SpotLightComponent>(m_SelectedEntities.back()).Intensity, Math::SpotLightHalfAngleToSolidAngle(entityManager.Get<SpotLightComponent>(m_SelectedEntities.back()).OuterConeAngle));
// 		else
// 			return Math::RadiantIntensityToCandela(entityManager.Get<SpotLightComponent>(m_SelectedEntities.back()).Intensity);
// 	}
// 
// 	float DetailsPanel::OnSpotLightOuterConeAngleRequested() const noexcept
// 	{
// 		EntityManager& entityManager = m_pEditor->GetActiveScene()->GetEntityManager();
// 		return Math::RadToDeg(entityManager.Get<SpotLightComponent>(m_SelectedEntities.back()).OuterConeAngle);
// 	}
// 
// 	float DetailsPanel::OnSpotLightTemperatureRequested() const noexcept
// 	{
// 		EntityManager& entityManager = m_pEditor->GetActiveScene()->GetEntityManager();
// 		return entityManager.Get<SpotLightComponent>(m_SelectedEntities.back()).Temperature;
// 	}
// 
// 	bool DetailsPanel::OnSpotLightUseTemperatureRequested() const noexcept
// 	{
// 		EntityManager& entityManager = m_pEditor->GetActiveScene()->GetEntityManager();
// 		return entityManager.Get<SpotLightComponent>(m_SelectedEntities.back()).UseTemperature;
// 	}

	//void DetailsPanel::OnLocationChanged(float value, EAxis axis, ETransformSpace space) noexcept
	//{
	//	Scene* pScene = m_pEditor->GetActiveScene();
	//
	//	std::for_each(m_SelectedEntities.begin(), m_SelectedEntities.end(), [&](entity e)
	//		{
	//			Vector3 location = space == ETransformSpace::Absolute ? pScene->GetWorldLocation(e) : pScene->GetLocalLocation(e);
	//			switch (axis)
	//			{
	//			case EAxis::X: location.x = value; break;
	//			case EAxis::Y: location.y = value; break;
	//			case EAxis::Z: location.z = value; break;
	//			}
	//
	//			switch (space)
	//			{
	//			case ETransformSpace::Absolute: pScene->SetWorldLocation(e, location); break;
	//			case ETransformSpace::Relative: pScene->SetLocalLocation(e, location); break;
	//			}
	//		});
	//}

// 	void DetailsPanel::OnLocationChanged(const Vector3& value, ComboBox* pTransformSpaceComboBox) noexcept
// 	{
// 		Scene* pScene = m_pEditor->GetActiveScene();
// 		const ETransformSpace space = static_cast<ETransformSpace>(pTransformSpaceComboBox->GetSelectedIndex());
// 
// 		std::for_each(m_SelectedEntities.begin(), m_SelectedEntities.end(), [&](entity e)
// 			{
// 				switch (space)
// 				{
// 				case ETransformSpace::Absolute: pScene->SetWorldLocation(e, value); break;
// 				case ETransformSpace::Relative: pScene->SetLocalLocation(e, value); break;
// 				}
// 			});
// 	}

	//void DetailsPanel::OnRotationChanged(float valueDeg, EAxis axis, ETransformSpace space) noexcept
	//{
	//	Scene* pScene = m_pEditor->GetActiveScene();
	//
	//	std::for_each(m_SelectedEntities.begin(), m_SelectedEntities.end(), [&](entity e)
	//		{
	//			const Quaternion currentRotation = space == ETransformSpace::Absolute ? pScene->GetWorldRotation(e) : pScene->GetLocalRotation(e);
	//
	//			const Vector3 eulerDeg = Math::RadToDeg(currentRotation.ToEuler());
	//
	//			Vector3 axisVec = Vector3::Zero;
	//			float currentAngle = 0.0f;
	//			switch (axis)
	//			{
	//			case EAxis::X: axisVec = Vector3::Right;	currentAngle = eulerDeg.x; break;
	//			case EAxis::Y: axisVec = Vector3::Up;		currentAngle = eulerDeg.y; break;
	//			case EAxis::Z: axisVec = Vector3::Forward;	currentAngle = eulerDeg.z; break;
	//			}
	//
	//			const float deltaAngleDeg = valueDeg - currentAngle;
	//			const float deltaAngleRad = Math::DegToRad(deltaAngleDeg);
	//
	//			const Quaternion deltaRot = Quaternion::CreateFromAxisAngle(axisVec, deltaAngleRad);
	//
	//			switch (space)
	//			{
	//				case ETransformSpace::Absolute: pScene->AddWorldRotation(e, Math::RadToDeg(deltaRot.ToEuler())); break;
	//				case ETransformSpace::Relative: pScene->AddLocalRotation(e, Math::RadToDeg(deltaRot.ToEuler())); break;
	//			}
	//		});
	//}

// 	void DetailsPanel::OnRotationChanged(const Vector3& valueDeg, ComboBox* pTransformSpaceComboBox) noexcept
// 	{
// 		Scene* pScene = m_pEditor->GetActiveScene();
// 		const ETransformSpace space = static_cast<ETransformSpace>(pTransformSpaceComboBox->GetSelectedIndex());
// 
// 		std::for_each(m_SelectedEntities.begin(), m_SelectedEntities.end(), [&](entity e)
// 			{
// 				const Quaternion currentRotation = space == ETransformSpace::Absolute ? pScene->GetWorldRotation(e) : pScene->GetLocalRotation(e);
// 
// 				const Vector3 currentEulerDeg = Math::RadToDeg(currentRotation.ToEuler());
// 
// 				const Vector3 deltaEulerDeg = Vector3{
// 					valueDeg.x - currentEulerDeg.x,
// 					valueDeg.y - currentEulerDeg.y,
// 					valueDeg.z - currentEulerDeg.z
// 				};
// 
// 				// Convert deg deltas to rad and build quaternions:
// 				Quaternion deltaQ = Quaternion::CreateFromAxisAngle(Vector3::Right, Math::DegToRad(deltaEulerDeg.x)) *
// 					Quaternion::CreateFromAxisAngle(Vector3::Up, Math::DegToRad(deltaEulerDeg.y)) *
// 					Quaternion::CreateFromAxisAngle(Vector3::Forward, Math::DegToRad(deltaEulerDeg.z));
// 
// 				switch (space)
// 				{
// 				case ETransformSpace::Absolute: pScene->AddWorldRotation(e, Math::RadToDeg(deltaQ.ToEuler())); break;
// 				case ETransformSpace::Relative: pScene->AddLocalRotation(e, Math::RadToDeg(deltaQ.ToEuler())); break;
// 				}
// 			});
// 	}

	//void DetailsPanel::OnScaleChanged(float value, EAxis axis, ETransformSpace space) noexcept
	//{
	//	Scene* pScene = m_pEditor->GetActiveScene();
	//
	//	std::for_each(m_SelectedEntities.begin(), m_SelectedEntities.end(), [&](entity e)
	//		{
	//			Vector3 scale = space == ETransformSpace::Absolute ? pScene->GetWorldScale(e) : pScene->GetLocalScale(e);
	//			switch (axis)
	//			{
	//			case EAxis::X: scale.x = value; break;
	//			case EAxis::Y: scale.y = value; break;
	//			case EAxis::Z: scale.z = value; break;
	//			}
	//
	//			switch (space)
	//			{
	//			case ETransformSpace::Absolute: pScene->SetWorldScale(e, scale); break;
	//			case ETransformSpace::Relative: pScene->SetLocalScale(e, scale); break;
	//			}
	//		});
	//}

// 	void DetailsPanel::OnScaleChanged(const Vector3& value, ComboBox* pTransformSpaceComboBox) noexcept
// 	{
// 		Scene* pScene = m_pEditor->GetActiveScene();
// 		const ETransformSpace space = static_cast<ETransformSpace>(pTransformSpaceComboBox->GetSelectedIndex());
// 
// 		std::for_each(m_SelectedEntities.begin(), m_SelectedEntities.end(), [&](entity e)
// 			{
// 				switch (space)
// 				{
// 				case ETransformSpace::Absolute: pScene->SetWorldScale(e, value); break;
// 				case ETransformSpace::Relative: pScene->SetLocalScale(e, value); break;
// 				}
// 			});
// 	}

// 	void DetailsPanel::OnEntityDestroyed(entity destroyedEntity) noexcept
// 	{
// 		auto it = std::find(m_SelectedEntities.begin(), m_SelectedEntities.end(), destroyedEntity);
// 		if (it == m_SelectedEntities.end())
// 			return;
// 		
// 		std::swap(*it, m_SelectedEntities.back());
// 		m_SelectedEntities.pop_back();
// 
// 		if (!m_SelectedEntities.empty())
// 			CreateFromSelection();
// 		else
// 			CreateEmpty();
// 	}

// 	void DetailsPanel::OnPreSceneChanged(Scene* pScene) noexcept
// 	{
// 		pScene->OnEntityDestroyed.Detach(this);
// 	}

// 	void DetailsPanel::OnRender() noexcept
// 	{
// 	}

// 	void DetailsPanel::OnSceneChanged(Scene* pScene) noexcept
// 	{
// 		pScene->OnEntityDestroyed.Connect(this, &DetailsPanel::OnEntityDestroyed);
// 		CreateEmpty();
// 	}

// 	void DetailsPanel::OnSelectionChanged(entity, ESelectionState) noexcept
// 	{
// 		//if (m_SelectionLocked)
// 		//	return;
// 		//
// 		//m_SelectedEntities = m_pEditor->GetSelection()->GetSelectedEntities();
// 		//
// 		//if (m_SelectedEntities.empty())
// 		//	CreateEmpty();
// 		//else
// 		//	CreateFromSelection();
// 	}

// 	template<>
// 	Ref<IBaseWidget> DetailsPanel::CreateComponentSection<DirectionalLightComponent>() noexcept
// 	{
// 		Ref<CollapsibleSection> pLightSection = new CollapsibleSection(std::format("{}  Light", ICON_FA_LIGHTBULB));
// 		Table* pTable = pLightSection->Add(new Table());
// 
// 		uint32 currentRow = 0;
// 
// 		AddRow(pTable, currentRow++, "Type", new ComboBox())
// 			->AddSelectables({ "Directional", "Point", "Spot" })
// 			->SetInitiallySelectedItem("Directional")
// 			->OnSelectionChanged(this, &DetailsPanel::OnLightTypeSelectionChanged);
// 
// 		AddRow(pTable, currentRow++, "Intensity", new FloatSlider(0.0f, 120'000.0f, "%.3f lux", ImGuiSliderFlags_Logarithmic))
// 			->Value(this, &DetailsPanel::OnDirectionalLightIntensityRequested)
// 			->OnValueChanged(this, &DetailsPanel::OnDirectionalLightIntensityChanged);
// 
// 		AddRow(pTable, currentRow++, "Light Color", new ColorPicker(Vector2(180.0f, 30.0f)))
// 			->Value(this, &DetailsPanel::OnDirectionalLightColorRequested)
// 			->OnValueChanged(this, &DetailsPanel::OnDirectionalLightColorChanged);
// 
// 		AddRow(pTable, currentRow++, "Use Temperature", new CheckBox())
// 			->Value(this, &DetailsPanel::OnDirectionalLightUseTemperatureRequested)
// 			->OnCheckStateChanged([this, pTable](bool state)
// 				{ 
// 					//if (FloatSlider* pTempSlider = pTable->FindByID<FloatSlider>("##TemperatureSlider"))
// 					//	pTempSlider->SetIsEnabled(state);
// 
// 					OnDirectionalLightUseTemperatureChanged(state);
// 				});
// 		
// 		AddRow(pTable, currentRow++, "Temperature", new FloatSlider(1'700.0f, 12'000.0f, "%.3f K"))
// 			->Value(this, &DetailsPanel::OnDirectionalLightTemperatureRequested)
// 			->OnValueChanged(this, &DetailsPanel::OnDirectionalLightTemperatureChanged)
// 			->SetIsEnabled(OnDirectionalLightUseTemperatureRequested());
// 
// 		return pLightSection;
// 	}
// 
// 	template<>
// 	Ref<IBaseWidget> DetailsPanel::CreateComponentSection<PointLightComponent>() noexcept
// 	{
// 		uint32 currentRow = 0;
// 		Ref<Table> pTable = new Table();
// 
// 		pTable->Add(new Label("Type"), 0, currentRow);
// 		pTable->Add(new ComboBox(), 1, currentRow)
// 			->AddSelectables({ "Directional", "Point", "Spot" })
// 			->SetInitiallySelectedItem("Point")
// 			->OnSelectionChanged(this, &DetailsPanel::OnLightTypeSelectionChanged);
// 		currentRow++;
// 
// 		Ref<FloatSlider> pIntensityFloatSlider = new FloatSlider(0.0f, 160.0f, "%.3f cd");
// 
// 		pTable->Add(new Label("Intensity Units"), 0, currentRow);
// 		ComboBox* pIntensityUnitsComboBox = pTable->Add(new ComboBox(), 1, currentRow)
// 			->AddSelectables({ "Candelas", "Lumens" })
// 			->SetInitiallySelectedItem("Candelas")
// 			->OnSelectionChanged([pIntensityFloatSlider](const char* pNewIntensityUnit)
// 				{
// 					if (strcmp(pNewIntensityUnit, "Candelas") == 0)
// 					{
// 						pIntensityFloatSlider->SetFormat("%.3f cd");
// 						pIntensityFloatSlider->SetMinValue(0.0f);
// 						pIntensityFloatSlider->SetMaxValue(160.0f);
// 					}
// 					else
// 					{
// 						pIntensityFloatSlider->SetFormat("%.3f lm");
// 						pIntensityFloatSlider->SetMinValue(0.0f);
// 						pIntensityFloatSlider->SetMaxValue(Math::RadiantIntensityToLumen(Math::CandelaToRadiantIntensity(160.0f)));
// 					}
// 				});
// 		currentRow++;
// 
// 		pTable->Add(new Label("Intensity"), 0, currentRow);
// 		 pTable->Add(pIntensityFloatSlider, 1, currentRow)
// 			->Value([this, pIntensityUnitsComboBox]() { return OnPointLightIntensityRequested(pIntensityUnitsComboBox->GetSelectedItem()); })
// 			->OnValueChanged([this, pIntensityUnitsComboBox](float intensity){ OnPointLightIntensityChanged(intensity, pIntensityUnitsComboBox->GetSelectedItem()); });
// 		currentRow++;
// 
// 		pTable->Add(new Label("Light Color"), 0, currentRow);
// 		pTable->Add(new ColorPicker(Vector2(180.0f, 30.0f)), 1, currentRow)
// 			->Value(this, &DetailsPanel::OnPointLightColorRequested)
// 			->OnValueChanged(this, &DetailsPanel::OnPointLightColorChanged);
// 		currentRow++;
// 
// 		pTable->Add(new Label("Attenuation Radius"), 0, currentRow);
// 		pTable->Add(new FloatSlider(0.08, 163.48f, "%.3f m", ImGuiSliderFlags_Logarithmic), 1, currentRow)
// 			->Value(this, &DetailsPanel::OnPointLightAttenuationRadiusRequested)
// 			->OnValueChanged(this, &DetailsPanel::OnPointLightAttenuationRadiusChanged);
// 		currentRow++;
// 
// 		Ref<FloatSlider> pTemperatureSlider = new FloatSlider(1'700.0f, 12'000.0f, "%.3f K");
// 		pTemperatureSlider
// 			->Value(this, &DetailsPanel::OnPointLightTemperatureRequested)
// 			->OnValueChanged(this, &DetailsPanel::OnPointLightTemperatureChanged)
// 			->SetIsEnabled(OnPointLightUseTemperatureRequested());
// 
// 		pTable->Add(new Label("Use Temperature"), 0, currentRow);
// 		pTable->Add(new CheckBox(), 1, currentRow)
// 			->Value(this, &DetailsPanel::OnPointLightUseTemperatureRequested)
// 			->OnCheckStateChanged([this, tempSlider = pTemperatureSlider.Get()](bool state)
// 				{
// 					if (tempSlider)
// 						tempSlider->SetIsEnabled(state);
// 
// 					OnPointLightUseTemperatureChanged(state);
// 				});
// 		currentRow++;
// 
// 		pTable->Add(new Label("Temperature"), 0, currentRow);
// 		pTable->Add(pTemperatureSlider, 1, currentRow);
// 		currentRow++;
// 		
// 		Ref<CollapsibleSection> pLightSection = new CollapsibleSection(std::format("{}  Light", ICON_FA_LIGHTBULB));
// 		pLightSection->Add(pTable);
// 
// 		return pLightSection;
// 	}

// 	template<>
// 	Ref<IBaseWidget> DetailsPanel::CreateComponentSection<SpotLightComponent>() noexcept
// 	{
// 		uint32 currentRow = 0;
// 		Ref<Table> pTable = new Table();
// 
// 		pTable->Add(new Label("Type"), 0, currentRow);
// 		pTable->Add(new ComboBox(), 1, currentRow)
// 			->AddSelectables({ "Directional", "Point", "Spot" })
// 			->SetInitiallySelectedItem("Spot")
// 			->OnSelectionChanged(this, &DetailsPanel::OnLightTypeSelectionChanged);
// 		currentRow++;
// 
// 		Ref<FloatSlider> pIntensityFloatSlider = new FloatSlider(0.0f, 160.0f, "%.3f cd");
// 
// 		pTable->Add(new Label("Intensity Units"), 0, currentRow);
// 		ComboBox* pIntensityUnitsComboBox = pTable->Add(new ComboBox(), 1, currentRow)
// 			->AddSelectables({ "Candelas", "Lumens" })
// 			->SetInitiallySelectedItem("Candelas")
// 			->OnSelectionChanged([pIntensityFloatSlider](const char* pNewIntensityUnit)
// 				{
// 					if (strcmp(pNewIntensityUnit, "Candelas") == 0)
// 					{
// 						pIntensityFloatSlider->SetFormat("%.3f cd");
// 						pIntensityFloatSlider->SetMinValue(0.0f);
// 						pIntensityFloatSlider->SetMaxValue(160.0f);
// 					}
// 					else
// 					{
// 						pIntensityFloatSlider->SetFormat("%.3f lm");
// 						pIntensityFloatSlider->SetMinValue(0.0f);
// 						pIntensityFloatSlider->SetMaxValue(Math::RadiantIntensityToLumen(Math::CandelaToRadiantIntensity(160.0f)));
// 					}
// 				});
// 		currentRow++;
// 
// 		pTable->Add(new Label("Intensity"), 0, currentRow);
// 		pTable->Add(pIntensityFloatSlider, 1, currentRow)
// 			->Value([this, pIntensityUnitsComboBox]() { return OnSpotLightIntensityRequested(pIntensityUnitsComboBox->GetSelectedItem()); })
// 			->OnValueChanged([this, pIntensityUnitsComboBox](float intensity) { OnSpotLightIntensityChanged(intensity, pIntensityUnitsComboBox->GetSelectedItem()); });
// 		currentRow++;
// 
// 		pTable->Add(new Label("Light Color"), 0, currentRow);
// 		pTable->Add(new ColorPicker(Vector2(180.0f, 30.0f)), 1, currentRow)
// 			->Value(this, &DetailsPanel::OnSpotLightColorRequested)
// 			->OnValueChanged(this, &DetailsPanel::OnSpotLightColorChanged);
// 		currentRow++;
// 
// 		pTable->Add(new Label("Attenuation Radius"), 0, currentRow);
// 		pTable->Add(new FloatSlider(0.08, 163.48f, "%.3f m", ImGuiSliderFlags_Logarithmic), 1, currentRow)
// 			->Value(this, &DetailsPanel::OnSpotLightAttenuationRadiusRequested)
// 			->OnValueChanged(this, &DetailsPanel::OnSpotLightAttenuationRadiusChanged);
// 		currentRow++;
// 
// 		pTable->Add(new Label("Inner Cone Angle"), 0, currentRow);
// 		pTable->Add(new FloatSlider(0.0f, 80.0f, "%.3f\xC2\xB0"), 1, currentRow)
// 			->Value(this, &DetailsPanel::OnSpotLightInnerConeAngleRequested)
// 			->OnValueChanged(this, &DetailsPanel::OnSpotLightInnerConeAngleChanged);
// 		currentRow++;
// 
// 		pTable->Add(new Label("Outer Cone Angle"), 0, currentRow);
// 		pTable->Add(new FloatSlider(0.0f, 80.0f, "%.3f\xC2\xB0"), 1, currentRow)
// 			->Value(this, &DetailsPanel::OnSpotLightOuterConeAngleRequested)
// 			->OnValueChanged(this, &DetailsPanel::OnSpotLightOuterConeAngleChanged);
// 		currentRow++;
// 
// 		Ref<FloatSlider> pTemperatureSlider = new FloatSlider(1'700.0f, 12'000.0f, "%.3f K");
// 		pTemperatureSlider
// 			->Value(this, &DetailsPanel::OnSpotLightTemperatureRequested)
// 			->OnValueChanged(this, &DetailsPanel::OnSpotLightTemperatureChanged)
// 			->SetIsEnabled(OnSpotLightUseTemperatureRequested());
// 
// 		pTable->Add(new Label("Use Temperature"), 0, currentRow);
// 		pTable->Add(new CheckBox(), 1, currentRow)
// 			->Value(this, &DetailsPanel::OnSpotLightUseTemperatureRequested)
// 			->OnCheckStateChanged([this, tempSlider = pTemperatureSlider.Get()](bool state)
// 				{
// 					if (tempSlider)
// 						tempSlider->SetIsEnabled(state);
// 
// 					OnSpotLightUseTemperatureChanged(state);
// 				});
// 		currentRow++;
// 
// 		pTable->Add(new Label("Temperature"), 0, currentRow);
// 		pTable->Add(pTemperatureSlider, 1, currentRow);
// 		currentRow++;
// 
// 		Ref<CollapsibleSection> pLightSection = new CollapsibleSection(std::format("{}  Light", ICON_FA_LIGHTBULB));
// 		pLightSection->Add(pTable);
// 
// 		return pLightSection;
// 	}

// 	template<>
// 	Ref<IBaseWidget> DetailsPanel::CreateComponentSection<TransformComponent>() noexcept
// 	{
// 		using namespace std::placeholders;
// 
// 		Ref<Table> pTable = new Table();
// 
// 		uint32 currentRow = 0u;
// 
// 		{
// 			Ref<ComboBox> pLocationComboBox = new ComboBox();
// 			pLocationComboBox->AddSelectables({ "Location", "Absolute Location" });
// 			pTable->Add(pLocationComboBox, 0, currentRow);
// 
// 			Float3Drag* pLocationDrag3 = pTable->Add(new Float3Drag(0.01f, -FLT_MAX, FLT_MAX, "%.2f"), 1, currentRow);
// 			pLocationDrag3
// 				->Value(std::bind(&DetailsPanel::GetLocation, this, pLocationComboBox.Get()))
// 				->OnValueChanged(std::bind(&DetailsPanel::OnLocationChanged, this, _1, pLocationComboBox.Get()))
// 				->SetIndicatorColor(0, Colors::OffRed)
// 				->SetIndicatorColor(1, Colors::OffGreen)
// 				->SetIndicatorColor(2, Colors::OffBlue)
// 				->SetSizePolicy(ESizePolicy::Stretch);
// 
// 			currentRow++;
// 		}
// 
// 		{
// 			Ref<ComboBox> pRotationComboBox = new ComboBox();
// 			pTable->Add(pRotationComboBox, 0, currentRow);
// 			pRotationComboBox->AddSelectables({ "Rotation", "Absolute Rotation" });
// 
// 			Float3Drag* pRotationDrag3 = pTable->Add(new Float3Drag(1.0f, -FLT_MAX, FLT_MAX, "%.2f\xC2\xB0"), 1, currentRow);
// 			pRotationDrag3
// 				->Value([this, pComboBox = pRotationComboBox.Get()]() { return Math::RadToDeg360(GetRotation(pComboBox)); })
// 				->OnValueChanged(std::bind(&DetailsPanel::OnRotationChanged, this, _1, pRotationComboBox.Get()))
// 				->SetIndicatorColor(0, Colors::OffRed)
// 				->SetIndicatorColor(1, Colors::OffGreen)
// 				->SetIndicatorColor(2, Colors::OffBlue)
// 				->SetSizePolicy(ESizePolicy::Stretch);
// 
// 			currentRow++;
// 		}
// 
// 		{
// 			Ref<ComboBox> pScaleComboBox = new ComboBox();
// 			pScaleComboBox->AddSelectables({ "Scale", "Absolute Scale" });
// 			pTable->Add(pScaleComboBox, 0, currentRow);
// 
// 			Float3Drag* pScaleDrag3 = pTable->Add(new Float3Drag(0.01f, 0.01f, FLT_MAX, "%.2f"), 1, currentRow);
// 			pScaleDrag3
// 				->Value(std::bind(&DetailsPanel::GetScale, this, pScaleComboBox.Get()))
// 				->OnValueChanged(std::bind(&DetailsPanel::OnScaleChanged, this, _1, pScaleComboBox.Get()))
// 				->SetIndicatorColor(0, Colors::OffRed)
// 				->SetIndicatorColor(1, Colors::OffGreen)
// 				->SetIndicatorColor(2, Colors::OffBlue)
// 				->SetSizePolicy(ESizePolicy::Stretch);
// 
// 			currentRow++;
// 		}
// 
// 		Ref<CollapsibleSection> pLightSection = new CollapsibleSection(ICON_FA_ROTATE "  Transform");
// 		pLightSection->Add(pTable);
// 
// 		return pLightSection;
// 	}
}
