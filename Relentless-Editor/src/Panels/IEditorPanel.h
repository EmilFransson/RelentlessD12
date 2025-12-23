#pragma once
#include <Relentless.h>

namespace Relentless
{
	class Editor;

	class IEditorPanel : public PanelBase
	{
	public:
		IEditorPanel(const char* aName, ImGuiWindowFlags someFlags, std::weak_ptr<Editor> aEditor) noexcept;
		virtual ~IEditorPanel() noexcept override = default;
	protected:
		std::weak_ptr<Editor> m_pEditor;
	};
}