#pragma once
#include "EntityDetailCustomization.h"

namespace Relentless
{
	class DirectionalLightComponentDetailCustomization : public EntityDetailCustomization<DirectionalLightComponent>
	{
		using IDetailCustomization::CustomizeDetails;
	protected:
		virtual void CustomizeDetails(IDetailLayoutBuilder& aDetailLayoutBuilder) noexcept override;
	private:
		void SetupConnections() noexcept;
	private:
		ScopedConnection<void(entity, TypeIndex, IComponent*, uint64)> m_OnDirectionalLightComponentPropertyChangedConnection;
	};
}