#pragma once
#include "IDetailsView.h"

#include "UI/Views/Details/Context/EnvironmentDetailsContext.h"
namespace Relentless
{
	class EnvironmentDetailsView : public IDetailsView
	{
	public:
		explicit EnvironmentDetailsView(const std::vector<Ref<Environment>>& someEnvironments) noexcept;
	private:
		EnvironmentDetailsContext m_DetailsContext;
	};
}