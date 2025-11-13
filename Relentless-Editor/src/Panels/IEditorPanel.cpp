#include "IEditorPanel.h"
#include "../Core/Editor.h"

namespace Relentless
{
	IEditorPanel::IEditorPanel(const char* aName, ImGuiWindowFlags someFlags, Editor* aEditor) noexcept
		:PanelBase(aName, someFlags),
		 m_pEditor{aEditor}
	{
		m_pEditor->OnShutDown.Connect([this]() { m_pEditor = nullptr; });
	}

	IEditorPanel::~IEditorPanel() noexcept
	{
		if (m_pEditor)
			m_pEditor->OnShutDown.Detach(this);
	}
}