#pragma once
#include "ILightComponentDetailCustomization.h"

namespace Relentless
{
	class IDetailLayoutBuilder;

	class SpotLightComponentDetailCustomization : public ILightComponentDetailCustomization<SpotLightComponent>
	{
	public:
		virtual void CustomizeDetails(IDetailLayoutBuilder& aDetailLayoutBuilder) noexcept override;
	private:
		NO_DISCARD float GetAttenuationRadius() const noexcept;
		NO_DISCARD float GetInnerConeAngleDegrees() const noexcept;
		NO_DISCARD float GetOuterConeAngleDegrees() const noexcept;

		NO_DISCARD bool IsAttenuationRadiusDefaultForInspected() const noexcept;
		NO_DISCARD bool IsInnerConeAngleDefaultForInspected() const noexcept;
		NO_DISCARD bool IsOuterConeAngleDefaultForInspected() const noexcept;

		void OnAttenuationRadiusChanged(float aRadius) noexcept;
		void OnInnerConeAngleChanged(float aAngleDegrees) noexcept;
		void OnOuterConeAngleChanged(float aAngleDegrees) noexcept;

		NO_DISCARD Ref<ITableRow> OnRequestAttenuationRadiusRow(const ItemInfo& aItemInfo) noexcept;
		NO_DISCARD Ref<ITableRow> OnRequestInnerConeAngleRow(const ItemInfo& aItemInfo) noexcept;
		NO_DISCARD Ref<ITableRow> OnRequestOuterConeAngleRow(const ItemInfo& aItemInfo) noexcept;
	private:
		Button* m_pRevertAttenuationRadiusButton = nullptr;
		Button* m_pRevertInnerConeAngleButton = nullptr;
		Button* m_pRevertOuterConeAngleButton = nullptr;
	};
}
