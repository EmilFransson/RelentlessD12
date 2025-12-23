#pragma once
#include "ILightComponentDetailCustomization.h"

namespace Relentless
{
	class IDetailLayoutBuilder;
	class EntityDetailLayoutBuilder;

	class DirectionalLightComponentDetailCustomization : public ILightComponentDetailCustomization<DirectionalLightComponent>
	{
	public:
		virtual void CustomizeDetails(IDetailLayoutBuilder& aDetailLayoutBuilder) noexcept override;
	};
}
