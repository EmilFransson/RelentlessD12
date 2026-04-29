#pragma once
#include "UI/Views/Details/EnvironmentDetailsView.h"

#include "Panels/ViewportPanel.h"

namespace Relentless
{
	class Scene;

	class EnvironmentViewportPanel : public ViewportPanel
	{
	public:
		EnvironmentViewportPanel(const std::vector<Ref<Environment>>& someEnvironments) noexcept;
		virtual ~EnvironmentViewportPanel() override;

		NO_DISCARD ViewRenderDesc BuildRenderDescriptor() const noexcept override;
		NO_DISCARD Ref<VerticalBox> BuildWindowLayout() noexcept;
	protected:
		void Update() noexcept override;
	private:
		Ref<Scene> m_pPreviewScene;
		EnvironmentDetailsView* m_pEnvironmentDetailsView = nullptr;
	};
}