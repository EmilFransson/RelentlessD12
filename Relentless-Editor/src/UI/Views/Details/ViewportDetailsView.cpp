#include "ViewportDetailsView.h"

#include "Core/Editor.h"

#include "Panels/ViewportPanel.h"

namespace Relentless
{
	ViewportDetailsView::ViewportDetailsView(ViewportPanel* aViewportPanel) noexcept
	{
		SetContext(&m_DetailsContext);
		
		Editor::Get()->OnSceneChanged.Connect(this, &ViewportDetailsView::OnSceneChanged);
		m_DetailsContext.CameraController = aViewportPanel->GetCameraController().get();
	}

	void ViewportDetailsView::OnSceneChanged(Scene* aScene) noexcept
	{
		m_DetailsContext.EntityManager = &aScene->GetEntityManager();
		aScene->OnEntityCreated.Connect(this, &ViewportDetailsView::OnEntityCreated);
	}

	void ViewportDetailsView::OnEntityCreated(entity aEntity) noexcept
	{
		m_DetailsContext.Entities.push_back(aEntity);

		Rebuild<ViewportDetailsContext>();
	}

}