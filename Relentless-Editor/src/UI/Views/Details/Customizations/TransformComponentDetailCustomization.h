#pragma once
#include "EntityDetailCustomization.h"

#include "UI/Widgets/ComboBox.h"

namespace Relentless
{
	enum class ETransformSpace : uint8 { Relative = 0, Absolute };
	enum class EVectorComponent : uint8 { X, Y, Z };

	class Button;
	class ITableRow;
	class IDetailLayoutBuilder;
	class EntityDetailLayoutBuilder;

	class TransformComponentDetailCustomization : public EntityDetailCustomization
	{
	public:
		TransformComponentDetailCustomization() noexcept;
		virtual ~TransformComponentDetailCustomization() noexcept;

		inline static constexpr Vector3 DEFAULT_LOCATION_ROTATION_VALUE = Vector3(0.0f, 0.0f, 0.0f);
		inline static constexpr Vector3 DEFAULT_SCALE_VALUE = Vector3(1.0f, 1.0f, 1.0f);

		virtual void CustomizeDetails(IDetailLayoutBuilder& aDetailLayoutBuilder) noexcept override;
	private:
		NO_DISCARD Vector3 GetLocation() noexcept;
		NO_DISCARD const char* GetLocationComponentFormat(EVectorComponent aComponent) noexcept;
		NO_DISCARD Vector3 GetRotation() noexcept;
		NO_DISCARD const char* GetRotationComponentFormat(EVectorComponent aComponent) noexcept;
		NO_DISCARD Vector3 GetScale() noexcept;
		NO_DISCARD const char* GetScaleComponentFormat(EVectorComponent aComponent) noexcept;

		NO_DISCARD bool IsLocationDefaultForInspected() const noexcept;
		NO_DISCARD bool IsLocationIdenticalForInspected(EVectorComponent aComponent) const noexcept;
		NO_DISCARD bool IsScaleDefaultForInspected() const noexcept;
		NO_DISCARD bool IsScaleIdenticalForInspected(EVectorComponent aComponent) const noexcept;
		NO_DISCARD bool IsRotationDefaultForInspected() const noexcept;
		NO_DISCARD bool IsRotationIdenticalForInspected(EVectorComponent aComponent) const noexcept;

		void OnEditorShutdown() noexcept;
		void OnEntityTransformed(entity aEntity) noexcept;

		void OnMouseEnterScaleButton(Button* aButton) noexcept;
		void OnMouseExitScaleButton(Button* aButton) noexcept;

		void OnLocationChanged(const Vector3& aNewLocation) noexcept;
		void OnRotationChanged(const Vector3& aNewRotation, bool aIsAdditive = true) noexcept;
		void OnScaleChanged(const Vector3& aNewScale, EVectorComponent aComponent) noexcept;

		NO_DISCARD Ref<ITableRow> OnRequestLocationRow(const ItemInfo& aItemInfo) noexcept;
		NO_DISCARD Ref<ITableRow> OnRequestRotationRow(const ItemInfo& aItemInfo) noexcept;
		NO_DISCARD Ref<ITableRow> OnRequestScaleRow(const ItemInfo& aItemInfo) noexcept;

		void OnScaleLockButtonClicked() noexcept;
		
		void OnScaleComboBoxSelectionChanged(const ComboBox::SelectionInfo& aSelectionInfo) noexcept;
		void OnRotationComboBoxSelectionChanged(const ComboBox::SelectionInfo& aSelectionInfo) noexcept;
		void OnLocationComboBoxSelectionChanged(const ComboBox::SelectionInfo& aSelectionInfo) noexcept;
	private:
		ETransformSpace m_LocationSpace = ETransformSpace::Relative;
		ETransformSpace m_RotationSpace = ETransformSpace::Relative;
		ETransformSpace m_ScaleSpace	= ETransformSpace::Relative;

		Button* m_pScaleLockButton = nullptr;
		Button* m_pRevertLocationButton = nullptr;
		Button* m_pRevertRotationButton = nullptr;
		Button* m_pRevertScaleButton = nullptr;
		
		bool m_ScaleLocked = false;
		bool m_SuspendNotifications = false;
		bool m_ShouldDetachFromEditorCallbacks = true;
	};
}
