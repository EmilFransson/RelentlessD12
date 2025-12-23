#include "IEditorPanel.h"
#include "../Core/Editor.h"

namespace Relentless
{
	IEditorPanel::IEditorPanel(const char* aName, ImGuiWindowFlags someFlags, std::weak_ptr<Editor> aEditor) noexcept
		:PanelBase(aName, someFlags),
		 m_pEditor{aEditor}
	{
	}
}