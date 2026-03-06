#include "TransformComponentDetailCustomization.h"

#include <Relentless.h>

#include "UI/Views/Details/IDetailsView.h"
#include "UI/Views/Details/LayoutBuilders/IDetailLayoutBuilder.h"
#include "UI/Views/Details/LayoutBuilders/IDetailCategoryBuilder.h"
#include "UI/Views/Details/Context/EntityDetailsContext.h"

namespace Relentless
{
	static constexpr Color AxisColors[3] = { Colors::Red, Colors::Green, Colors::Blue };

	NO_DISCARD static float& Component(Vector3& aVector, int aComponentIndex) noexcept
	{
		RLS_ASSERT(aComponentIndex >= 0 && aComponentIndex < 3, "[Component]: Vector component index out of range");
		return (&aVector.x)[aComponentIndex];
	}

	NO_DISCARD static float Component(const Vector3& aVector, int aComponentIndex) noexcept
	{
		RLS_ASSERT(aComponentIndex >= 0 && aComponentIndex < 3, "[Component]: Vector component index out of range");
		return (&aVector.x)[aComponentIndex];
	}

	NO_DISCARD static ETransformSpace GetTransformSpace(PropertyHandle<int>* aPropertyHandle) noexcept
	{
		int space = 0;
		RLS_VERIFY(aPropertyHandle->GetValue(space) == EPropertyAccessResult::Success, "Transform space property handle is ambiguous.");
		return static_cast<ETransformSpace>(space);
	}

	static void SyncRevertButtonState(Button* aButton, bool aIsEnabled) noexcept
	{
		aButton->SetIsEnabled(aIsEnabled);
		aButton->SetTextColor(aIsEnabled ? Colors::Gray : Colors::Transparent);
	}

	NO_DISCARD static bool LocationDiffersFrom(const Vector3& aValue, const EntityDetailsContext& aContext, ETransformSpace aTransformSpace) noexcept
	{
		return !std::ranges::all_of(aContext.Entities, [aTransformSpace, &aContext, &aValue](entity aEntity)
			{
				if (aTransformSpace == ETransformSpace::Absolute)
					return aContext.EntityManager->Get<TransformComponent>(aEntity).GetWorldLocation() == aValue;
				else
					return aContext.EntityManager->Get<TransformComponent>(aEntity).GetLocalLocation() == aValue;
			});
	}

	NO_DISCARD static bool LocationDiffersFrom(float aValue, int aComponentIndex, const EntityDetailsContext& aContext, ETransformSpace aTransformSpace) noexcept
	{
		return !std::ranges::all_of(aContext.Entities, [aTransformSpace, &aContext, aValue, aComponentIndex](entity aEntity)
			{
				if (aTransformSpace == ETransformSpace::Absolute)
					return Component(aContext.EntityManager->Get<TransformComponent>(aEntity).GetWorldLocation(), aComponentIndex) == aValue;
				else
					return Component(aContext.EntityManager->Get<TransformComponent>(aEntity).GetLocalLocation(), aComponentIndex) == aValue;
			});
	}

	NO_DISCARD static bool ScaleDiffersFrom(const Vector3& aValue, const EntityDetailsContext& aContext, ETransformSpace aTransformSpace) noexcept
	{
		return !std::ranges::all_of(aContext.Entities, [aTransformSpace, &aContext, &aValue](entity aEntity)
			{
				if (aTransformSpace == ETransformSpace::Absolute)
					return aContext.EntityManager->Get<TransformComponent>(aEntity).GetWorldScale() == aValue;
				else
					return aContext.EntityManager->Get<TransformComponent>(aEntity).GetLocalScale() == aValue;
			});
	}

	NO_DISCARD static bool ScaleDiffersFrom(float aValue, int aComponentIndex, const EntityDetailsContext& aContext, ETransformSpace aTransformSpace) noexcept
	{
		return !std::ranges::all_of(aContext.Entities, [aTransformSpace, &aContext, aValue, aComponentIndex](entity aEntity)
			{
				if (aTransformSpace == ETransformSpace::Absolute)
					return Component(aContext.EntityManager->Get<TransformComponent>(aEntity).GetWorldScale(), aComponentIndex) == aValue;
				else
					return Component(aContext.EntityManager->Get<TransformComponent>(aEntity).GetLocalScale(), aComponentIndex) == aValue;
			});
	}

