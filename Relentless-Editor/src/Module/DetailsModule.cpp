#include "DetailsModule.h"

#include "UI/Views/Details/Customizations/ViewportCameraDetailCustomization.h"
#include "UI/Views/Details/Customizations/ViewportTestDetailCustomization.h"
#include "UI/Views/Details/ViewportDetailsContext.h"

namespace Relentless
{
	const DetailCustomizationRegistry& DetailsModule::GetRegistry() const noexcept
	{
		return m_DetailCustomizationRegistry;
	}

	void DetailsModule::OnLoad()
	{
		m_DetailCustomizationRegistry.Register<ViewportDetailsContext, ViewportCameraDetailCustomization>();
		m_DetailCustomizationRegistry.Register<ViewportDetailsContext, ViewportTestDetailCustomization>();
	}
}
