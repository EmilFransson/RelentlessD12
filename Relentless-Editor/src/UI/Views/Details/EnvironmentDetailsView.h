#pragma once
#include "IDetailsView.h"

#include "UI/Views/Details/Context/EnvironmentDetailsContext.h"
namespace Relentless
{
	class EnvironmentDetailsView : public IDetailsView
	{
	public:
		explicit EnvironmentDetailsView(const AssetHandle& aEnvironmentHandle) noexcept;
		virtual ~EnvironmentDetailsView() noexcept;
		
		void SetEnvironment(const AssetHandle& aEnvironmentHandle) noexcept;
	private:
		EnvironmentDetailsContext m_DetailsContext;
	};
}