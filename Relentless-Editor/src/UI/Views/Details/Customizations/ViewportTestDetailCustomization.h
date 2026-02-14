#pragma once
#include "IDetailCustomization.h"

namespace Relentless
{
	class ViewportTestDetailCustomization : public IDetailCustomization
	{
	public:
		virtual void CustomizeDetails(IDetailLayoutBuilder& aDetailLayoutBuilder) noexcept override;
	};
}