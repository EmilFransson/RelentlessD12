#include "EnvironmentDetailsView.h"

namespace Relentless
{
	EnvironmentDetailsView::EnvironmentDetailsView(const std::vector<Ref<Environment>>& someEnvironments) noexcept
	{
		SetHorizontalSizePolicy(ESizePolicy::Stretch);
		SetVerticalSizePolicy(ESizePolicy::Stretch);

		m_DetailsContext.Environments = someEnvironments;

		SetContext(&m_DetailsContext);
		Rebuild<EnvironmentDetailsContext>();
	}
}