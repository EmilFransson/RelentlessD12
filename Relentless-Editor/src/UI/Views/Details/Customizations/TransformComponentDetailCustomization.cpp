#include "TransformComponentDetailCustomization.h"

#include "../../../../Core/Selection.h"
#include "../LayoutBuilders/EntityDetailLayoutBuilder.h"
#include "../TableRows/EntityDetailRow.h"

namespace Relentless
{
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
		return m_pBuilder->GetScene().GetWorldLocation(m_pBuilder->GetSelection().GetFirstSelected());
	}

	Vector3 TransformComponentDetailCustomization::GetRotation() noexcept
	{
		return Math::RadToDeg360(m_pBuilder->GetScene().GetWorldRotation(m_pBuilder->GetSelection().GetFirstSelected()).ToEuler());

		//const ETransformSpace transformSpace = static_cast<ETransformSpace>(pTransformSpaceComboBox->GetSelectedIndex());

		//switch (transformSpace)
		//{
		//case ETransformSpace::Relative:
		//	return m_pEditor->GetActiveScene()->GetLocalRotation(m_SelectedEntities[0]).ToEuler();
		//case ETransformSpace::Absolute:
		//	return m_pEditor->GetActiveScene()->GetWorldRotation(m_SelectedEntities[0]).ToEuler();
		//default:
		//	RLS_ASSERT(false, "Unreachable.");
		//	return Vector3::Zero;
		//}
	}

	Vector3 TransformComponentDetailCustomization::GetScale() noexcept
	{
		return m_pBuilder->GetScene().GetWorldScale(m_pBuilder->GetSelection().GetFirstSelected());
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

	void TransformComponentDetailCustomization::OnLocationChanged(Vector3& aNewLocation) noexcept
	{
		Scene& scene = m_pBuilder->GetScene();
		//const ETransformSpace space = static_cast<ETransformSpace>(pTransformSpaceComboBox->GetSelectedIndex());
		const std::vector<entity>& selectedEntities = m_pBuilder->GetSelection().GetSelectedEntities();
		std::for_each(selectedEntities.begin(), selectedEntities.end(), [&](entity aEntity)
			{
				scene.SetWorldLocation(aEntity, aNewLocation);

				//switch (space)
				//{
				//case ETransformSpace::Absolute: pScene->SetWorldLocation(e, value); break;
				//case ETransformSpace::Relative: pScene->SetLocalLocation(e, value); break;
				//}
			});
	}

	void TransformComponentDetailCustomization::OnRotationChanged(Vector3& aNewRotation) noexcept
	{
		Scene& scene = m_pBuilder->GetScene();
		const std::vector<entity>& selectedEntities = m_pBuilder->GetSelection().GetSelectedEntities();
		//const ETransformSpace space = static_cast<ETransformSpace>(pTransformSpaceComboBox->GetSelectedIndex());

		std::for_each(selectedEntities.begin(), selectedEntities.end(), [&](entity aEntity)
			{
				//const Quaternion currentRotation = space == ETransformSpace::Absolute ? pScene->GetWorldRotation(e) : pScene->GetLocalRotation(e);
				const Quaternion currentRotation = scene.GetWorldRotation(aEntity);

				const Vector3 currentEulerDeg = Math::RadToDeg(currentRotation.ToEuler());

				const Vector3 deltaEulerDeg = Vector3
				{
					aNewRotation.x - currentEulerDeg.x,
					aNewRotation.y - currentEulerDeg.y,
					aNewRotation.z - currentEulerDeg.z
				};

				// Convert deg deltas to rad and build quaternions:
				Quaternion deltaQ = Quaternion::CreateFromAxisAngle(Vector3::Right, Math::DegToRad(deltaEulerDeg.x)) *
					Quaternion::CreateFromAxisAngle(Vector3::Up, Math::DegToRad(deltaEulerDeg.y)) *
					Quaternion::CreateFromAxisAngle(Vector3::Forward, Math::DegToRad(deltaEulerDeg.z));

				scene.AddWorldRotation(aEntity, Math::RadToDeg(deltaQ.ToEuler()));

				//switch (space)
				//{
				//case ETransformSpace::Absolute: pScene->AddWorldRotation(e, Math::RadToDeg(deltaQ.ToEuler())); break;
				//case ETransformSpace::Relative: pScene->AddLocalRotation(e, Math::RadToDeg(deltaQ.ToEuler())); break;
				//}
			});
	}

	void TransformComponentDetailCustomization::OnScaleChanged(Vector3& aNewScale) noexcept
	{
		Scene& scene = m_pBuilder->GetScene();
		//const ETransformSpace space = static_cast<ETransformSpace>(pTransformSpaceComboBox->GetSelectedIndex());
		const std::vector<entity>& selectedEntities = m_pBuilder->GetSelection().GetSelectedEntities();
		std::for_each(selectedEntities.begin(), selectedEntities.end(), [&](entity aEntity)
			{
				scene.SetWorldScale(aEntity, aNewScale);

				//switch (space)
				//{
				//case ETransformSpace::Absolute: pScene->SetWorldLocation(e, value); break;
				//case ETransformSpace::Relative: pScene->SetLocalLocation(e, value); break;
				//}
			});
	}

	Ref<ITableRow> TransformComponentDetailCustomization::OnRequestLocationRow(const ItemInfo& aItemInfo) noexcept
	{
		Ref<EntityDetailRow> pRow = new EntityDetailRow();

		{
			Ref<HorizontalBoxEx> pBox = new HorizontalBoxEx();

			Ref<HorizontalBoxEx> pInnerBox = new HorizontalBoxEx(Vector2(110.0f, 32.0f), true);
			pInnerBox->SetMargin(IntRect(0.0f, 3.0f, 0.0f, 3.0f));
			pInnerBox->SetSpacing(0.0f);
			pInnerBox->SetSizePolicy(ESizePolicy::Fixed);

			pInnerBox->AddWidget(new ComboBox())
				->AddSelectables({ "Location", "Absolute Location" })
				->SetSizePolicy(ESizePolicy::Stretch);

			pBox->AddWidget(pInnerBox);

			pRow->SetColumnWidget(0, pBox);
		}
		{
			Ref<HorizontalBoxEx> pBox = new HorizontalBoxEx();

			Ref<HorizontalBoxEx> pDragBox = new HorizontalBoxEx();
			pDragBox->SetMargin(IntRect(0,3,0,3));
			pDragBox->SetSpacing(1.0f);

			pDragBox->AddWidget(new FloatDrag(0.01f, -FLT_MAX, FLT_MAX, "%.2f"))
				->Value([this]() { return GetLocation().x; })
				->OnValueChanged([this](float aValueX) 
					{ 
						const Vector3 currentLocation = GetLocation();
						Vector3 newLocation = Vector3(aValueX, currentLocation.y, currentLocation.z);
						OnLocationChanged(newLocation); 
					})
				->SetIndicatorColor(Colors::OffRed)
				->SetSizePolicy(ESizePolicy::Stretch);

			pDragBox->AddWidget(new FloatDrag(0.01f, -FLT_MAX, FLT_MAX, "%.2f"))
				->Value([this]() { return GetLocation().y; })
				->OnValueChanged([this](float aValueY)
					{
						const Vector3 currentLocation = GetLocation();
						Vector3 newLocation = Vector3(currentLocation.x, aValueY, currentLocation.z);
						OnLocationChanged(newLocation);
					})
				->SetIndicatorColor(Colors::OffGreen)
				->SetSizePolicy(ESizePolicy::Stretch);

			pDragBox->AddWidget(new FloatDrag(0.01f, -FLT_MAX, FLT_MAX, "%.2f"))
				->Value([this]() { return GetLocation().z; })
				->OnValueChanged([this](float aValueZ)
					{
						const Vector3 currentLocation = GetLocation();
						Vector3 newLocation = Vector3(currentLocation.x, currentLocation.y, aValueZ);
						OnLocationChanged(newLocation);
					})
				->SetIndicatorColor(Colors::OffBlue)
				->SetSizePolicy(ESizePolicy::Stretch);
			
			pBox->AddWidget(pDragBox);
			pRow->SetColumnWidget(1, pBox);
		}

		return pRow;
	}

	Ref<ITableRow> TransformComponentDetailCustomization::OnRequestRotationRow(const ItemInfo& aItemInfo) noexcept
	{
		Ref<EntityDetailRow> pRow = new EntityDetailRow();

		{
			Ref<HorizontalBoxEx> pBox = new HorizontalBoxEx();

			Ref<HorizontalBoxEx> pInnerBox = new HorizontalBoxEx(Vector2(110.0f, 32.0f), true);
			pInnerBox->SetMargin(IntRect(0.0f, 3.0f, 0.0f, 3.0f));
			pInnerBox->SetSpacing(0.0f);
			pInnerBox->SetSizePolicy(ESizePolicy::Fixed);

			pInnerBox->AddWidget(new ComboBox())
				->AddSelectables({ "Rotation", "Absolute Rotation" })
				->SetSizePolicy(ESizePolicy::Stretch);

			pBox->AddWidget(pInnerBox);

			pRow->SetColumnWidget(0, pBox);
		}
		{
			Ref<HorizontalBoxEx> pBox = new HorizontalBoxEx();

			Ref<HorizontalBoxEx> pDragBox = new HorizontalBoxEx();
			pDragBox->SetMargin(IntRect(0, 3, 0, 3));
			pDragBox->SetSpacing(1.0f);

			pDragBox->AddWidget(new FloatDrag(1.0f, -FLT_MAX, FLT_MAX, "%.2f\xC2\xB0"))
				->Value([this]() { return GetRotation().x; })
				->OnValueChanged([this](float aValueX)
					{
						const Vector3 currentRotation = GetRotation();
						Vector3 newLocation = Vector3(aValueX, currentRotation.y, currentRotation.z);
						OnRotationChanged(newLocation);
					})
				->SetIndicatorColor(Colors::OffRed)
				->SetSizePolicy(ESizePolicy::Stretch);

			pDragBox->AddWidget(new FloatDrag(1.0f, -FLT_MAX, FLT_MAX, "%.2f\xC2\xB0"))
				->Value([this]() { return GetRotation().y; })
				->OnValueChanged([this](float aValueY)
					{
						const Vector3 currentRotation = GetRotation();
						Vector3 newLocation = Vector3(currentRotation.x, aValueY, currentRotation.z);
						OnRotationChanged(newLocation);
					})
				->SetIndicatorColor(Colors::OffGreen)
				->SetSizePolicy(ESizePolicy::Stretch);

			pDragBox->AddWidget(new FloatDrag(1.0f, -FLT_MAX, FLT_MAX, "%.2f\xC2\xB0"))
				->Value([this]() { return GetRotation().z; })
				->OnValueChanged([this](float aValueZ)
					{
						const Vector3 currentRotation = GetRotation();
						Vector3 newLocation = Vector3(currentRotation.x, currentRotation.y, aValueZ);
						OnRotationChanged(newLocation);
					})
				->SetIndicatorColor(Colors::OffBlue)
				->SetSizePolicy(ESizePolicy::Stretch);

			pBox->AddWidget(pDragBox);
			pRow->SetColumnWidget(1, pBox);
		}

		return pRow;
	}

	Ref<ITableRow> TransformComponentDetailCustomization::OnRequestScaleRow(const ItemInfo& aItemInfo) noexcept
	{
		Ref<EntityDetailRow> pRow = new EntityDetailRow();

		{
			Ref<HorizontalBoxEx> pBox = new HorizontalBoxEx();

			Ref<HorizontalBoxEx> pInnerBox = new HorizontalBoxEx(Vector2(140.0f, 32.0f), true);
			pInnerBox->SetMargin(IntRect(0.0f, 3.0f, 0.0f, 3.0f));
			pInnerBox->SetSpacing(0.0f);
			pInnerBox->SetSizePolicy(ESizePolicy::Fixed);

			pInnerBox->AddWidget(new ComboBox())
				->AddSelectables({ "Scale", "Absolute Scale" })
				->SetSizePolicy(ESizePolicy::Stretch);

			m_pScaleLockButton = pInnerBox->AddWidget(new Button(ICON_FA_LOCK_OPEN, Vector2(30, 30)))
				->OnClicked(this, &TransformComponentDetailCustomization::OnScaleButtonClicked)
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
			Ref<HorizontalBoxEx> pBox = new HorizontalBoxEx();

			Ref<HorizontalBoxEx> pDragBox = new HorizontalBoxEx();
			pDragBox->SetMargin(IntRect(0, 3, 0, 3));
			pDragBox->SetSpacing(1.0f);

			pDragBox->AddWidget(new FloatDrag(0.01f, 0.01f, FLT_MAX, "%.2f"))
				->Value([this]() { return GetScale().x; })
				->OnValueChanged([this](float aValueX)
					{
						const Vector3 currentScale = GetScale();
						Vector3 newLocation = Vector3(aValueX, currentScale.y, currentScale.z);
						OnScaleChanged(newLocation);
					})
				->SetIndicatorColor(Colors::OffRed)
				->SetSizePolicy(ESizePolicy::Stretch);

			pDragBox->AddWidget(new FloatDrag(0.01f, 0.01f, FLT_MAX, "%.2f"))
				->Value([this]() { return GetScale().y; })
				->OnValueChanged([this](float aValueY)
					{
						const Vector3 currentScale = GetScale();
						Vector3 newLocation = Vector3(currentScale.x, aValueY, currentScale.z);
						OnScaleChanged(newLocation);
					})
				->SetIndicatorColor(Colors::OffGreen)
				->SetSizePolicy(ESizePolicy::Stretch);

			pDragBox->AddWidget(new FloatDrag(0.01f, 0.01f, FLT_MAX, "%.2f"))
				->Value([this]() { return GetScale().z; })
				->OnValueChanged([this](float aValueZ)
					{
						const Vector3 currentScale = GetScale();
						Vector3 newLocation = Vector3(currentScale.x, currentScale.y, aValueZ);
						OnScaleChanged(newLocation);
					})
				->SetIndicatorColor(Colors::OffBlue)
				->SetSizePolicy(ESizePolicy::Stretch);

			pBox->AddWidget(pDragBox);
			pRow->SetColumnWidget(1, pBox);
		}

		return pRow;
	}

	void TransformComponentDetailCustomization::OnScaleButtonClicked() noexcept
	{
		m_ScaleLocked = !m_ScaleLocked;
		m_pScaleLockButton->SetText(m_ScaleLocked ? ICON_FA_LOCK : ICON_FA_LOCK_OPEN);
		m_pScaleLockButton->SetTextColor(Colors::White);
	}

}
