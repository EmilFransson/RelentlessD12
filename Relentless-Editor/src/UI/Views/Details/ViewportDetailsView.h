#pragma once
#include "IDetailsView.h"

#include "UI/Views/Details/ViewportDetailsContext.h"
namespace Relentless
{
	class ViewportPanel;

	class ViewportDetailsView : public IDetailsView
	{
	public:
		explicit ViewportDetailsView(ViewportPanel* aViewportPanel) noexcept;
		virtual ~ViewportDetailsView() noexcept override = default;

		void OnSceneChanged(Scene* aScene) noexcept;
		void OnEntityCreated(entity aEntity) noexcept;
	private:
		ViewportDetailsContext m_DetailsContext;
	};
}