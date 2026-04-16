#pragma once
#include <Relentless.h>

#include "UI/Views/Details/Customizations/IDetailCustomization.h"

namespace Relentless
{
	class IDetailLayoutBuilder;

	class SkyBoxComponentDetailCustomization : public IDetailCustomization
	{
	public:
		virtual ~SkyBoxComponentDetailCustomization() noexcept override;

	protected:
		virtual void CustomizeDetails(IDetailLayoutBuilder& aDetailLayoutBuilder) noexcept override;

		NO_DISCARD virtual bool ShouldCustomize(IDetailLayoutBuilder& aDetailLayoutBuilder) const noexcept override;
	private:
		CallbackID m_OnSkyBoxComponentPropertyChangedCallbackID = INVALID_CALLBACK_ID;
	};
}