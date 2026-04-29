#pragma once
#include "ViewportPanel.h"

namespace Relentless
{
	class EditorViewportPanel : public ViewportPanel
	{
	public:
		EditorViewportPanel() noexcept;
		virtual ~EditorViewportPanel() noexcept override;

		NO_DISCARD virtual ViewRenderDesc BuildRenderDescriptor() const noexcept override;
	};
}