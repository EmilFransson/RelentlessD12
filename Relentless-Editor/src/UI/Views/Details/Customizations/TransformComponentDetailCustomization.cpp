#include "TransformComponentDetailCustomization.h"

#include "Core/Editor.h"

#include "UI/Widgets/Button.h"
#include "UI/Views/Details/LayoutBuilders/EntityDetailLayoutBuilder.h"
#include "UI/Views/Details/TableRows/EntityDetailRow.h"
#include "UI/Widgets/FloatDrag.h"
#include "UI/Widgets/FloatEntryBox.h"
#include "UI/Widgets/ITableRow.h"

namespace Relentless
{
	TransformComponentDetailCustomization::TransformComponentDetailCustomization() noexcept
	{
		Editor::OnShutDown.Connect(this, &TransformComponentDetailCustomization::OnEditorShutdown);
		Editor::OnEntityTransformed.Connect(this, &TransformComponentDetailCustomization::OnEntityTransformed);
	}

	TransformComponentDetailCustomization::~TransformComponentDetailCustomization() noexcept
	{
		if (m_ShouldDetachFromEditorCallbacks)
		{
			Editor::OnEntityTransformed.Detach(this);
			Editor::OnShutDown.Detach(this);
		}
	}

	void TransformComponentDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& aDetailLayoutBuilder) noexcept
	{
		EntityDetailLayoutBuilder& builder = static_cast<EntityDetailLayoutBuilder&>(aDetailLayoutBuilder);
		IDetailCategoryBuilder& categoryBuilder = builder.EditCategory("Transform");

		DetailNode& locationNode = categoryBuilder.AddProperty("Location");
		locationNode.OnRequestRow(this, &TransformComponentDetailCustomization::OnRequestLocationRow);

		DetailNode& rotationNode = categoryBuilder.AddProperty("Rotation");
		rotationNode.OnRequestRow(this, &TransformComponentDetailCustomization::OnRequestRotationRow);
		
		DetailNode& scaleNode = categoryBuilder.AddProperty("Scale");
		scaleNode.OnRequestRow(this, &TransformComponentDetailCustomization::OnRequestScaleRow);

		m_pBuilder = &builder;
	}

	Vector3 TransformComponentDetailCustomization::GetLocation() noexcept
	{
		Scene& scene = m_pBuilder->GetScene();
		EntityManager& entityManager = scene.GetEntityManager();
		auto& tc = entityManager.Get<TransformComponent>(GetPrimaryEntity());

		switch (m_LocationSpace)
		{
			case ETransformSpace::Relative: return tc.GetLocalLocation();
			case ETransformSpace::Absolute: return tc.GetWorldLocation();
			default:
				RLS_ASSERT(false, "Unreachable.");
				return Vector3::Zero;
		}
	}

	const char* TransformComponentDetailCustomization::GetLocationComponentFormat(EVectorComponent aComponent) noexcept
	{
		return IsLocationIdenticalForInspected(aComponent) ? "%.2f" : "Mixed";
	}

	Vector3 TransformComponentDetailCustomization::GetRotation() noexcept
	{
		Scene& scene = m_pBuilder->GetScene();
		EntityManager& entityManager = scene.GetEntityManager();
		auto& tc = entityManager.Get<TransformComponent>(GetPrimaryEntity());

		switch (m_RotationSpace)
		{
			case ETransformSpace::Relative: return Math::RadToDeg360(tc.GetLocalRotation().ToEuler());
			case ETransformSpace::Absolute: return Math::RadToDeg360(tc.GetWorldRotation().ToEuler());
			default:
				RLS_ASSERT(false, "Unreachable.");
				return Vector3::Zero;
		}
	}

	const char* TransformComponentDetailCustomization::GetRotationComponentFormat(EVectorComponent aComponent) noexcept
	{
		return IsRotationIdenticalForInspected(aComponent) ? "%.2f\xC2\xB0" : "Mixed";
	}

	Vector3 TransformComponentDetailCustomization::GetScale() noexcept
	{
		Scene& scene = m_pBuilder->GetScene();
		EntityManager& entityManager = scene.GetEntityManager();
		auto& tc = entityManager.Get<TransformComponent>(GetPrimaryEntity());

		switch (m_ScaleSpace)
		{
			case ETransformSpace::Relative: return tc.GetLocalScale();
			case ETransformSpace::Absolute: return tc.GetWorldScale();
			default:
				RLS_ASSERT(false, "Unreachable.");
				return Vector3::Zero;
		}
	}

	const char* TransformComponentDetailCustomization::GetScaleComponentFormat(EVectorComponent aComponent) noexcept
	{
		return IsScaleIdenticalForInspected(aComponent) ? "%.2f" : "Mixed";
	}

	bool TransformComponentDetailCustomization::IsLocationDefaultForInspected() const noexcept
	{
		Scene& scene = m_pBuilder->GetScene();
		EntityManager& entityManager = scene.GetEntityManager();

		return std::ranges::all_of(GetInspectedEntities(), [this, &entityManager](entity aEntity)
			{
				auto& tc = entityManager.Get<TransformComponent>(aEntity);

				const Vector3 location = m_LocationSpace == ETransformSpace::Absolute ? tc.GetWorldLocation() : tc.GetLocalLocation();
				return Math::AreValuesClose(location.x, 0.0f) && Math::AreValuesClose(location.y, 0.0f) && Math::AreValuesClose(location.z, 0.0f);
			});
	}

	bool TransformComponentDetailCustomization::IsLocationIdenticalForInspected(EVectorComponent aComponent) const noexcept
	{
		Scene& scene = m_pBuilder->GetScene();
		EntityManager& entityManager = scene.GetEntityManager();

		const std::vector<entity>& entities = GetInspectedEntities();

		if (entities.size() <= 1)
			return true;

		auto&& GetLocation = [this, &entityManager](entity aEntity) -> Vector3
			{ 
				auto& tc = entityManager.Get<TransformComponent>(aEntity);
				return m_LocationSpace == ETransformSpace::Absolute ? tc.GetWorldLocation() : tc.GetLocalLocation();
			};
		
		const Vector3 referenceLocation = GetLocation(entities.front());

		auto&& Equals = [aComponent](const Vector3& aLocationA, const Vector3& aLocationB) -> bool
			{
				switch (aComponent)
				{
				case EVectorComponent::X: return Math::AreValuesClose(aLocationA.x, aLocationB.x);
				case EVectorComponent::Y: return Math::AreValuesClose(aLocationA.y, aLocationB.y);
				case EVectorComponent::Z: return Math::AreValuesClose(aLocationA.z, aLocationB.z);
				}
			};

		return std::ranges::all_of(std::next(entities.begin()), entities.end(), [&](entity aEntity)
			{
				return Equals(GetLocation(aEntity), referenceLocation);
			});
	}

	bool TransformComponentDetailCustomization::IsScaleDefaultForInspected() const noexcept
	{
		Scene& scene = m_pBuilder->GetScene();
		EntityManager& entityManager = scene.GetEntityManager();

		return std::ranges::all_of(GetInspectedEntities(), [this, &entityManager](entity aEntity)
			{
				auto& tc = entityManager.Get<TransformComponent>(aEntity);
				const Vector3 scale = m_ScaleSpace == ETransformSpace::Absolute ? tc.GetWorldScale() : tc.GetLocalScale();
				return Math::AreValuesClose(scale.x, 1.0f) && Math::AreValuesClose(scale.y, 1.0f) && Math::AreValuesClose(scale.z, 1.0f);
			});
	}

	bool TransformComponentDetailCustomization::IsScaleIdenticalForInspected(EVectorComponent aComponent) const noexcept
	{
		Scene& scene = m_pBuilder->GetScene();
		EntityManager& entityManager = scene.GetEntityManager();

		const std::vector<entity>& entities = GetInspectedEntities();

		if (entities.size() <= 1)
			return true;

		auto&& GetScale = [this, &entityManager](entity aEntity) -> Vector3
			{ 
				auto& tc = entityManager.Get<TransformComponent>(aEntity);
				return m_ScaleSpace == ETransformSpace::Absolute ? tc.GetWorldScale() : tc.GetLocalScale();
			};

		const Vector3 referenceScale = GetScale(entities.front());

		auto&& Equals = [aComponent](const Vector3& aScaleA, const Vector3& aScaleB) -> bool
			{ 
				switch (aComponent)
				{
					case EVectorComponent::X: return Math::AreValuesClose(aScaleA.x, aScaleB.x);
					case EVectorComponent::Y: return Math::AreValuesClose(aScaleA.y, aScaleB.y);
					case EVectorComponent::Z: return Math::AreValuesClose(aScaleA.z, aScaleB.z);
				}
			};

		return std::ranges::all_of(std::next(entities.begin()), entities.end(), [&](entity aEntity)
			{
				return Equals(GetScale(aEntity), referenceScale);
			});
	}

	bool TransformComponentDetailCustomization::IsRotationDefaultForInspected() const noexcept
	{
		Scene& scene = m_pBuilder->GetScene();
		EntityManager& entityManager = scene.GetEntityManager();

		return std::ranges::all_of(GetInspectedEntities(), [this, &entityManager](entity aEntity)
			{
				auto& tc = entityManager.Get<TransformComponent>(aEntity);
				const Vector3 rotation = m_RotationSpace == ETransformSpace::Absolute ? tc.GetWorldRotationEulerDegrees() : tc.GetLocalRotationEulerDegrees();
				return Math::AreValuesClose(rotation.x, 0.0f) && Math::AreValuesClose(rotation.y, 0.0f) && Math::AreValuesClose(rotation.z, 0.0f);
			});
	}

	bool TransformComponentDetailCustomization::IsRotationIdenticalForInspected(EVectorComponent aComponent) const noexcept
	{
		Scene& scene = m_pBuilder->GetScene();
		EntityManager& entityManager = scene.GetEntityManager();
		
		const std::vector<entity>& entities = GetInspectedEntities();

		if (entities.size() <= 1)
			return true;

		auto&& GetRotation = [this, &entityManager](entity aEntity) -> Vector3
			{ 
				auto& tc = entityManager.Get<TransformComponent>(aEntity);
				return m_RotationSpace == ETransformSpace::Absolute ? tc.GetWorldRotationEulerDegrees() : tc.GetLocalRotationEulerDegrees();
			};

		const Vector3 referenceRotation = GetRotation(entities.front());

		auto&& Equals = [aComponent](const Vector3& aRotationA, const Vector3& aRotationB) -> bool
			{
				switch (aComponent)
				{
				case EVectorComponent::X: return Math::AreValuesClose(aRotationA.x, aRotationB.x);
				case EVectorComponent::Y: return Math::AreValuesClose(aRotationA.y, aRotationB.y);
				case EVectorComponent::Z: return Math::AreValuesClose(aRotationA.z, aRotationB.z);
				}
			};

		return std::ranges::all_of(std::next(entities.begin()), entities.end(), [&](entity aEntity)
			{
				return Equals(GetRotation(aEntity), referenceRotation);
			});
	}

	void TransformComponentDetailCustomization::OnEditorShutdown() noexcept
	{
		m_ShouldDetachFromEditorCallbacks = false;
	}

	void TransformComponentDetailCustomization::OnEntityTransformed(entity aEntity) noexcept
	{
		if (m_SuspendNotifications)
			return;

		if (!std::ranges::any_of(GetInspectedEntities(), [aEntity](entity aInspectedEntity) { return aInspectedEntity == aEntity; }))
			return;

		m_pRevertLocationButton->SetIsVisible(!IsLocationDefaultForInspected());
		m_pRevertRotationButton->SetIsVisible(!IsRotationDefaultForInspected());
		m_pRevertScaleButton->SetIsVisible(!IsScaleDefaultForInspected());
	}

	void TransformComponentDetailCustomization::OnMouseEnterScaleButton(Button* aButton) noexcept
	{
		if (aButton)
			aButton->SetTextColor(Colors::White);
	}

	void TransformComponentDetailCustomization::OnMouseExitScaleButton(Button* aButton) noexcept
	{
		if (aButton && !m_ScaleLocked)
			aButton->SetTextColor(Colors::Gray);
	}

	void TransformComponentDetailCustomization::OnLocationChanged(const Vector3& aNewLocation) noexcept
	{
		Scene& scene = m_pBuilder->GetScene();
		ScopedSuspend suspend(m_SuspendNotifications);

		EditComponentData<TransformComponent>(scene, [&](entity aEntity, TransformComponent& aTC)
			{
				switch (m_LocationSpace)
				{
				case ETransformSpace::Absolute: aTC.SetWorldLocation(aNewLocation); break;
				case ETransformSpace::Relative: aTC.SetLocalLocation(aNewLocation); break;
				}

				Editor::OnEntityTransformed(aEntity);
			});

		m_pRevertLocationButton->SetIsVisible(!IsLocationDefaultForInspected());
	}

	void TransformComponentDetailCustomization::OnRotationChanged(const Vector3& aNewRotation, bool aIsAdditive) noexcept
	{
		Scene& scene = m_pBuilder->GetScene();
		EntityManager& entityManager = scene.GetEntityManager();

		std::ranges::for_each(GetInspectedEntities(), [&](entity aEntity)
			{
				auto& tc = entityManager.Get<TransformComponent>(aEntity);

				if (aIsAdditive)
				{
					const Quaternion currentRotation = m_RotationSpace == ETransformSpace::Absolute ? tc.GetWorldRotation() : tc.GetLocalRotation();
					const Vector3 currentEulerDeg = Math::RadToDeg(currentRotation.ToEuler());

					const Vector3 deltaEulerDeg = Vector3
					{
						aNewRotation.x - currentEulerDeg.x,
						aNewRotation.y - currentEulerDeg.y,
						aNewRotation.z - currentEulerDeg.z
					};

					// Convert deg deltas to rad and build quaternions:
					const Quaternion deltaQ = Quaternion::CreateFromAxisAngle(Vector3::Right, Math::DegToRad(deltaEulerDeg.x)) *
						Quaternion::CreateFromAxisAngle(Vector3::Up, Math::DegToRad(deltaEulerDeg.y)) *
						Quaternion::CreateFromAxisAngle(Vector3::Forward, Math::DegToRad(deltaEulerDeg.z));

					switch (m_RotationSpace)
					{
						case ETransformSpace::Absolute: tc.AddWorldRotationEulerDegrees(Math::RadToDeg(deltaQ.ToEuler())); break;
						case ETransformSpace::Relative: tc.AddLocalRotationEulerDegrees(Math::RadToDeg(deltaQ.ToEuler())); break;
					}
				}
				else
				{
					switch (m_RotationSpace)
					{
					case ETransformSpace::Absolute: tc.SetWorldRotation(Quaternion::CreateFromYawPitchRoll(Math::DegToRad(aNewRotation.x), Math::DegToRad(aNewRotation.y), Math::DegToRad(aNewRotation.z))); break;
					case ETransformSpace::Relative: tc.SetLocalRotation(Quaternion::CreateFromYawPitchRoll(Math::DegToRad(aNewRotation.x), Math::DegToRad(aNewRotation.y), Math::DegToRad(aNewRotation.z))); break;
					}
				}
				
			});

		m_pRevertRotationButton->SetIsVisible(!IsRotationDefaultForInspected());
	}

	void TransformComponentDetailCustomization::OnScaleChanged(const Vector3& aNewScale, EVectorComponent aComponent) noexcept
	{
		Scene& scene = m_pBuilder->GetScene();
		EntityManager& entityManager = scene.GetEntityManager();

		if (m_ScaleLocked)
		{
			const entity primary = GetPrimaryEntity();
			auto& tc = entityManager.Get<TransformComponent>(primary);

			const Vector3 currentScaleVector = m_ScaleSpace == ETransformSpace::Absolute ? tc.GetWorldScale() : tc.GetLocalScale();

			auto&& GetComponent = [](const Vector3& aScaleVector, EVectorComponent aComponent) -> float
				{
					switch (aComponent)
					{
						case EVectorComponent::X: return aScaleVector.x;
						case EVectorComponent::Y: return aScaleVector.y;
						case EVectorComponent::Z: return aScaleVector.z;
					}

					return 0.0f;
				};

			const float currentScale = GetComponent(currentScaleVector, aComponent);
			const float newScale = GetComponent(aNewScale, aComponent);

			const float delta = newScale - currentScale;
			const Vector3 scaleOffset(delta, delta, delta);

			std::ranges::for_each(GetInspectedEntities(), [&](entity aEntity)
				{
					auto& tc = entityManager.Get<TransformComponent>(aEntity);

					switch (m_ScaleSpace)
					{
					case ETransformSpace::Absolute:
					{
						const Vector3 currentWorldScale = tc.GetWorldScale();
						const Vector3 newWorldScale = currentWorldScale + scaleOffset;
						tc.SetWorldScale(newWorldScale);
						break;
					}
					case ETransformSpace::Relative:
					{
						const Vector3 currentLocalScale = tc.GetLocalScale();
						const Vector3 newLocalScale = currentLocalScale + scaleOffset;
						tc.SetLocalScale(newLocalScale);
						break;
					}
				}
			});
		}
		else
		{
			std::ranges::for_each(GetInspectedEntities(), [&](entity aEntity)
				{
					auto& tc = entityManager.Get<TransformComponent>(aEntity);

					switch (m_ScaleSpace)
					{
						case ETransformSpace::Absolute: tc.SetWorldScale(aNewScale); break;
						case ETransformSpace::Relative: tc.SetLocalScale(aNewScale); break;
					}
				});
		}

		m_pRevertScaleButton->SetIsVisible(!IsScaleDefaultForInspected());
	}

	Ref<ITableRow> TransformComponentDetailCustomization::OnRequestLocationRow(MAYBE_UNUSED const ItemInfo& aItemInfo) noexcept
	{
		const bool isMultiSelectionActive = GetInspectedEntities().size() > 1u;

		Ref<EntityDetailRow> pRow = new EntityDetailRow();

		{
			Ref<HorizontalBox> pBox = new HorizontalBox();

			Ref<HorizontalBox> pInnerBox = new HorizontalBox(Vector2(110.0f, 32.0f), true);
			pInnerBox->SetMargin(FloatRect(0.0f, 3.0f, 0.0f, 3.0f));
			pInnerBox->SetSpacing(0.0f);
			pInnerBox->SetSizePolicy(ESizePolicy::Fixed);

			pInnerBox->AddWidget(new ComboBox())
				->AddSelectables({ "Location", "Absolute Location" })
				->OnSelectionChanged(this, &TransformComponentDetailCustomization::OnLocationComboBoxSelectionChanged)
				->SetSizePolicy(ESizePolicy::Stretch);

			pBox->AddWidget(pInnerBox);

			pRow->SetColumnWidget(0, pBox);
		}
		{
			Ref<HorizontalBox> pBox = new HorizontalBox();

			Ref<HorizontalBox> pDragBox = new HorizontalBox();
			pDragBox->SetMargin(FloatRect(0.0f, 3.0f, 0.0f, 3.0f));
			pDragBox->SetSpacing(1.0f);

			if (isMultiSelectionActive)
			{
				{
					FloatEntryBox* pEntryBox = pDragBox->AddWidget(new FloatEntryBox(0.0f, GetLocationComponentFormat(EVectorComponent::X)));

					pEntryBox->Value([this]() { return GetLocation().x; });
					pEntryBox->OnValueChanged([this, pEntryBox](float aValueX)
						{
							const Vector3 currentLocation = GetLocation();
							OnLocationChanged(Vector3(aValueX, currentLocation.y, currentLocation.z));

							pEntryBox->SetFormat(GetLocationComponentFormat(EVectorComponent::X));
						});
					
					pEntryBox->SetIndicatorColor(Colors::OffRed);
					pEntryBox->SetSizePolicy(ESizePolicy::Stretch);
				}
				{
					FloatEntryBox* pEntryBox = pDragBox->AddWidget(new FloatEntryBox(0.0f, GetLocationComponentFormat(EVectorComponent::Y)));
					pEntryBox->Value([this]() { return GetLocation().y; });
					pEntryBox->OnValueChanged([this, pEntryBox](float aValueY)
						{
							const Vector3 currentLocation = GetLocation();
							OnLocationChanged(Vector3(currentLocation.x, aValueY, currentLocation.z));

							pEntryBox->SetFormat(GetLocationComponentFormat(EVectorComponent::Y));
						});
					
					pEntryBox->SetIndicatorColor(Colors::OffGreen);
					pEntryBox->SetSizePolicy(ESizePolicy::Stretch);
				}
				{
					FloatEntryBox* pEntryBox = pDragBox->AddWidget(new FloatEntryBox(0.0f, GetLocationComponentFormat(EVectorComponent::Z)));
					pEntryBox->Value([this]() { return GetLocation().z; });
					pEntryBox->OnValueChanged([this, pEntryBox](float aValueZ)
						{
							const Vector3 currentLocation = GetLocation();
							OnLocationChanged(Vector3(currentLocation.x, currentLocation.y, aValueZ));

							pEntryBox->SetFormat(GetLocationComponentFormat(EVectorComponent::Z));
						});
					pEntryBox->SetIndicatorColor(Colors::OffBlue);
					pEntryBox->SetSizePolicy(ESizePolicy::Stretch);
				}
			}
			else
			{
				pDragBox->AddWidget(new FloatDrag(0.01f, -FLT_MAX, FLT_MAX, "%.2f"))
					->Value([this]() { return GetLocation().x; })
					->OnValueChanged([this](float aValueX)
						{
							const Vector3 currentLocation = GetLocation();
							OnLocationChanged(Vector3(aValueX, currentLocation.y, currentLocation.z));
						})
					->SetIndicatorColor(Colors::OffRed)
					->SetSizePolicy(ESizePolicy::Stretch);

				pDragBox->AddWidget(new FloatDrag(0.01f, -FLT_MAX, FLT_MAX, "%.2f"))
					->Value([this]() { return GetLocation().y; })
					->OnValueChanged([this](float aValueY)
						{
							const Vector3 currentLocation = GetLocation();
							OnLocationChanged(Vector3(currentLocation.x, aValueY, currentLocation.z));
						})
					->SetIndicatorColor(Colors::OffGreen)
					->SetSizePolicy(ESizePolicy::Stretch);

				pDragBox->AddWidget(new FloatDrag(0.01f, -FLT_MAX, FLT_MAX, "%.2f"))
					->Value([this]() { return GetLocation().z; })
					->OnValueChanged([this](float aValueZ)
						{
							const Vector3 currentLocation = GetLocation();
							OnLocationChanged(Vector3(currentLocation.x, currentLocation.y, aValueZ));
						})
					->SetIndicatorColor(Colors::OffBlue)
					->SetSizePolicy(ESizePolicy::Stretch);
			}
			
			pBox->AddWidget(pDragBox);
			pRow->SetColumnWidget(1, pBox);
		}
		{
			Ref<HorizontalBox> pBox = new HorizontalBox();
			m_pRevertLocationButton = pBox->AddWidget(AddRevertButtonWidget([this](const Vector3& aValue) { OnLocationChanged(aValue); }, DEFAULT_LOCATION_ROTATION_VALUE, !IsLocationDefaultForInspected()));

			pRow->SetColumnWidget(2, pBox);
		}

		return pRow;
	}

	Ref<ITableRow> TransformComponentDetailCustomization::OnRequestRotationRow(MAYBE_UNUSED const ItemInfo& aItemInfo) noexcept
	{
		const bool isMultiSelectionActive = GetInspectedEntities().size() > 1u;

		Ref<EntityDetailRow> pRow = new EntityDetailRow();

		{
			Ref<HorizontalBox> pBox = new HorizontalBox();

			Ref<HorizontalBox> pInnerBox = new HorizontalBox(Vector2(110.0f, 32.0f), true);
			pInnerBox->SetMargin(FloatRect(0.0f, 3.0f, 0.0f, 3.0f));
			pInnerBox->SetSpacing(0.0f);
			pInnerBox->SetSizePolicy(ESizePolicy::Fixed);

			pInnerBox->AddWidget(new ComboBox())
				->AddSelectables({ "Rotation", "Absolute Rotation" })
				->OnSelectionChanged(this, &TransformComponentDetailCustomization::OnRotationComboBoxSelectionChanged)
				->SetSizePolicy(ESizePolicy::Stretch);

			pBox->AddWidget(pInnerBox);

			pRow->SetColumnWidget(0, pBox);
		}
		{
			Ref<HorizontalBox> pBox = new HorizontalBox();

			Ref<HorizontalBox> pDragBox = new HorizontalBox();
			pDragBox->SetMargin(FloatRect(0.0f, 3.0f, 0.0f, 3.0f));
			pDragBox->SetSpacing(1.0f);

			if (isMultiSelectionActive)
			{
				{
					FloatEntryBox* pEntryBox = pDragBox->AddWidget(new FloatEntryBox(0.0f, GetRotationComponentFormat(EVectorComponent::X)));

					pEntryBox->Value([this]() { return GetRotation().x; });
					pEntryBox->OnValueChanged([this, pEntryBox](float aValueX)
						{
							const Vector3 currentRotation = GetRotation();
							Vector3 newRotation = Vector3(aValueX, currentRotation.y, currentRotation.z);
							OnRotationChanged(newRotation);

							pEntryBox->SetFormat(GetRotationComponentFormat(EVectorComponent::X));
						});

					pEntryBox->SetIndicatorColor(Colors::OffRed);
					pEntryBox->SetSizePolicy(ESizePolicy::Stretch);
				}
				{
					FloatEntryBox* pEntryBox = pDragBox->AddWidget(new FloatEntryBox(0.0f, GetRotationComponentFormat(EVectorComponent::Y)));

					pEntryBox->Value([this]() { return GetRotation().y; });
					pEntryBox->OnValueChanged([this, pEntryBox](float aValueY)
						{
							const Vector3 currentRotation = GetRotation();
							Vector3 newRotation = Vector3(currentRotation.x, aValueY, currentRotation.z);
							OnRotationChanged(newRotation);

							pEntryBox->SetFormat(GetRotationComponentFormat(EVectorComponent::Y));
						});
					
					pEntryBox->SetIndicatorColor(Colors::OffGreen);
					pEntryBox->SetSizePolicy(ESizePolicy::Stretch);
				}
				{
					FloatEntryBox* pEntryBox = pDragBox->AddWidget(new FloatEntryBox(0.0f, GetRotationComponentFormat(EVectorComponent::Z)));

					pEntryBox->Value([this]() { return GetRotation().z; });
					pEntryBox->OnValueChanged([this, pEntryBox](float aValueZ)
						{
							const Vector3 currentRotation = GetRotation();
							Vector3 newRotation = Vector3(currentRotation.x, currentRotation.y, aValueZ);
							OnRotationChanged(newRotation);

							pEntryBox->SetFormat(GetRotationComponentFormat(EVectorComponent::Z));

						});
					pEntryBox->SetIndicatorColor(Colors::OffBlue);
					pEntryBox->SetSizePolicy(ESizePolicy::Stretch);
				}
			}
			else
			{
				pDragBox->AddWidget(new FloatDrag(1.0f, -FLT_MAX, FLT_MAX, "%.2f\xC2\xB0"))
					->Value([this]() { return GetRotation().x; })
					->OnValueChanged([this](float aValueX)
						{
							const Vector3 currentRotation = GetRotation();
							Vector3 newRotation = Vector3(aValueX, currentRotation.y, currentRotation.z);
							OnRotationChanged(newRotation);
						})
					->SetIndicatorColor(Colors::OffRed)
					->SetSizePolicy(ESizePolicy::Stretch);

				pDragBox->AddWidget(new FloatDrag(1.0f, -FLT_MAX, FLT_MAX, "%.2f\xC2\xB0"))
					->Value([this]() { return GetRotation().y; })
					->OnValueChanged([this](float aValueY)
						{
							const Vector3 currentRotation = GetRotation();
							Vector3 newRotation = Vector3(currentRotation.x, aValueY, currentRotation.z);
							OnRotationChanged(newRotation);
						})
					->SetIndicatorColor(Colors::OffGreen)
					->SetSizePolicy(ESizePolicy::Stretch);

				pDragBox->AddWidget(new FloatDrag(1.0f, -FLT_MAX, FLT_MAX, "%.2f\xC2\xB0"))
					->Value([this]() { return GetRotation().z; })
					->OnValueChanged([this](float aValueZ)
						{
							const Vector3 currentRotation = GetRotation();
							Vector3 newRotation = Vector3(currentRotation.x, currentRotation.y, aValueZ);
							OnRotationChanged(newRotation);
						})
					->SetIndicatorColor(Colors::OffBlue)
					->SetSizePolicy(ESizePolicy::Stretch);
			}

			pBox->AddWidget(pDragBox);
			pRow->SetColumnWidget(1, pBox);
		}
		{
			Ref<HorizontalBox> pBox = new HorizontalBox();
			m_pRevertRotationButton = pBox->AddWidget(AddRevertButtonWidget([this](const Vector3& aValue) { OnRotationChanged(aValue, false); }, DEFAULT_LOCATION_ROTATION_VALUE, !IsRotationDefaultForInspected()));
			pRow->SetColumnWidget(2, pBox);
		}

		return pRow;
	}

	Ref<ITableRow> TransformComponentDetailCustomization::OnRequestScaleRow(MAYBE_UNUSED const ItemInfo& aItemInfo) noexcept
	{
		const bool isMultiSelectionActive = GetInspectedEntities().size() > 1u;

		Ref<EntityDetailRow> pRow = new EntityDetailRow();

		{
			Ref<HorizontalBox> pBox = new HorizontalBox();

			Ref<HorizontalBox> pInnerBox = new HorizontalBox(Vector2(140.0f, 32.0f), true);
			pInnerBox->SetMargin(FloatRect(0.0f, 3.0f, 0.0f, 3.0f));
			pInnerBox->SetSpacing(0.0f);
			pInnerBox->SetSizePolicy(ESizePolicy::Fixed);

			pInnerBox->AddWidget(new ComboBox())
				->AddSelectables({ "Scale", "Absolute Scale" })
				->OnSelectionChanged(this, &TransformComponentDetailCustomization::OnScaleComboBoxSelectionChanged)
				->SetSizePolicy(ESizePolicy::Stretch);

			m_pScaleLockButton = pInnerBox->AddWidget(new Button(ICON_FA_LOCK_OPEN, Vector2(30.0f, 30.0f)))
				->OnClicked(this, &TransformComponentDetailCustomization::OnScaleLockButtonClicked)
				->OnMouseEnter(this, &TransformComponentDetailCustomization::OnMouseEnterScaleButton)
				->OnMouseExit(this, &TransformComponentDetailCustomization::OnMouseExitScaleButton)
				->SetBackgroundColor(Colors::Transparent)
				->SetBorderColor(Colors::Transparent)
				->SetHoverColor(Colors::Transparent)
				->SetActiveColor(Colors::Transparent)
				->SetTextColor(Colors::Gray)
				->SetFont(ImGui::GetIO().Fonts->Fonts[2]);

			pBox->AddWidget(pInnerBox);
			pRow->SetColumnWidget(0, pBox);
		}
		{
			Ref<HorizontalBox> pBox = new HorizontalBox();

			Ref<HorizontalBox> pDragBox = new HorizontalBox();
			pDragBox->SetMargin(FloatRect(0.0f, 3.0f, 0.0f, 3.0f));
			pDragBox->SetSpacing(1.0f);

			if (isMultiSelectionActive)
			{
				{
					FloatEntryBox* pEntryBoxX = pDragBox->AddWidget(new FloatEntryBox(0.0f, GetScaleComponentFormat(EVectorComponent::X)));

					pEntryBoxX->Value([this]() { return GetScale().x; });
					pEntryBoxX->OnValueChanged([this, pEntryBoxX](float aValueX)
						{
							const Vector3 currentScale = GetScale();
							Vector3 newScale(aValueX, currentScale.y, currentScale.z);
							OnScaleChanged(newScale, EVectorComponent::X);

							pEntryBoxX->SetFormat(GetScaleComponentFormat(EVectorComponent::X));
						});

					pEntryBoxX->SetIndicatorColor(Colors::OffRed);
					pEntryBoxX->SetSizePolicy(ESizePolicy::Stretch);
				}

				{
					FloatEntryBox* pEntryBoxY = pDragBox->AddWidget(new FloatEntryBox(0.0f, GetScaleComponentFormat(EVectorComponent::Y)));

					pEntryBoxY->Value([this]() { return GetScale().y; });

					pEntryBoxY->OnValueChanged([this, pEntryBoxY](float aValueY)
						{
							const Vector3 currentScale = GetScale();
							Vector3 newScale(currentScale.x, aValueY, currentScale.z);
							OnScaleChanged(newScale, EVectorComponent::Y);

							pEntryBoxY->SetFormat(GetScaleComponentFormat(EVectorComponent::Y));
						});

					pEntryBoxY->SetIndicatorColor(Colors::OffGreen);
					pEntryBoxY->SetSizePolicy(ESizePolicy::Stretch);
				}

				{
					FloatEntryBox* pEntryBoxZ = pDragBox->AddWidget(new FloatEntryBox(0.0f, GetScaleComponentFormat(EVectorComponent::Z)));

					pEntryBoxZ->Value([this]() { return GetScale().z; });

					pEntryBoxZ->OnValueChanged([this, pEntryBoxZ](float aValueZ)
						{
							const Vector3 currentScale = GetScale();
							Vector3 newScale(currentScale.x, currentScale.y, aValueZ);
							OnScaleChanged(newScale, EVectorComponent::Z);

							pEntryBoxZ->SetFormat(GetScaleComponentFormat(EVectorComponent::Z));
						});

					pEntryBoxZ->SetIndicatorColor(Colors::OffBlue);
					pEntryBoxZ->SetSizePolicy(ESizePolicy::Stretch);
				}

			}
			else
			{
				pDragBox->AddWidget(new FloatDrag(0.01f, -FLT_MAX, FLT_MAX, "%.2f"))
					->Value([this]() { return GetScale().x; })
					->OnValueChanged([this](float aValueX)
						{
							const Vector3 currentScale = GetScale();
							Vector3 newScale = Vector3(aValueX, currentScale.y, currentScale.z);
							OnScaleChanged(newScale, EVectorComponent::X);
						})
					->SetIndicatorColor(Colors::OffRed)
					->SetSizePolicy(ESizePolicy::Stretch);

				pDragBox->AddWidget(new FloatDrag(0.01f, -FLT_MAX, FLT_MAX, "%.2f"))
					->Value([this]() { return GetScale().y; })
					->OnValueChanged([this](float aValueY)
						{
							const Vector3 currentScale = GetScale();
							Vector3 newScale = Vector3(currentScale.x, aValueY, currentScale.z);
							OnScaleChanged(newScale, EVectorComponent::Y);
						})
					->SetIndicatorColor(Colors::OffGreen)
					->SetSizePolicy(ESizePolicy::Stretch);

				pDragBox->AddWidget(new FloatDrag(0.01f, -FLT_MAX, FLT_MAX, "%.2f"))
					->Value([this]() { return GetScale().z; })
					->OnValueChanged([this](float aValueZ)
						{
							const Vector3 currentScale = GetScale();
							Vector3 newScale = Vector3(currentScale.x, currentScale.y, aValueZ);
							OnScaleChanged(newScale, EVectorComponent::Z);
						})
					->SetIndicatorColor(Colors::OffBlue)
					->SetSizePolicy(ESizePolicy::Stretch);
			}

			pBox->AddWidget(pDragBox);
			pRow->SetColumnWidget(1, pBox);
		}
		{
			Ref<HorizontalBox> pBox = new HorizontalBox();
			m_pRevertScaleButton = pBox->AddWidget(AddRevertButtonWidget([this](const Vector3& aValue)
				{
					const bool scaleLocked = m_ScaleLocked;
					m_ScaleLocked = false;
					OnScaleChanged(aValue, EVectorComponent::X);
					OnScaleChanged(aValue, EVectorComponent::Y);
					OnScaleChanged(aValue, EVectorComponent::Z); 
					m_ScaleLocked = scaleLocked;

				}, DEFAULT_SCALE_VALUE, !IsScaleDefaultForInspected())
			);

			pRow->SetColumnWidget(2, pBox);
		}

		return pRow;
	}

	void TransformComponentDetailCustomization::OnScaleLockButtonClicked() noexcept
	{
		m_ScaleLocked = !m_ScaleLocked;
		m_pScaleLockButton->SetText(m_ScaleLocked ? ICON_FA_LOCK : ICON_FA_LOCK_OPEN);
		m_pScaleLockButton->SetTextColor(Colors::White);
	}

	void TransformComponentDetailCustomization::OnScaleComboBoxSelectionChanged(const ComboBox::SelectionInfo& aSelectionInfo) noexcept
	{
		m_ScaleSpace = static_cast<ETransformSpace>(aSelectionInfo.Index);
		m_pRevertScaleButton->SetIsVisible(!IsScaleDefaultForInspected());
	}

	void TransformComponentDetailCustomization::OnRotationComboBoxSelectionChanged(const ComboBox::SelectionInfo& aSelectionInfo) noexcept
	{
		m_RotationSpace = static_cast<ETransformSpace>(aSelectionInfo.Index);
		m_pRevertRotationButton->SetIsVisible(!IsRotationDefaultForInspected());
	}

	void TransformComponentDetailCustomization::OnLocationComboBoxSelectionChanged(const ComboBox::SelectionInfo& aSelectionInfo) noexcept
	{
		m_LocationSpace = static_cast<ETransformSpace>(aSelectionInfo.Index);
		m_pRevertLocationButton->SetIsVisible(!IsLocationDefaultForInspected());
	}
}
