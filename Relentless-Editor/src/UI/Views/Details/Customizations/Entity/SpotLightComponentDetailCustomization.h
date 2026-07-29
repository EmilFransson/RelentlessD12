#pragma once
#include "EntityDetailCustomization.h"

namespace Relentless
{
	class SpotLightComponentDetailCustomization : public EntityDetailCustomization<SpotLightComponent>
	{
		using IDetailCustomization::CustomizeDetails;
	protected:
		virtual void CustomizeDetails(IDetailLayoutBuilder& aDetailLayoutBuilder) noexcept override;
	private:
		void SetupConnections() noexcept;
	private:
		ScopedConnection<void(entity, TypeIndex, IComponent*, uint64)> m_OnSpotLightComponentPropertyChangedConnection;
	};
}