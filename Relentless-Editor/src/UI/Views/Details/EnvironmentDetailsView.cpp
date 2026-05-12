#include "EnvironmentDetailsView.h"

namespace Relentless
{
	EnvironmentDetailsView::EnvironmentDetailsView(const AssetHandle& aEnvironmentHandle) noexcept
	{
		SetHorizontalSizePolicy(ESizePolicy::Stretch);
		SetVerticalSizePolicy(ESizePolicy::Stretch);

		SetEnvironment(aEnvironmentHandle);
	}

	EnvironmentDetailsView::~EnvironmentDetailsView() noexcept
	{
		TearDown();
	}

	void EnvironmentDetailsView::SetEnvironment(const AssetHandle& aEnvironmentHandle) noexcept
	{
		m_DetailsContext.EnvironmentHandle = aEnvironmentHandle;
		m_DetailsContext.Environment = AssetManager::Get<Environment>(aEnvironmentHandle);

		SetContext(&m_DetailsContext);
		Rebuild<EnvironmentDetailsContext>();
		RequestRefresh();
	}
}