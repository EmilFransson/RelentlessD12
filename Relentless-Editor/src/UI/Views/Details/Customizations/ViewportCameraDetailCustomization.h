#pragma once
#include "IDetailCustomization.h"

namespace Relentless
{
	class ViewportCameraDetailCustomization : public IDetailCustomization
	{
	public:
		virtual void CustomizeDetails(IDetailLayoutBuilder& aDetailLayoutBuilder) noexcept override;
	};
}
