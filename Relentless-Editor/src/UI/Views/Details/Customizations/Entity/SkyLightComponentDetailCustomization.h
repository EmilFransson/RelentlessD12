#pragma once
#include <Relentless.h>

#include "UI/Views/Details/Customizations/IDetailCustomization.h"

namespace Relentless
{
	class IDetailLayoutBuilder;

	class SkyLightComponentDetailCustomization : public IDetailCustomization
	{
	public:
		virtual ~SkyLightComponentDetailCustomization() noexcept override;
	protected:
		virtual void CustomizeDetails(IDetailLayoutBuilder& aDetailLayoutBuilder) noexcept override;

		NO_DISCARD virtual bool ShouldCustomize(IDetailLayoutBuilder& aDetailLayoutBuilder) const noexcept override;
	private:
		CallbackID m_OnSkyLightComponentPropertyChangedCallbackID = INVALID_CALLBACK_ID;
	};
}