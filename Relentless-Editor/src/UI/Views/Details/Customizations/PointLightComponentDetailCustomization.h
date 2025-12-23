#pragma once
#include "ILightComponentDetailCustomization.h"

namespace Relentless
{
	class IDetailLayoutBuilder;
	class EntityDetailLayoutBuilder;

	class PointLightComponentDetailCustomization : public ILightComponentDetailCustomization<PointLightComponent>
	{
	public:
		virtual void CustomizeDetails(IDetailLayoutBuilder& aDetailLayoutBuilder) noexcept override;
	private:
		NO_DISCARD float GetAttenuationRadius() const noexcept;
		
		NO_DISCARD bool IsAttenuationRadiusDefaultForInspected() const noexcept;

		void OnAttenuationRadiusChanged(float aRadius) noexcept;

		NO_DISCARD Ref<ITableRow> OnRequestAttenuationRadiusRow(const ItemInfo& aItemInfo) noexcept;
	private:
		Button* m_pRevertAttenuationRadiusButton = nullptr;
	};
}
