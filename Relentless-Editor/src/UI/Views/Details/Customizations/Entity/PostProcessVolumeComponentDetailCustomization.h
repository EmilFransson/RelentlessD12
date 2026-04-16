#pragma once
#include "UI/Views/Details/Customizations/IDetailCustomization.h"

namespace Relentless
{
	class IDetailLayoutBuilder;

	class PostProcessVolumeComponentDetailCustomization : public IDetailCustomization
	{
	protected:
		virtual void CustomizeDetails(IDetailLayoutBuilder& aDetailLayoutBuilder) noexcept override;

		NO_DISCARD virtual bool ShouldCustomize(IDetailLayoutBuilder& aDetailLayoutBuilder) const noexcept override;
	};
}