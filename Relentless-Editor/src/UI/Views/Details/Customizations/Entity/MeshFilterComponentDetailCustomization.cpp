#include "MeshFilterComponentDetailCustomization.h"

#include "Core/Editor.h"

#include "Subsystem/EngineContentSubsystem.h"

#include "UI/Views/Details/Context/EntityDetailsContext.h"
#include "UI/Views/Details/IDetailsView.h"
#include "UI/Views/Details/LayoutBuilders/IDetailLayoutBuilder.h"
#include "UI/Views/Details/LayoutBuilders/IDetailCategoryBuilder.h"
#include "UI/Views/Details/DetailHelpers.h"

namespace Relentless
{
	void MeshFilterComponentDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& aDetailLayoutBuilder) noexcept
	{
		using MFC = MeshFilterComponent;

		SetupConnections();

		EntityDetailsContext& detailsContext = aDetailLayoutBuilder.GetDetailsView()->GetContext<EntityDetailsContext>();
		IDetailCategoryBuilder& categoryBuilder = aDetailLayoutBuilder.EditCategory(ICON_FA_CUBE "  Mesh Filter");
		categoryBuilder.AddHeaderAction("Remove", [this]() { RemoveFromInspected(); });

		DetailHelpers::EntityHandleFactory<MFC> handleFactory({ .Entities = detailsContext.Entities, .EntityManager = *detailsContext.EntityManager });
		handleFactory.MakeAssetTarget(categoryBuilder, "Mesh", { Mesh::StaticType() }, &MFC::GetMeshHandle, &MFC::SetMesh, &MFC::RemoveMesh);
	}

	void MeshFilterComponentDetailCustomization::SetupConnections() noexcept
	{
		m_OnMeshFilterComponentPropertyChangedConnection = ScopedConnection(CoreObjectBroadcasters::OnEntityComponentPropertyChanged,
			[this](entity aEntity, TypeIndex aComponentType, MAYBE_UNUSED IComponent* aComponent, uint64 aProperty)
			{
				if (aComponentType != MeshFilterComponent::StaticType())
					return;
				if (!IsEntityInspected(aEntity))
					return;
				if (aProperty == "m_MeshHandle"_h)
				{
					if (IDetailLayoutBuilder* pLayoutBuilder = GetDetailLayoutBuilder())
						pLayoutBuilder->ForceRefreshDetails();
				}
			});
	}
}