	NO_DISCARD static bool RotationDiffersFrom(const Vector3& aValue, const EntityDetailsContext& aContext, ETransformSpace aTransformSpace) noexcept
	{
		return !std::ranges::all_of(aContext.Entities, [aTransformSpace, &aContext, &aValue](entity aEntity)
			{
				if (aTransformSpace == ETransformSpace::Absolute)
					return aContext.EntityManager->Get<TransformComponent>(aEntity).GetWorldRotationEulerDegrees() == aValue;
				else
					return aContext.EntityManager->Get<TransformComponent>(aEntity).GetLocalRotationEulerDegrees() == aValue;
			});
	}

	NO_DISCARD static bool RotationDiffersFrom(float aValue, int aComponentIndex, const EntityDetailsContext& aContext, ETransformSpace aTransformSpace) noexcept
	{
		return !std::ranges::all_of(aContext.Entities, [aTransformSpace, &aContext, aValue, aComponentIndex](entity aEntity)
			{
				if (aTransformSpace == ETransformSpace::Absolute)
					return Component(aContext.EntityManager->Get<TransformComponent>(aEntity).GetWorldRotationEulerDegrees(), aComponentIndex) == aValue;
				else
					return Component(aContext.EntityManager->Get<TransformComponent>(aEntity).GetLocalRotationEulerDegrees(), aComponentIndex) == aValue;
			});
	}

	NO_DISCARD static Ref<Button> CreateRevertButton(bool aEnabled) noexcept
	{
		Ref<Button> pRevertButton = RLS_NEW Button(ICON_FA_ARROW_ROTATE_LEFT);
		pRevertButton->SetBackgroundColor(Colors::Transparent);
		pRevertButton->SetBorderColor(Colors::Transparent);
		pRevertButton->SetHoverColor(Colors::Transparent);
		pRevertButton->SetActiveColor(Colors::Transparent);
		pRevertButton->SetFont(ImGui::GetIO().Fonts->Fonts[2]);
		pRevertButton->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);
		pRevertButton->OnMouseEnter([](Button* aButton) 
			{  
				if (!aButton->IsEnabled())
					return;

				aButton->SetTextColor(Colors::White); 
			});
		pRevertButton->OnMouseExit([](Button* aButton) 
			{ 
				if (!aButton->IsEnabled())
					return;
				
				aButton->SetTextColor(Colors::Gray); 
			});

