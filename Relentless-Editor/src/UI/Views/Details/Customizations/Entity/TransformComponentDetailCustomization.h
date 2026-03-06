#pragma once

#include "UI/Views/Details/Customizations/IDetailCustomization.h"

namespace Relentless
{
	class TransformComponentDetailCustomization : public IDetailCustomization
	{
	public:
		virtual void CustomizeDetails(IDetailLayoutBuilder& aDetailLayoutBuilder) noexcept override;
	};
}