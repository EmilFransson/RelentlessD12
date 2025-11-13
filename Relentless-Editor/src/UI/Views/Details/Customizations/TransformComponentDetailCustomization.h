#pragma once
#include "IDetailCustomization.h"

namespace Relentless
{
	class IDetailLayoutBuilder;
	class EntityDetailLayoutBuilder;

	class TransformComponentDetailCustomization : public IDetailCustomization
	{
	public:
		virtual void CustomizeDetails(IDetailLayoutBuilder& aDetailLayoutBuilder) noexcept override;
	private:
		NO_DISCARD Vector3 GetLocation() noexcept;
		NO_DISCARD Vector3 GetRotation() noexcept;
		NO_DISCARD Vector3 GetScale() noexcept;

		void OnMouseEnterScaleButton(Button* aButton) noexcept;
		void OnMouseExitScaleButton(Button* aButton) noexcept;

		void OnLocationChanged(Vector3& aNewLocation) noexcept;
		void OnRotationChanged(Vector3& aNewRotation) noexcept;
		void OnScaleChanged(Vector3& aNewScale) noexcept;

		NO_DISCARD Ref<ITableRow> OnRequestLocationRow(const ItemInfo& aItemInfo) noexcept;
		NO_DISCARD Ref<ITableRow> OnRequestRotationRow(const ItemInfo& aItemInfo) noexcept;
		NO_DISCARD Ref<ITableRow> OnRequestScaleRow(const ItemInfo& aItemInfo) noexcept;

		void OnScaleButtonClicked() noexcept;
	private:
		EntityDetailLayoutBuilder* m_pBuilder = nullptr;
		Button* m_pScaleLockButton = nullptr;
		bool m_ScaleLocked = false;
	};
}
