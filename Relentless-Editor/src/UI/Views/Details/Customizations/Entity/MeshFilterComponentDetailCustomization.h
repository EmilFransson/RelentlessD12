#pragma once
#include "EntityDetailCustomization.h"

namespace Relentless
{
	class MeshFilterComponentDetailCustomization : public EntityDetailCustomization<MeshFilterComponent>
	{
		using IDetailCustomization::CustomizeDetails;
	protected:
		virtual void CustomizeDetails(IDetailLayoutBuilder& aDetailLayoutBuilder) noexcept override;
	private:
		void SetupConnections() noexcept;
	private:
		ScopedConnection<void(entity, TypeIndex, IComponent*, uint64)> m_OnMeshFilterComponentPropertyChangedConnection;
	};
}