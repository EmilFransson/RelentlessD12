#include "ViewportTestDetailCustomization.h"

#include "UI/Views/Details/IDetailsView.h"
#include "UI/Views/Details/LayoutBuilders/IDetailLayoutBuilder.h"
#include "UI/Views/Details/LayoutBuilders/IDetailCategoryBuilder.h"
#include "UI/Views/Details/ViewportDetailsContext.h"

namespace Relentless
{
	void ViewportTestDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& aDetailLayoutBuilder) noexcept
	{
		ViewportDetailsContext& context = aDetailLayoutBuilder.GetDetailsView()->GetContext<ViewportDetailsContext>();

		Ref<EntityPropertyHandle<float, TransformComponent>> pPropertyHandle = RLS_NEW EntityPropertyHandle<float, TransformComponent>(*context.EntityManager, context.Entities, 
			[](const TransformComponent& aTransformComponent) 
			{ 
				return aTransformComponent.GetWorldLocation().x; 
			},
			[](TransformComponent& aTransformComponent, const float& aValue) 
			{ 
				const Vector3 worldLocation = aTransformComponent.GetWorldLocation();
				aTransformComponent.SetWorldLocation(Vector3(aValue, worldLocation.y, worldLocation.z));
			},
			0.0f
		);

		IDetailCategoryBuilder& builder = aDetailLayoutBuilder.EditCategory("Multi Property Test");
		
		DetailPropertyRowBuilder<float> rowBuilder = builder.AddProperty<float>("World Location (X)", pPropertyHandle);
		rowBuilder
			.SpinBox();
	}
}
