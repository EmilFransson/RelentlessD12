#include "DragDropOperation.h"

#include "Tooltip.h"

namespace Relentless
{
	void DragDropOperation::SetTooltipText(std::string_view text) noexcept
	{
		if (!m_pTooltip)
			m_pTooltip = new Tooltip(text);
		else
			m_pTooltip->SetText(text);
	}

	void DragDropOperation::Update() noexcept
	{
		if (m_pTooltip)
			m_pTooltip->OnRender();
	}
}
