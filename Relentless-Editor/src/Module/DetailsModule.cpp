#include "DetailsModule.h"

#include "UI/Views/Details/Customizations/Assets/EnvironmentDetailCustomization.h"
#include "UI/Views/Details/Customizations/Assets/MaterialDetailCustomization.h"

#include "UI/Views/Details/Customizations/Entity/DirectionalLightComponentDetailCustomization.h"
#include "UI/Views/Details/Customizations/Entity/MeshFilterComponentDetailCustomization.h"
#include "UI/Views/Details/Customizations/Entity/MeshRendererComponentDetailCustomization.h"
#include "UI/Views/Details/Customizations/Entity/PointLightComponentDetailCustomization.h"
#include "UI/Views/Details/Customizations/Entity/PostProcessVolumeComponentDetailCustomization.h"
#include "UI/Views/Details/Customizations/Entity/SkyBoxComponentDetailCustomization.h"
#include "UI/Views/Details/Customizations/Entity/SkyLightComponentDetailCustomization.h"
#include "UI/Views/Details/Customizations/Entity/SpotLightComponentDetailCustomization.h"
#include "UI/Views/Details/Customizations/Entity/TransformComponentDetailCustomization.h"

#include "UI/Views/Details/Customizations/Viewport/ViewportCameraDetailCustomization.h"

#include "UI/Views/Details/Context/EntityDetailsContext.h"
#include "UI/Views/Details/Context/EnvironmentDetailsContext.h"
#include "UI/Views/Details/Context/MaterialDetailsContext.h"
#include "UI/Views/Details/Context/ViewportDetailsContext.h"

namespace Relentless
{
	const DetailCustomizationRegistry& DetailsModule::GetRegistry() const noexcept
	{
		return m_DetailCustomizationRegistry;
	}

	void DetailsModule::OnLoad()
	{
		//Viewport Details:
		m_DetailCustomizationRegistry.Register<ViewportDetailsContext, ViewportCameraDetailCustomization>();

		//Entity Details:
		m_DetailCustomizationRegistry.Register<EntityDetailsContext, TransformComponentDetailCustomization>();
		m_DetailCustomizationRegistry.Register<EntityDetailsContext, DirectionalLightComponentDetailCustomization>();
		m_DetailCustomizationRegistry.Register<EntityDetailsContext, PointLightComponentDetailCustomization>();
		m_DetailCustomizationRegistry.Register<EntityDetailsContext, SpotLightComponentDetailCustomization>();
		m_DetailCustomizationRegistry.Register<EntityDetailsContext, SkyBoxComponentDetailCustomization>();
		m_DetailCustomizationRegistry.Register<EntityDetailsContext, SkyLightComponentDetailCustomization>();
		m_DetailCustomizationRegistry.Register<EntityDetailsContext, PostProcessVolumeComponentDetailCustomization>();
		m_DetailCustomizationRegistry.Register<EntityDetailsContext, MeshRendererComponentDetailCustomization>();
		m_DetailCustomizationRegistry.Register<EntityDetailsContext, MeshFilterComponentDetailCustomization>();

		//Assets
		m_DetailCustomizationRegistry.Register<EnvironmentDetailsContext, EnvironmentDetailCustomization>();
		m_DetailCustomizationRegistry.Register<MaterialDetailsContext, MaterialDetailCustomization>();
	}
}
