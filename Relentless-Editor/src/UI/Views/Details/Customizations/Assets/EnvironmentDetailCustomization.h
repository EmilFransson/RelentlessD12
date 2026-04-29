#pragma once
#include <Relentless.h>

#include "UI/Views/Details/Customizations/IDetailCustomization.h"

namespace Relentless
{
	class IDetailLayoutBuilder;

	class EnvironmentDetailCustomization : public IDetailCustomization
	{
	public:
		virtual ~EnvironmentDetailCustomization() noexcept override;
	protected:
		virtual void CustomizeDetails(IDetailLayoutBuilder& aDetailLayoutBuilder) noexcept override;

		NO_DISCARD virtual bool ShouldCustomize(IDetailLayoutBuilder& aDetailLayoutBuilder) const noexcept override;
	private:
		CallbackID m_OnAssetPropertyChangedCallbackID = INVALID_CALLBACK_ID;
		bool m_SuspendRefresh = false;
	};
}