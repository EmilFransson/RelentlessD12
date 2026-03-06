#pragma once
#include "IDetailsView.h"

#include "UI/Views/Details/Context/ViewportDetailsContext.h"
namespace Relentless
{
	class ViewportPanel;

	class ViewportDetailsView : public IDetailsView
	{
	public:
		explicit ViewportDetailsView(ViewportPanel* aViewportPanel) noexcept;
		virtual ~ViewportDetailsView() noexcept override = default;
	private:
		ViewportDetailsContext m_DetailsContext;
	};
}