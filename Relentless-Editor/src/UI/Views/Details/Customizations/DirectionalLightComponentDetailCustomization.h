#pragma once
#include "IDetailCustomization.h"

namespace Relentless
{
	class IDetailLayoutBuilder;
	class EntityDetailLayoutBuilder;

	class DirectionalLightComponentDetailCustomization : public IDetailCustomization
	{
	public:
		virtual void CustomizeDetails(IDetailLayoutBuilder& aDetailLayoutBuilder) noexcept override;
	private:
		NO_DISCARD Color GetColor() const noexcept;
		NO_DISCARD float GetIntensity() const noexcept;
		NO_DISCARD float GetTemperature() const noexcept;
		NO_DISCARD bool GetUseTemperature() const noexcept;

		NO_DISCARD bool IsTemperatureEnabled() const noexcept;

		void OnColorChanged(const Color& aColor) noexcept;
		void OnIntensityChanged(float aIntensity) noexcept;
		void OnTemperatureChanged(float aTemperature) noexcept;
		void OnUseTemperatureCheckStateChanged(bool aState) noexcept;

		NO_DISCARD Ref<ITableRow> OnRequestColorRow(const ItemInfo& aItemInfo) noexcept;
		NO_DISCARD Ref<ITableRow> OnRequestTypeRow(const ItemInfo& aItemInfo) noexcept;
		NO_DISCARD Ref<ITableRow> OnRequestIntensityRow(const ItemInfo& aItemInfo) noexcept;
		NO_DISCARD Ref<ITableRow> OnRequestTemperatureRow(const ItemInfo& aItemInfo) noexcept;
		NO_DISCARD Ref<ITableRow> OnRequestUseTemperatureRow(const ItemInfo& aItemInfo) noexcept;
	private:
		EntityDetailLayoutBuilder* m_pBuilder = nullptr;
		FloatSlider* m_pTemperatureSlider = nullptr;
	};
}
