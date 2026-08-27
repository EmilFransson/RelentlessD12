#include "ViewportDetailsView.h"

#include "Core/Editor.h"

#include "Panels/ViewportPanel.h"

namespace Relentless
{
	ViewportDetailsView::ViewportDetailsView(ViewportPanel* aViewportPanel) noexcept
	{
		m_DetailsContext.ViewportPanel = aViewportPanel;

		SetContext(&m_DetailsContext);
		Rebuild<ViewportDetailsContext>();
	}
}