		SyncRevertButtonState(pRevertButton, aEnabled);
		return pRevertButton;
	}

	NO_DISCARD static Ref<NumericEntryBox<float>> CreateRotationNumericEntryBox(EntityDetailsContext& aContext, const Ref<PropertyHandle<int>>& aSpacePropertyHandle, int aComponentIndex, Button* aButton) noexcept
	{
		Ref<NumericEntryBox<float>> pNumericEntryBox = RLS_NEW NumericEntryBox<float>();
		pNumericEntryBox->SetSuffix("\xC2\xB0");
		pNumericEntryBox->SetSteppingEnabled(false);
		pNumericEntryBox->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);
		pNumericEntryBox->SetHorizontalSizePolicy(ESizePolicy::Stretch);
		
		const ETransformSpace transformSpace = GetTransformSpace(aSpacePropertyHandle);
		const TransformComponent& tc = aContext.EntityManager->Get<TransformComponent>(aContext.Entities.front());
		const Vector3 degrees = (transformSpace == ETransformSpace::Absolute) ? tc.GetWorldRotationEulerDegrees() : tc.GetLocalRotationEulerDegrees();
		const float referenceDegrees = Component(degrees, aComponentIndex);

		if (RotationDiffersFrom(referenceDegrees, aComponentIndex, aContext, transformSpace))
			pNumericEntryBox->SetDisplayText("Mixed");

		pNumericEntryBox->Value([&aContext, aSpacePropertyHandle, aComponentIndex]()
			{
				if (GetTransformSpace(aSpacePropertyHandle) == ETransformSpace::Absolute)
					return Component(aContext.CachedEulerWorldRotation, aComponentIndex);
				else
					return Component(aContext.CachedEulerLocalRotation, aComponentIndex);
			});

		pNumericEntryBox->OnValueCommitted([&aContext, aSpacePropertyHandle, numericEntryBox = pNumericEntryBox.Get(), aComponentIndex](float aValue, MAYBE_UNUSED ETextCommitType aCommitType)
			{
				const ETransformSpace transformSpace = GetTransformSpace(aSpacePropertyHandle);
				Vector3 newRotation = (transformSpace == ETransformSpace::Absolute) ? aContext.CachedEulerWorldRotation : aContext.CachedEulerLocalRotation;

				if (transformSpace == ETransformSpace::Absolute)
				{
					Component(newRotation, aComponentIndex) = aValue;
					Component(aContext.CachedEulerWorldRotation, aComponentIndex) = aValue;
				}
				else
				{
					Component(newRotation, aComponentIndex) = aValue;
					Component(aContext.CachedEulerLocalRotation, aComponentIndex) = aValue;
				}

				for (const entity aEntity : aContext.Entities)
				{
					TransformComponent& tc = aContext.EntityManager->Get<TransformComponent>(aEntity);
					Quaternion targetRotation = Math::EulerDegreesToQuaternion_YawPitchRoll(newRotation);
					targetRotation.Normalize();

					if (transformSpace == ETransformSpace::Absolute)
						tc.SetWorldRotation(targetRotation);
					else
						tc.SetLocalRotation(targetRotation);
				}

				numericEntryBox->SetSuffix("\xC2\xB0");
			});

		return pNumericEntryBox;
	}

	NO_DISCARD static Ref<NumericEntryBox<float>> CreateLocationNumericEntryBox(EntityDetailsContext& aContext, const Ref<PropertyHandle<int>>& aSpacePropertyHandle, int aComponentIndex, Button* aButton) noexcept
	{
		Ref<NumericEntryBox<float>> pNumericEntryBox = RLS_NEW NumericEntryBox<float>();
		pNumericEntryBox->SetSteppingEnabled(false);
		pNumericEntryBox->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);
		pNumericEntryBox->SetHorizontalSizePolicy(ESizePolicy::Stretch);

		const ETransformSpace transformSpace = GetTransformSpace(aSpacePropertyHandle);
		const TransformComponent& referenceTC = aContext.EntityManager->Get<TransformComponent>(aContext.Entities.front());
		const float referenceLocationComponent = (transformSpace == ETransformSpace::Absolute) ? Component(referenceTC.GetWorldLocation(), aComponentIndex) : Component(referenceTC.GetLocalLocation(), aComponentIndex);

		if (LocationDiffersFrom(referenceLocationComponent, aComponentIndex, aContext, transformSpace))
			pNumericEntryBox->SetDisplayText("Mixed");

		pNumericEntryBox->Value([&aContext, aSpacePropertyHandle, aComponentIndex]()
			{
				if (GetTransformSpace(aSpacePropertyHandle) == ETransformSpace::Absolute)
					return Component(aContext.EntityManager->Get<TransformComponent>(aContext.Entities.front()).GetWorldLocation(), aComponentIndex);
				else
					return Component(aContext.EntityManager->Get<TransformComponent>(aContext.Entities.front()).GetLocalLocation(), aComponentIndex);
			});

		pNumericEntryBox->OnValueCommitted([&aContext, aSpacePropertyHandle, numericEntryBox = pNumericEntryBox.Get(), aComponentIndex](float aValue, MAYBE_UNUSED ETextCommitType aCommitType)
			{
				const ETransformSpace transformSpace = GetTransformSpace(aSpacePropertyHandle);

				for (const entity aEntity : aContext.Entities)
				{
					TransformComponent& tc = aContext.EntityManager->Get<TransformComponent>(aEntity);
					if (transformSpace == ETransformSpace::Absolute)
					{
						Vector3 location = tc.GetWorldLocation();
						Component(location, aComponentIndex) = aValue;
						tc.SetWorldLocation(location);
					}
					else
					{
						Vector3 location = tc.GetLocalLocation();
						Component(location, aComponentIndex) = aValue;
						tc.SetLocalLocation(location);
					}
				}
			});

		return pNumericEntryBox;
	}

	NO_DISCARD static Ref<NumericEntryBox<float>> CreateScaleNumericEntryBox(EntityDetailsContext& aContext, const Ref<PropertyHandle<int>>& aSpacePropertyHandle, int aComponentIndex, Button* aButton) noexcept
	{
		Ref<NumericEntryBox<float>> pNumericEntryBox = RLS_NEW NumericEntryBox<float>();
		pNumericEntryBox->SetSteppingEnabled(false);
		pNumericEntryBox->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);
		pNumericEntryBox->SetHorizontalSizePolicy(ESizePolicy::Stretch);

		const ETransformSpace transformSpace = GetTransformSpace(aSpacePropertyHandle);
		const TransformComponent& referenceTC = aContext.EntityManager->Get<TransformComponent>(aContext.Entities.front());
		const float referenceScaleComponent = (transformSpace == ETransformSpace::Absolute) ? Component(referenceTC.GetWorldScale(), aComponentIndex) : Component(referenceTC.GetLocalScale(), aComponentIndex);

		if (ScaleDiffersFrom(referenceScaleComponent, aComponentIndex, aContext, transformSpace))
			pNumericEntryBox->SetDisplayText("Mixed");

		pNumericEntryBox->Value([&aContext, aSpacePropertyHandle, aComponentIndex]()
			{
				if (GetTransformSpace(aSpacePropertyHandle) == ETransformSpace::Absolute)
					return Component(aContext.EntityManager->Get<TransformComponent>(aContext.Entities.front()).GetWorldScale(), aComponentIndex);
				else
					return Component(aContext.EntityManager->Get<TransformComponent>(aContext.Entities.front()).GetLocalScale(), aComponentIndex);
			});

		pNumericEntryBox->OnValueCommitted([&aContext, aSpacePropertyHandle, numericEntryBox = pNumericEntryBox.Get(), aComponentIndex](float aValue, MAYBE_UNUSED ETextCommitType aCommitType)
			{
				const ETransformSpace transformSpace = GetTransformSpace(aSpacePropertyHandle);

				for (const entity aEntity : aContext.Entities)
				{
					TransformComponent& tc = aContext.EntityManager->Get<TransformComponent>(aEntity);
					if (aContext.ScaleLocked)
					{
						const Vector3 newScale = Vector3(Math::Max(0.01f, aValue), Math::Max(0.01f, aValue), Math::Max(0.01f, aValue));
						if (transformSpace == ETransformSpace::Absolute)
							tc.SetWorldScale(newScale);
						else
							tc.SetLocalScale(newScale);
					}
					else
					{
						Vector3 scale = (transformSpace == ETransformSpace::Absolute) ? tc.GetWorldScale() : tc.GetLocalScale();
						Component(scale, aComponentIndex) = aValue;
						if (transformSpace == ETransformSpace::Absolute)
							tc.SetWorldScale(scale);
						else
							tc.SetLocalScale(scale);
					}
				}
			});

		return pNumericEntryBox;
	}

	NO_DISCARD Ref<SpinBox<float>> CreateLocationSpinBox(EntityDetailsContext& aContext, const Ref<PropertyHandle<int>>& aSpacePropertyHandle, int aComponentIndex, Button* aButton) noexcept
	{
		Ref<SpinBox<float>> pSpinBox = RLS_NEW SpinBox<float>();
		pSpinBox->SetDelta(0.01f);
		pSpinBox->SetDrawColorIndicator(true);
		pSpinBox->SetIndicatorColor(AxisColors[aComponentIndex]);
		pSpinBox->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);
		pSpinBox->SetHorizontalSizePolicy(ESizePolicy::Stretch);

		pSpinBox->Value([&aContext, aComponentIndex, aSpacePropertyHandle]()
			{
				if (GetTransformSpace(aSpacePropertyHandle) == ETransformSpace::Absolute)
					return Component(aContext.EntityManager->Get<TransformComponent>(aContext.Entities.front()).GetWorldLocation(), aComponentIndex);
				else
					return Component(aContext.EntityManager->Get<TransformComponent>(aContext.Entities.front()).GetLocalLocation(), aComponentIndex);
			});
		pSpinBox->OnValueChanged([&aContext, aSpacePropertyHandle, aComponentIndex, aButton](float aValue)
			{
				const ETransformSpace transformSpace = GetTransformSpace(aSpacePropertyHandle);
				SyncRevertButtonState(aButton, LocationDiffersFrom(Vector3::Zero, aContext, transformSpace));

				for (const entity aEntity : aContext.Entities)
				{
					TransformComponent& tc = aContext.EntityManager->Get<TransformComponent>(aEntity);
					if (transformSpace == ETransformSpace::Absolute)
					{
						Vector3 worldLocation = tc.GetWorldLocation();
						Component(worldLocation, aComponentIndex) = aValue;
						tc.SetWorldLocation(worldLocation);
					}
					else
					{
						Vector3 localLocation = tc.GetLocalLocation();
						Component(localLocation, aComponentIndex) = aValue;
						tc.SetLocalLocation(localLocation);
					}
				}
			});

		return pSpinBox;
	}

	NO_DISCARD Ref<SpinBox<float>> CreateRotationSpinBox(EntityDetailsContext& aContext, const Ref<PropertyHandle<int>>& aSpacePropertyHandle, int aComponentIndex, Button* aButton) noexcept
	{
		Ref<SpinBox<float>> pSpinBox = RLS_NEW SpinBox<float>();
		pSpinBox->SetDelta(1.0f);
		pSpinBox->SetDrawColorIndicator(true);
		pSpinBox->SetIndicatorColor(AxisColors[aComponentIndex]);
		pSpinBox->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);
		pSpinBox->SetHorizontalSizePolicy(ESizePolicy::Stretch);
		pSpinBox->SetSuffix("\xC2\xB0");

		pSpinBox->Value([&aContext, aComponentIndex, aSpacePropertyHandle]()
			{
				if (GetTransformSpace(aSpacePropertyHandle) == ETransformSpace::Absolute)
					return Component(aContext.CachedEulerWorldRotation, aComponentIndex); 
				else
					return Component(aContext.CachedEulerLocalRotation, aComponentIndex);
			});
		pSpinBox->OnValueChanged([&aContext, aSpacePropertyHandle, aComponentIndex, aButton](float aValue)
			{
				const ETransformSpace transformSpace = GetTransformSpace(aSpacePropertyHandle);
				Vector3 newRotation = (transformSpace == ETransformSpace::Absolute) ? aContext.CachedEulerWorldRotation : aContext.CachedEulerLocalRotation;
				Component(newRotation, aComponentIndex) = aValue;

				if (transformSpace == ETransformSpace::Absolute)
				{
					Component(aContext.CachedEulerWorldRotation, aComponentIndex) = aValue;
					SyncRevertButtonState(aButton, aContext.CachedEulerWorldRotation != Vector3::Zero);
				}
				else
				{
					Component(aContext.CachedEulerLocalRotation, aComponentIndex) = aValue;
					SyncRevertButtonState(aButton, aContext.CachedEulerLocalRotation != Vector3::Zero);
				}

				for (const entity aEntity : aContext.Entities)
				{
					TransformComponent& tc = aContext.EntityManager->Get<TransformComponent>(aEntity);
					Quaternion targetRotation = Math::EulerDegreesToQuaternion_YawPitchRoll(newRotation);
					targetRotation.Normalize();
					
					if (transformSpace == ETransformSpace::Absolute)
						tc.SetWorldRotation(targetRotation);
					else
						tc.SetLocalRotation(targetRotation);
				}
			});

		return pSpinBox;
	}

	NO_DISCARD Ref<SpinBox<float>> CreateScaleSpinBox(EntityDetailsContext& aContext, const Ref<PropertyHandle<int>>& aSpacePropertyHandle, int aComponentIndex, Button* aButton) noexcept
	{
		Ref<SpinBox<float>> pSpinBox = RLS_NEW SpinBox<float>();
		pSpinBox->SetDelta(0.01f);
		pSpinBox->SetDrawColorIndicator(true);
		pSpinBox->SetIndicatorColor(AxisColors[aComponentIndex]);
		pSpinBox->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);
		pSpinBox->SetHorizontalSizePolicy(ESizePolicy::Stretch);
		pSpinBox->SetMinValue(0.01f);

		pSpinBox->Value([&aContext, aComponentIndex, aSpacePropertyHandle]()
			{
				if (GetTransformSpace(aSpacePropertyHandle) == ETransformSpace::Absolute)
					return Component(aContext.EntityManager->Get<TransformComponent>(aContext.Entities.front()).GetWorldScale(), aComponentIndex);
				else
					return Component(aContext.EntityManager->Get<TransformComponent>(aContext.Entities.front()).GetLocalScale(), aComponentIndex);
			});
		pSpinBox->OnValueChanged([&aContext, aSpacePropertyHandle, aComponentIndex, aButton](float aValue)
			{
				const ETransformSpace transformSpace = GetTransformSpace(aSpacePropertyHandle);

				for (const entity aEntity : aContext.Entities)
				{
					TransformComponent& tc = aContext.EntityManager->Get<TransformComponent>(aEntity);
					Vector3 scale = (transformSpace == ETransformSpace::Absolute) ? tc.GetWorldScale() : tc.GetLocalScale();
					if (aContext.ScaleLocked)
					{
						const float diff = aValue - Component(scale, aComponentIndex);
						const Vector3 newScale = Vector3(Math::Max(0.01f, scale.x + diff), Math::Max(0.01f, scale.y + diff), Math::Max(0.01f, scale.z + diff));

						if (transformSpace == ETransformSpace::Absolute)
							tc.SetWorldScale(newScale);
						else
							tc.SetLocalScale(newScale);
					}
					else
					{
						Component(scale, aComponentIndex) = aValue;
						if (transformSpace == ETransformSpace::Absolute)
							tc.SetWorldScale(scale);
						else
							tc.SetLocalScale(scale);
					}
				}

				SyncRevertButtonState(aButton, ScaleDiffersFrom(Vector3::One, aContext, transformSpace));
			});

		return pSpinBox;
	}

	NO_DISCARD static Ref<IBaseWidget> CreateRotationWidget(EntityDetailsContext& aContext, const Ref<PropertyHandle<int>>& aSpacePropertyHandle, int aComponentIndex, Button* aButton) noexcept
	{
		if (aContext.Entities.size() == 1u)
			return CreateRotationSpinBox(aContext, aSpacePropertyHandle, aComponentIndex, aButton);
		else
			return CreateRotationNumericEntryBox(aContext, aSpacePropertyHandle, aComponentIndex, aButton);
	}

	NO_DISCARD static Ref<IBaseWidget> CreateLocationWidget(EntityDetailsContext& aContext, const Ref<PropertyHandle<int>>& aSpacePropertyHandle, int aComponentIndex, Button* aButton) noexcept
	{
		if (aContext.Entities.size() == 1u)
			return CreateLocationSpinBox(aContext, aSpacePropertyHandle, aComponentIndex, aButton);
		else
			return CreateLocationNumericEntryBox(aContext, aSpacePropertyHandle, aComponentIndex, aButton);
	}

	NO_DISCARD static Ref<IBaseWidget> CreateScaleWidget(EntityDetailsContext& aContext, const Ref<PropertyHandle<int>>& aSpacePropertyHandle, int aComponentIndex, Button* aButton) noexcept
	{
		if (aContext.Entities.size() == 1u)
			return CreateScaleSpinBox(aContext, aSpacePropertyHandle, aComponentIndex, aButton);
		else
			return CreateScaleNumericEntryBox(aContext, aSpacePropertyHandle, aComponentIndex, aButton);
	}

	static void CustomizeLocationRow(IDetailCategoryBuilder& aCategoryBuilder, EntityDetailsContext& aContext) noexcept
	{
		Ref<PropertyHandle<int>> pLocationComboHandle = RLS_NEW PropertyHandle<int>(
			[&aContext]() { return static_cast<int>(aContext.LocationTransformSpace); },
			[&aContext](const int& aValue) { aContext.LocationTransformSpace = static_cast<ETransformSpace>(aValue); });
		
		Ref<Button> pRevertButton = CreateRevertButton(LocationDiffersFrom(Vector3::Zero, aContext, GetTransformSpace(pLocationComboHandle)));
		pLocationComboHandle->OnValueChanged.Connect([&aContext, revertButton = pRevertButton.Get(), locationComboHandle = pLocationComboHandle.Get()](const int& aValue)
			{
				SyncRevertButtonState(revertButton, LocationDiffersFrom(Vector3::Zero, aContext, GetTransformSpace(locationComboHandle)));
			});
		pRevertButton->OnClicked([&aContext, locationComboHandle = pLocationComboHandle.Get(), pSelf = pRevertButton.Get()]()
			{
				const ETransformSpace transformSpace = GetTransformSpace(locationComboHandle);

				for (const entity aEntity : aContext.Entities)
				{
					TransformComponent& tc = aContext.EntityManager->Get<TransformComponent>(aEntity);
					if (transformSpace == ETransformSpace::Absolute)
						tc.SetLocalLocation(Vector3::Zero);
					else
						tc.SetWorldLocation(Vector3::Zero);
				}

				SyncRevertButtonState(pSelf, false);
			});

		aCategoryBuilder.AddProperty<Vector3>("Location", nullptr)
			.NameSlot().ComboBox().Bind(pLocationComboHandle).Options({ "Location", "Absolute Location" }).Row()
			.ValueSlot().Widget([&aContext, pLocationComboHandle, revertButton = pRevertButton.Get()]()
				{
					Ref<HorizontalBox> pBox = RLS_NEW HorizontalBox();
					pBox->AddWidget(CreateLocationWidget(aContext, pLocationComboHandle, 0, revertButton));
					pBox->AddWidget(CreateLocationWidget(aContext, pLocationComboHandle, 1, revertButton));
					pBox->AddWidget(CreateLocationWidget(aContext, pLocationComboHandle, 2, revertButton));
					return pBox;
				})
			.RevertSlot().Widget([pRevertButton]() { return pRevertButton; });
	}

	static void CustomizeRotationRow(IDetailCategoryBuilder& aCategoryBuilder, EntityDetailsContext& aContext) noexcept
	{
		Ref<PropertyHandle<int>> pRotationComboHandle = RLS_NEW PropertyHandle<int>(
			[&aContext]() { return static_cast<int>(aContext.RotationTransformSpace); },
			[&aContext](const int& aValue) { aContext.RotationTransformSpace = static_cast<ETransformSpace>(aValue); });

		Ref<Button> pRevertButton = CreateRevertButton(RotationDiffersFrom(Vector3::Zero, aContext, GetTransformSpace(pRotationComboHandle)));
		pRotationComboHandle->OnValueChanged.Connect([&aContext, revertButton = pRevertButton.Get()](const int& aValue)
			{
				const ETransformSpace transformSpace = static_cast<ETransformSpace>(aValue);
				SyncRevertButtonState(revertButton, RotationDiffersFrom(Vector3::Zero, aContext, transformSpace));
			});
		pRevertButton->OnClicked([&aContext, rotationComboHandle = pRotationComboHandle.Get(), pSelf = pRevertButton.Get()]()
			{
				const ETransformSpace transformSpace = GetTransformSpace(rotationComboHandle);

				for (const entity aEntity : aContext.Entities)
				{
					TransformComponent& tc = aContext.EntityManager->Get<TransformComponent>(aEntity);
					if (transformSpace == ETransformSpace::Absolute)
					{
						aContext.CachedEulerWorldRotation = Vector3::Zero;
						tc.SetLocalRotationEulerDegrees(Vector3::Zero);
					}
					else
					{
						aContext.CachedEulerLocalRotation = Vector3::Zero;
						tc.SetWorldRotationEulerDegrees(Vector3::Zero);
					}
				}

				SyncRevertButtonState(pSelf, false);
			});

		aCategoryBuilder.AddProperty<Vector3>("Rotation", nullptr)
			.NameSlot().ComboBox().Bind(pRotationComboHandle).Options({ "Rotation", "Absolute Rotation" }).Row()
			.ValueSlot().Widget([&aContext, pRotationComboHandle, revertButton = pRevertButton.Get()]()
				{
					Ref<HorizontalBox> pBox = RLS_NEW HorizontalBox();
					pBox->AddWidget(CreateRotationWidget(aContext, pRotationComboHandle, 0, revertButton));
					pBox->AddWidget(CreateRotationWidget(aContext, pRotationComboHandle, 1, revertButton));
					pBox->AddWidget(CreateRotationWidget(aContext, pRotationComboHandle, 2, revertButton));
					return pBox;
				})
			.RevertSlot().Widget([pRevertButton]() { return pRevertButton; });
	}

	static void CustomizeScaleRow(IDetailCategoryBuilder& aCategoryBuilder, EntityDetailsContext& aContext) noexcept
	{
		Ref<PropertyHandle<int>> pScaleComboHandle = RLS_NEW PropertyHandle<int>(
			[&aContext]() { return static_cast<int>(aContext.ScaleTransformSpace); },
			[&aContext](const int& aValue) { aContext.ScaleTransformSpace = static_cast<ETransformSpace>(aValue); });

		Ref<Button> pRevertButton = CreateRevertButton(ScaleDiffersFrom(Vector3::One, aContext, GetTransformSpace(pScaleComboHandle)));
		pScaleComboHandle->OnValueChanged.Connect([&aContext, revertButton = pRevertButton.Get()](const int& aValue)
			{
				const ETransformSpace transformSpace = static_cast<ETransformSpace>(aValue);
				SyncRevertButtonState(revertButton, ScaleDiffersFrom(Vector3::One, aContext, transformSpace));
			});
		pRevertButton->OnClicked([&aContext, scaleComboHandle = pScaleComboHandle.Get(), pSelf = pRevertButton.Get()]()
			{
				const ETransformSpace transformSpace = GetTransformSpace(scaleComboHandle);

				for (const entity aEntity : aContext.Entities)
				{
					TransformComponent& tc = aContext.EntityManager->Get<TransformComponent>(aEntity);
					if (transformSpace == ETransformSpace::Absolute)
						tc.SetLocalScale(Vector3::One);
					else
						tc.SetWorldScale(Vector3::One);
				}

				SyncRevertButtonState(pSelf, false);
			});

		aCategoryBuilder.AddProperty<Vector3>("Scale", nullptr)
			.NameSlot().Widget([&aContext, pScaleComboHandle]()
				{
					Ref<HorizontalBox> pBox = RLS_NEW HorizontalBox();
					pBox->AddWidget(RLS_NEW ComboBox())
						->Bind(pScaleComboHandle)
						->AddSelectables({ "Scale", "Absolute Scale" })
						->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);

					Button* pButton = pBox->AddWidget(RLS_NEW Button(aContext.ScaleLocked ? ICON_FA_LOCK : ICON_FA_LOCK_OPEN));
					pButton->SetBackgroundColor(Colors::Transparent);
					pButton->SetBorderColor(Colors::Transparent);
					pButton->SetHoverColor(Colors::Transparent);
					pButton->SetActiveColor(Colors::Transparent);
					pButton->SetTextColor(Colors::Gray);
					pButton->SetFont(ImGui::GetIO().Fonts->Fonts[2]);
					pButton->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);

					pButton->OnMouseEnter([](Button* aButton) { aButton->SetTextColor(Colors::White); });
					pButton->OnMouseExit([](Button* aButton) { aButton->SetTextColor(Colors::Gray); });

					pButton->OnClicked([pButton, &aContext]()
						{
							aContext.ScaleLocked = !aContext.ScaleLocked;
							pButton->SetText(aContext.ScaleLocked ? ICON_FA_LOCK : ICON_FA_LOCK_OPEN);
						});

					return pBox;
				})
			.ValueSlot().Widget([&aContext, pScaleComboHandle, revertButton = pRevertButton.Get()]()
				{
					Ref<HorizontalBox> pBox = RLS_NEW HorizontalBox();
					pBox->AddWidget(CreateScaleWidget(aContext, pScaleComboHandle, 0, revertButton));
					pBox->AddWidget(CreateScaleWidget(aContext, pScaleComboHandle, 1, revertButton));
					pBox->AddWidget(CreateScaleWidget(aContext, pScaleComboHandle, 2, revertButton));
					return pBox;
				})
			.RevertSlot().Widget([pRevertButton]() { return pRevertButton; });
	}

	void TransformComponentDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& aDetailLayoutBuilder) noexcept
	{
		EntityDetailsContext& context = aDetailLayoutBuilder.GetDetailsView()->GetContext<EntityDetailsContext>();
		if (context.Entities.empty())
			return;

		const TransformComponent& tc = context.EntityManager->Get<TransformComponent>(context.Entities.front());
		context.CachedEulerLocalRotation = tc.GetLocalRotationEulerDegrees();
		context.CachedEulerWorldRotation = tc.GetWorldRotationEulerDegrees();

		IDetailCategoryBuilder& categoryBuilder = aDetailLayoutBuilder.EditCategory(ICON_FA_ARROWS_UP_DOWN_LEFT_RIGHT "  Transform");
		CustomizeLocationRow(categoryBuilder, context);
		CustomizeRotationRow(categoryBuilder, context);
		CustomizeScaleRow(categoryBuilder, context);
	}

}